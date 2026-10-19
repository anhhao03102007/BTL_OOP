#include "service/AccountService.hpp"

namespace Banking {

AccountService::AccountService(AccountRepository& accountRepo, UserRepository& userRepo, AuditRepository& auditRepo)
    : m_accountRepo(accountRepo), m_userRepo(userRepo), m_auditRepo(auditRepo) {}

std::string AccountService::generateNewAccountId(const std::string& type) {
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (type == "CREDIT") return "CRE_" + std::to_string(nowMs);
    if (type == "LOAN")   return "LON_" + std::to_string(nowMs);
    if (type == "TERM_SAVINGS" || type == "DEMAND_SAVINGS") return "SAV_" + std::to_string(nowMs);
    
    return "10" + std::to_string(nowMs % 1000000000); // Mã tài khoản thanh toán 10 số
}

// =========================================================================
// TÍNH NĂNG 1: XEM DANH SÁCH & SỐ DƯ TÀI KHOẢN (RBAC ENFORCEMENT)
// =========================================================================
GetAccountsResult AccountService::getAccounts(const std::string& operatorUserId, const std::string& targetUserId) {
    GetAccountsResult result;

    // 1. Nạp thông tin người thực hiện thao tác
    std::shared_ptr<User> operatorUser = m_userRepo.findById(operatorUserId);
    if (!operatorUser) {
        result.success = false;
        result.message = "Người dùng thực hiện không tồn tại trên hệ thống!";
        return result;
    }

    // 2. Kiểm tra RBAC: Khách hàng CHỈ ĐƯỢC XEM chính chủ. GDV/Admin xem được tất cả.
    if (operatorUser->getRole() == "CUSTOMER" && operatorUserId != targetUserId) {
        result.success = false;
        result.message = "Lỗi phân quyền: Khách hàng chỉ được phép truy vấn tài khoản chính chủ!";
        return result;
    }

    // 3. Truy vấn dữ liệu từ Repository
    std::vector<AccountDTO> dtos = m_accountRepo.getAccountsByUserId(targetUserId);
    result.success = true;
    result.message = "Truy vấn danh sách tài khoản thành công!";
    result.accounts = dtos;

    return result;
}

// =========================================================================
// TÍNH NĂNG 2: TRA CỨU LỊCH SỬ GIAO DỊCH / IN SAO KÊ TÀI KHOẢN
// =========================================================================
TransactionHistoryResult AccountService::getTransactionHistory(const std::string& operatorUserId, 
                                                              const std::string& stk, 
                                                              int limit, 
                                                              int page) {
    TransactionHistoryResult result;
    result.current_page = page;

    // 1. Xác thực người thao tác
    std::shared_ptr<User> operatorUser = m_userRepo.findById(operatorUserId);
    if (!operatorUser) {
        result.success = false;
        result.message = "Người dùng thao tác không tồn tại trên hệ thống!";
        return result;
    }

    std::string role = operatorUser->getRole();

    // 2. Kiểm tra RBAC: Khách hàng phải là chủ sở hữu tài khoản
    if (role == "CUSTOMER") {
        bool isOwner = m_accountRepo.isAccountOwnedByUser(stk, operatorUserId);
        if (!isOwner) {
            result.success = false;
            result.message = "Lỗi phân quyền: Khách hàng chỉ được phép xem sao kê tài khoản chính chủ!";
            return result;
        }
    }

    // 3. Lấy dữ liệu phân trang từ Repository
    result = m_accountRepo.getHistoryPaginated(stk, page, limit);
    return result;
}

// =========================================================================
// TÍNH NĂNG 3: MỞ TÀI KHOẢN MỚI (eKYC KHÁCH HÀNG / GDV MỞ TẠI QUẦY)
// =========================================================================
TransactionResult AccountService::openNewAccount(const std::string& operatorUserId, 
                                                const std::string& targetUserId, 
                                                const std::string& accountType) {
    TransactionResult result;

    // 1. Xác thực người dùng thực hiện
    std::shared_ptr<User> operatorUser = m_userRepo.findById(operatorUserId);
    if (!operatorUser) {
        result.success = false;
        result.message = "Không xác thực được người dùng thực hiện thao tác!";
        return result;
    }

    // 2. Kiểm tra RBAC mở tài khoản
    std::string role = operatorUser->getRole();
    if (role == "CUSTOMER") {
        if (operatorUserId != targetUserId) {
            result.success = false;
            result.message = "Lỗi phân quyền: Khách hàng chỉ được phép tự mở tài khoản eKYC cho chính mình!";
            return result;
        }
    } else if (role != "TELLER" && role != "ADMIN") {
        result.success = false;
        result.message = "Người dùng không có quyền mở tài khoản!";
        return result;
    }

    // 3. Sinh số tài khoản mới & Khởi tạo trạng thái ban đầu
    std::string newStk = generateNewAccountId(accountType);

    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&now));
    std::string createdAt(buf);

    // 4. Lưu tài khoản mới xuống CSDL
    bool created = m_accountRepo.createNewAccount(newStk, targetUserId, accountType, 0, "ACTIVE", createdAt);
    if (!created) {
        result.success = false;
        result.message = "Lỗi CSDL khi tạo tài khoản mới!";
        return result;
    }

    // 5. Ghi nhận Audit Log
    m_auditRepo.logAction(operatorUserId, "OPEN_ACCOUNT_SUCCESS", newStk, 
                          "Type: " + accountType + " for User: " + targetUserId);

    result.success = true;
    result.message = "Mở tài khoản " + accountType + " mới thành công!";
    result.transaction_id = newStk;
    result.post_balance_vnd = 0;
    return result;
}

