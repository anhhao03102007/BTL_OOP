#ifndef ACCOUNT_DTOS_HPP
#define ACCOUNT_DTOS_HPP

#include <string>
#include <memory>
#include <vector>
#include "model/Account/Account.hpp"
#include <Core ( Backend )/database/json.hpp>

namespace Banking {

// =============================================================
// 1. DTO THÔNG TIN TÀI KHOẢN THUẦN (DATA DTO)
// =============================================================
struct AccountDTO {
    std::string stk;                   // Số tài khoản thanh toán
    std::string user_id;               // Mã sở hữu
    double balance = 0.0;              // Số dư hiện tại
    std::string account_type;          // "CHECKING", "SAVINGS", "CREDIT"
    std::string status;                // "ACTIVE", "LOCKED", "CLOSED"
    std::string created_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        AccountDTO,
        stk,
        user_id,
        balance,
        account_type,
        status,
        created_at
    )
};

// =============================================================
// 2. KẾT QUẢ TRUY VẤN 1 TÀI KHOẢN (SINGLE ACCOUNT RESULT)
// =============================================================
struct AccountQueryResult {
    bool success = false;              // Truy vấn CSDL có thành công không?
    bool found = false;                // Có tìm thấy tài khoản không?
    std::string message;               // Thông báo ("Thành công", "Không tìm thấy STK")
    AccountDTO account;                // Dữ liệu tài khoản thuần (POD)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        AccountQueryResult,
        success,
        found,
        message,
        account
    )
};

// =============================================================
// 3. KẾT QUẢ TRUY VẤN DANH SÁCH TÀI KHOẢN (LIST ACCOUNT RESULT)
// =============================================================
struct AccountListResult {
    bool success = false;
    std::string message;
    size_t total_count = 0;
    std::vector<AccountDTO> accounts;  // Danh sách DTO thuần

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        AccountListResult,
        success,
        message,
        total_count,
        accounts
    )
};

} // namespace Banking

#endif // ACCOUNT_DTOS_HPP