#include "services/AuthService.hpp"

namespace Banking {

AuthService::AuthService(AuthRepository& authRepo, UserRepository& userRepo,
                         AccountRepository& accRepo, AuditRepository& auditRepo, 
                         SessionManager& sessionManager)
    : m_authRepo(authRepo), m_userRepo(userRepo), m_accRepo(accRepo),
      m_auditRepo(auditRepo), m_sessionManager(sessionManager) {}

// 1. ĐĂNG NHẬP HỆ THỐNG
LoginResult AuthService::login(const LoginRequest& req) {
    LoginResult result;
    const std::string GENERIC_ERROR = "Thông tin đăng nhập không chính xác hoặc tài khoản đang bị khóa!";

    // 1. Tra cứu người dùng qua Username / Phone / Email / CCCD
    auto user = m_authRepo.findByIdentifier(req.identifier);
    if (!user) {
        m_auditRepo.logAction("GUEST", "LOGIN_FAILED", req.identifier, "Identifier not found");
        result.success = false;
        result.message = GENERIC_ERROR;
        return result;
    }

    // 2. Kiểm tra tài khoản bị khóa vĩnh viễn hoặc khóa tạm thời
    if (user->isBlocked()) {
        m_auditRepo.logAction(user->getUserId(), "LOGIN_BLOCKED", user->getUserId(), "Account is blocked");
        result.success = false;
        result.message = GENERIC_ERROR;
        return result;
    }

    // 3. Xác thực Mật khẩu qua PBKDF2-HMAC-SHA256
    bool passValid = AuthSecurity::verifyPassword(req.password, user->getPasswordHash(), user->getSalt());

    if (!passValid) {
        // 3a. Tăng số lần đăng nhập sai từ giá trị RAM hiện tại
        int failedCount = user->getFailedAttempts() + 1;
        
        // 3b. Cập nhật số lần sai xuống TursoDB
        m_authRepo.updateFailedAttempts(user->getUserId(), failedCount);
        
        // 3c. Cập nhật lại giá trị trên đối tượng RAM để giữ đồng bộ trong phiên
        user->setFailedAttempts(failedCount);

        // 3d. Ghi vết Audit Log
        m_auditRepo.logAction(user->getUserId(), "LOGIN_FAILED", user->getUserId(),
                              "Sai mật khẩu. Số lần nhập sai: " + std::to_string(failedCount));

        // 3e. Nếu đạt ngưỡng tối đa (5 lần), tự động khóa tạm thời 15 phút
        if (failedCount >= MAX_FAILED_ATTEMPTS) {
            m_authRepo.lockUserTemporarily(user->getUserId(), LOCK_DURATION_MINUTES);
            m_auditRepo.logAction("SYSTEM", "USER_LOCKOUT", user->getUserId(), 
                                  "Khóa tài khoản " + std::to_string(LOCK_DURATION_MINUTES) + " phút do nhập sai 5 lần liên tiếp");
        }

        result.success = false;
        result.message = GENERIC_ERROR;
        return result;
    }

    // 4. Đăng nhập thành công -> Reset đếm sai
    m_authRepo.resetFailedAttempts(user->getUserId());

    // 5. Lấy tài khoản thanh toán mặc định
    std::string primaryAcc = m_authRepo.getPrimaryAccountId(user->getUserId());

    // 6. Tạo phiên cô lập trên RAM (Frontend chỉ nhận session_token)
    std::string token = m_sessionManager.createSession(
        user->getUserId(), user->getFullName(), user->getRole(), primaryAcc
    );

    m_auditRepo.logAction(user->getUserId(), "LOGIN_SUCCESS", user->getUserId(), "Role: " + user->getRole());

    // 7. Trả kết quả DTO
    result.success = true;
    result.message = "Đăng nhập thành công!";
    result.session.session_token = token;
    result.session.user_id = user->getUserId();
    result.session.full_name = user->getFullName();
    result.session.role = user->getRole();
    result.session.primary_account_id = primaryAcc;

    return result;
}

// 2. ĐĂNG KÝ TÀI KHOẢN KHÁCH HÀNG MỚI
RegisterResult AuthService::registerCustomer(const RegisterRequest& req) {
    RegisterResult result;

    if (m_authRepo.existsByIdentifier(req.username) || 
        m_authRepo.existsByIdentifier(req.email) ||
        m_authRepo.existsByIdentifier(req.phone) ||
        m_authRepo.existsByIdentifier(req.cccd)) {
        result.success = false;
        result.message = "Tên đăng nhập, Email, SĐT hoặc Số CCCD đã tồn tại trên hệ thống!";
        return result;
    }

    std::string salt = AuthSecurity::generateSalt();
    std::string passHash = AuthSecurity::hashPasswordPBKDF2(req.password, salt);

    std::string newUserId = std::to_string(std::time(nullptr));
    std::string newAccId = std::to_string(100000 + (std::rand() % 900000));

    bool ok = m_authRepo.createCustomer(newUserId, req.username, passHash, salt,
                                        req.fullname, req.email, req.cccd, req.phone) &&
              m_accRepo.createCheckingAccount(newAccId, newUserId);

    if (ok) {
        m_auditRepo.logAction("SYSTEM", "REGISTER_SUCCESS", newUserId, "Created customer account: " + newAccId);
        result.success = true;
        result.message = "Đăng ký tài khoản thành công!";
        result.user_id = newUserId;
        result.account_id = newAccId;
    } else {
        result.success = false;
        result.message = "Lỗi CSDL khi khởi tạo tài khoản!";
    }

    return result;
}

// 3. QUÊN MẬT KHẨU / ĐẶT LẠI MẬT KHẨU
ForgotPasswordResult AuthService::forgotPassword(const ForgotPasswordRequest& req) {
    ForgotPasswordResult res;

    // 1. Xác thực mã OTP (Giả lập kiểm tra định dạng/mã OTP 6 số)
    if (req.otp.empty() || req.otp.length() != 6) {
        res.success = false;
        res.message = "Mã OTP xác thực phải bao gồm 6 chữ số!";
        return res;
    }

    // 2. Tra cứu người dùng theo Identifier
    std::shared_ptr<User> user = m_authRepo.findByIdentifier(req.identifier);
    if (!user) {
        m_auditRepo.logAction("GUEST", "FORGOT_PASS_FAILED", req.identifier, "Identifier not found");
        res.success = false;
        res.message = "Thông tin xác thực không chính xác!";
        return res;
    }

    // 3. Đối chiếu eKYC đa yếu tố: Kiểm tra số CCCD và Số điện thoại chính chủ
    if (user->getPhone() != req.phone || user->getCCCD() != req.cccd) {
        m_auditRepo.logAction(user->getUserId(), "FORGOT_PASS_FAILED", user->getUserId(), "eKYC mismatch (Phone/CCCD)");
        res.success = false;
        res.message = "Số điện thoại hoặc số CCCD không khớp với dữ liệu đăng ký!";
        return res;
    }

    // 4. Sinh Salt ngẫu nhiên mới & Băm mật khẩu mới bằng PBKDF2-HMAC-SHA256
    std::string newSalt = AuthSecurity::generateToken(16);
    std::string newPassHash = AuthSecurity::hashPasswordPBKDF2(req.new_password, newSalt);

    // 5. Cập nhật mật khẩu băm & Salt mới vào TursoDB
    bool updated = m_userRepo.updatePassword(user->getUserId(), newPassHash, newSalt);
    if (updated) {
        m_authRepo.resetFailedAttempts(user->getUserId());
        res.success = true;
        res.message = "Đặt lại mật khẩu thành công! Bạn có thể đăng nhập ngay bằng mật khẩu mới.";

        // 6. Ghi vết Audit Log thành công
        m_auditRepo.logAction(user->getUserId(), "FORGOT_PASS_SUCCESS", user->getUserId(), "Password reset successfully via eKYC & OTP");
    } else {
        res.success = false;
        res.message = "Lỗi CSDL khi cập nhật mật khẩu mới!";
    }

    return res;
}

// 4. ĐĂNG XUẤT (THU HỒI PHIÊN RAM)
bool AuthService::logout(const std::string& sessionToken) {
    auto sessOpt = m_sessionManager.getSession(sessionToken);
    if (sessOpt.has_value()) {
        m_auditRepo.logAction(sessOpt->user_id, "LOGOUT", sessOpt->user_id, "User logged out");
        m_sessionManager.revokeSession(sessionToken);
        return true;
    }
    return false;
}

} // namespace Banking