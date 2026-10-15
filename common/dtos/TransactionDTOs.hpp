#ifndef BANKING_DTOS_HPP
#define BANKING_DTOS_HPP

#include <string>
#include <vector>
#include <Core ( Backend )/database/json.hpp>

namespace Banking {

// =============================================================
// 1. STRUCT TRUYỀN TỪ UI -> SERVICE (REQUEST DTOs)
// =============================================================

// Struct đóng gói dữ liệu Yêu cầu Chuyển tiền từ UI
struct TransferRequest {
    std::string sender_stk;
    std::string receiver_stk;
    double amount = 0.0;
    std::string description;
    std::string target_bank = "OOPBANK";
    std::string pin_input;
    std::string otp_input;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        TransferRequest, 
        sender_stk, 
        receiver_stk, 
        amount, 
        description, 
        target_bank, 
        pin_input, 
        otp_input
    )
};

// =============================================================
// 2. STRUCT TRUYỀN TỪ BACKEND -> UI (RESPONSE DTOs / BIÊN LAI)
// =============================================================

// Struct kết quả trả về từ Service lên UI cho MỌI GIAO DỊCH
struct TransactionResult {
    bool success = false;
    std::string message;            // Thông báo lỗi hoặc thành công
    std::string transaction_ref;    // Mã giao dịch (Mã tham chiếu FT2026...)
    double amount = 0.0;            // Số tiền thực hiện
    double fee_applied = 0.0;       // Phí giao dịch
    double new_balance = 0.0;       // Số dư mới sau giao dịch
    std::string timestamp;          // Thời gian thực hiện

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        TransactionResult,
        success,
        message,
        transaction_ref,
        amount,
        fee_applied,
        new_balance,
        timestamp
    )
};

} // namespace Banking

#endif // BANKING_DTOS_HPP