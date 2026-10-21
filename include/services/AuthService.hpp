#ifndef AUTH_SERVICE_HPP
#define AUTH_SERVICE_HPP

#include <string>
#include <ctime>
#include <cstdlib>

#include "dtos/AuthDTOs.hpp"
#include "repository/AuthRepository.hpp"
#include "repository/UserRepository.hpp"
#include "repository/AccountRepository.hpp"
#include "repository/AuditRepository.hpp"
#include "core/SessionManager.hpp"
#include "core/AuthSecurity.hpp"

namespace Banking {

class AuthService {
private:
    AuthRepository&    m_authRepo;
    UserRepository&    m_userRepo;
    AccountRepository& m_accRepo;
    AuditRepository&   m_auditRepo;
    SessionManager&    m_sessionManager;

    static constexpr int MAX_FAILED_ATTEMPTS = 5;
    static constexpr int LOCK_DURATION_MINUTES = 15;

public:
    AuthService(AuthRepository& authRepo, UserRepository& userRepo, 
                AccountRepository& accRepo, AuditRepository& auditRepo, 
                SessionManager& sessionManager);

    // 1. ĐĂNG NHẬP HỆ THỐNG
    LoginResult login(const LoginRequest& req);

    // 2. ĐĂNG KÝ TÀI KHOẢN KHÁCH HÀNG MỚI
    RegisterResult registerCustomer(const RegisterRequest& req);

    // 3. QUÊN MẬT KHẨU / ĐẶT LẠI MẬT KHẨU
    ForgotPasswordResult forgotPassword(const ForgotPasswordRequest& req);

    // 4. ĐĂNG XUẤT (THU HỒI PHIÊN RAM)
    bool logout(const std::string& sessionToken);
};

} // namespace Banking

#endif // AUTH_SERVICE_HPP