#ifndef AUTH_DTOS_HPP
#define AUTH_DTOS_HPP

#include <string>
#include <optional>
#include <Core ( Backend )/database/json.hpp>

namespace Banking {

// =============================================================
// 1. STRUCT DÀNH CHO ĐĂNG NHẬP (LOGIN)
// =============================================================

// Yêu cầu đăng nhập gửi từ UI Form
struct LoginRequest {
    std::string identifier; // Username, Email, Số điện thoại hoặc CCCD
    std::string password;   // Mật khẩu thô do người dùng nhập

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        LoginRequest, 
        identifier, 
        password
    )
};

// Dữ liệu phiên đăng nhập lưu trong RAM sau khi xác thực thành công
struct UserSessionDTO {
    std::string user_id;
    std::string full_name;
    std::string role;               // "CUSTOMER", "TELLER", "ADMIN"
    std::string primary_account_id; // Số tài khoản thanh toán mặc định
    std::string session_token;      // Mã Session duy nhất cho phiên làm việc

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        UserSessionDTO, 
        user_id, 
        full_name, 
        role, 
        primary_account_id, 
        session_token
    )
};

// Kết quả phản hồi từ AuthService trả về UI
struct LoginResult {
    bool success = false;
    std::string message;
    std::optional<UserSessionDTO> session; // Trả về JSON dưới dạng object hoặc null nếu thất bại

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        LoginResult, 
        success, 
        message, 
        session
    )
};

// =============================================================
// 2. STRUCT DÀNH CHO ĐĂNG KÝ (REGISTER)
// =============================================================

// Yêu cầu đăng ký tài khoản mới gửi từ UI Form
struct RegisterRequest {
    std::string full_name;
    std::string cccd_number;   // Số Căn cước công dân (Dùng định danh unique)
    std::string phone_number;
    std::string email;
    std::string raw_password;  // Mật khẩu thô
    std::string raw_pin;       // Mã PIN 6 số dùng cho giao dịch
    std::string address;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        RegisterRequest, 
        full_name, 
        cccd_number, 
        phone_number, 
        email, 
        raw_password, 
        raw_pin, 
        address
    )
};

// Kết quả phản hồi từ AuthService trả về UI sau khi tạo tài khoản
struct RegisterResult {
    bool success = false;
    std::string message;
    std::string created_user_id;
    std::string default_account_id; // STK thanh toán tự động cấp cho người dùng mới ( dung ham tao ma otp tao ngau nhien chuoi 10 so lam ma tai khoan)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        RegisterResult, 
        success, 
        message, 
        created_user_id, 
        default_account_id
    )
};

} //

#endif // AUTH_DTOS_HPP