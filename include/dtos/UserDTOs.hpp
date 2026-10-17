#ifndef USER_DTOS_HPP
#define USER_DTOS_HPP

#include <string>
#include <vector>
#include <Core_Backend/database/json.hpp>

namespace Banking {

// =============================================================
// 1. STRUCT THÔNG TIN HỒ SƠ NGƯỜI DÙNG (RESPONSE DTO)
// =============================================================
struct UserProfileDTO {
    std::string user_id;
    std::string full_name;
    std::string cccd_number;
    std::string phone_number;
    std::string email;
    std::string address;
    std::string role;               // "CUSTOMER", "TELLER", "ADMIN"
    std::string status;             // "ACTIVE", "BLOCKED"
    std::string primary_account_id; // Số tài khoản thanh toán mặc định

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        UserProfileDTO,
        user_id,
        full_name,
        cccd_number,
        phone_number,
        email,
        address,
        role,
        status,
        primary_account_id
    )
};

// =============================================================
// 2. STRUCT YÊU CẦU CẬP NHẬT THÔNG TIN CÁ NHÂN (REQUEST DTO)
// =============================================================
struct UpdateProfileRequest {
    std::string user_id;
    std::string full_name;
    std::string phone_number;
    std::string email;
    std::string address;
    std::string pin_input;          // Mã PIN xác nhận thay đổi

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        UpdateProfileRequest,
        user_id,
        full_name,
        phone_number,
        email,
        address,
        pin_input
    )
};

// =============================================================
// 3. STRUCT YÊU CẦU ĐỔI MẬT KHẨU (REQUEST DTO)
// =============================================================
struct ChangePasswordRequest {
    std::string user_id;
    std::string old_password;
    std::string new_password;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        ChangePasswordRequest,
        user_id,
        old_password,
        new_password
    )
};

// =============================================================
// 4. STRUCT YÊU CẦU ĐỔI MÃ PIN GIAO DỊCH (REQUEST DTO)
// =============================================================
struct ChangePinRequest {
    std::string user_id;
    std::string old_pin;
    std::string new_pin;
    std::string password_input;     // Mật khẩu xác nhận để đổi PIN

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        ChangePinRequest,
        user_id,
        old_pin,
        new_pin,
        password_input
    )
};

// =============================================================
// 5. STRUCT KẾT QUẢ TRUY VẤN HỒ SƠ NGƯỜI DÙNG (RESPONSE DTO)
// =============================================================
struct UserProfileResult {
    bool success = false;
    std::string message;
    UserProfileDTO profile;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        UserProfileResult,
        success,
        message,
        profile
    )
};

} // namespace Banking

#endif // USER_DTOS_HPP