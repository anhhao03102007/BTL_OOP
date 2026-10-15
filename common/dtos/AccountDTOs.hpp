#ifndef ACCOUNT_DTOS_HPP
#define ACCOUNT_DTOS_HPP

#include <string>
#include <memory>
#include <vector>
#include "model/Account/Account.hpp"

// -------------------------------------------------------------
// 1. STRUCT KẾT QUẢ TRUY VẤN 1 TÀI KHOẢN (SINGLE ACCOUNT RESULT)
// -------------------------------------------------------------
struct AccountQueryResult {
    bool success = false;          // Truy vấn CSDL có thành công không?
    bool found = false;            // Có tìm thấy tài khoản trong CSDL không?
    std::string message;           // Thông báo (VD: "Thành công", "Không tìm thấy STK")
    std::unique_ptr<Account> account = nullptr; // Pointer chứa Class Account Đa hình
};

// -------------------------------------------------------------
// 2. STRUCT KẾT QUẢ TRUY VẤN DANH SÁCH TÀI KHOẢN (LIST ACCOUNT RESULT)
// -------------------------------------------------------------
struct AccountListResult {
    bool success = false;
    std::string message;
    size_t total_count = 0;
    std::vector<std::shared_ptr<Account>> accounts; // Danh sách các Account
};

#endif // ACCOUNT_DTOS_HPP