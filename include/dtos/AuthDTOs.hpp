#ifndef AUTH_DTOS_HPP
#define AUTH_DTOS_HPP

#include <string>
#include <cstdint>
#include <core/json.hpp>

namespace Banking {

using json = nlohmann::json;

// 1. Đăng nhập
struct LoginRequest {
    std::string identifier; // Username / Email / Phone / CCCD
    std::string password;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginRequest, identifier, password)
};

struct UserSessionDTO {
    std::string session_token;
    std::string user_id;
    std::string full_name;
    std::string role;
    std::string primary_account_id;
    int64_t     created_at = 0;
    int64_t     expires_at = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserSessionDTO, session_token, user_id, full_name, role, primary_account_id, created_at, expires_at)
};

struct LoginResult {
    bool           success = false;
    std::string    message;
    UserSessionDTO session;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResult, success, message, session)
};

// 2. Đăng ký tài khoản
struct RegisterRequest {
    std::string username;
    std::string password;
    std::string fullname;
    std::string email;
    std::string cccd;
    std::string phone;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RegisterRequest, username, password, fullname, email, cccd, phone)
};

struct RegisterResult {
    bool        success = false;
    std::string message;
    std::string user_id;
    std::string account_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RegisterResult, success, message, user_id, account_id)
};

// 3. Quên / Đặt lại mật khẩu (Xác minh eKYC: Identifier + CCCD + Phone + OTP)
struct ForgotPasswordRequest {
    std::string identifier;   // Username hoặc Email
    std::string cccd;         // Số CCCD chính chủ
    std::string phone;        // Số điện thoại đăng ký
    std::string otp;          // Mã OTP xác thực (6 số)
    std::string new_password; // Mật khẩu mới

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ForgotPasswordRequest, identifier, cccd, phone, otp, new_password)
};

struct ForgotPasswordResult {
    bool        success = false;
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ForgotPasswordResult, success, message)
};

} // namespace Banking

#endif // AUTH_DTOS_HPP