// =========================================================================
// TÍNH NĂNG 4: KHÓA / MỞ KHÓA TÀI KHOẢN KHẨN CẤP
// =========================================================================
TransactionResult AccountService::toggleAccountLock(const std::string& operatorUserId, 
                                                   const std::string& stk, 
                                                   bool lockRequest, 
                                                   const std::string& pinOrVerifyCode) {
    TransactionResult result;

    // 1. Xác thực người thao tác
    std::shared_ptr<User> operatorUser = m_userRepo.findById(operatorUserId);
    if (!operatorUser) {
        result.success = false;
        result.message = "Người dùng thực hiện không tồn tại!";
        return result;
    }

    std::string role = operatorUser->getRole();

    // 2. Quy tắc an ninh & Phân quyền RBAC
    if (role == "CUSTOMER") {
        // Khách hàng chỉ được tác động tài khoản chính chủ
        if (!m_accountRepo.isAccountOwnedByUser(stk, operatorUserId)) {
            result.success = false;
            result.message = "Lỗi phân quyền: Khách hàng không sở hữu tài khoản này!";
            return result;
        }

        // Khách hàng CHỈ ĐƯỢC TỰ KHÓA. Khi mở khóa phải ra quầy xác minh
        if (!lockRequest) {
            result.success = false;
            result.message = "BẢO MẬT: Khách hàng chỉ được tự KHÓA khẩn cấp. Vui lòng liên hệ Giao dịch viên tại quầy để MỞ KHÓA!";
            return result;
        }

        // Xác thực mã PIN 6 số trước khi khóa
        if (!operatorUser->verifyPin(pinOrVerifyCode)) {
            m_auditRepo.logAction(operatorUserId, "LOCK_ACCOUNT_FAILED", stk, "Mã PIN không chính xác");
            result.success = false;
            result.message = "Mã PIN xác thực khóa khẩn cấp không chính xác!";
            return result;
        }
    } 
    else if (role == "TELLER" || role == "ADMIN") {
        // GDV / Admin mở hoặc khóa sau khi đối chiếu giấy tờ định danh (CCCD)
    }

    // 3. Cập nhật trạng thái CSDL ("LOCKED" hoặc "ACTIVE")
    std::string newStatus = lockRequest ? "LOCKED" : "ACTIVE";
    bool updated = m_accountRepo.updateAccountStatus(stk, newStatus);

    if (!updated) {
        result.success = false;
        result.message = "Lỗi CSDL khi cập nhật trạng thái tài khoản!";
        return result;
    }

    // 4. Ghi nhận Audit Log
    std::string logActionStr = lockRequest ? "LOCK_ACCOUNT_SUCCESS" : "UNLOCK_ACCOUNT_SUCCESS";
    m_auditRepo.logAction(operatorUserId, logActionStr, stk, "Operator: " + role);

    result.success = true;
    result.message = lockRequest ? "Đã khóa khẩn cấp tài khoản thành công!" : "Đã mở khóa tài khoản thành công!";
    result.transaction_id = stk;
    return result;
}

} // namespace Banking