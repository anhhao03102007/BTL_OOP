#ifndef BANKING_DTOS_HPP
#define BANKING_DTOS_HPP

#include <string>
#include <vector>
#include <Core ( Backend )/database/json.hpp>

namespace Banking {

// =============================================================
// 1. STRUCT BẢN GHI LỊCH SỬ GIAO DỊCH (TRANSACTION ITEM DTO)
// =============================================================
struct TransactionDTO {
    std::string transaction_id;   // Mã giao dịch (VD: "TX_1729080000")
    std::string sender_stk;       // Tài khoản nguồn
    std::string receiver_stk;     // Tài khoản đích
    double amount = 0.0;          // Số tiền giao dịch
    double post_balance = 0.0;    // Số dư sau giao dịch
    std::string description;      // Nội dung chuyển khoản
    std::string transaction_type; // "TRANSFER", "DEPOSIT", "WITHDRAWAL", v.v.
    std::string timestamp;        // Thời gian thực hiện (YYYY-MM-DD HH:MM:SS)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        TransactionDTO,
        transaction_id,
        sender_stk,
        receiver_stk,
        amount,
        post_balance,
        description,
        transaction_type,
        timestamp
    )
};

// =============================================================
// 2. PHÂN ĐOẠN 1: BƯỚC TRA CỨU & XÁC THỰC TÀI KHOẢN ĐÍCH
// =============================================================

// Yêu cầu tra cứu thông tin tài khoản nhận từ UI
struct VerifyAccountRequest {
    std::string sender_stk;       // Tài khoản người gửi
    std::string receiver_stk;     // Số tài khoản nhận nhập từ UI
    std::string target_bank = "OOPBANK"; // Ngân hàng thụ hưởng

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        VerifyAccountRequest,
        sender_stk,
        receiver_stk,
        target_bank
    )
};

// Kết quả xác thực tài khoản nhận trả về UI để hiển thị tên chủ tài khoản
struct VerifyAccountResult {
    bool success = false;
    std::string message;
    std::string receiver_stk;     // Số tài khoản thụ hưởng
    std::string receiver_name;    // Tên chủ tài khoản thụ hưởng (VD: "NGUYEN VAN A")
    std::string target_bank;      // Tên ngân hàng thụ hưởng

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        VerifyAccountResult,
        success,
        message,
        receiver_stk,
        receiver_name,
        target_bank
    )
};

// =============================================================
// 3. PHÂN ĐOẠN 2 & 3: BƯỚC THỰC THI GIAO DỊCH (SỐ TIỀN + MÔ TẢ + PIN/OTP)
// =============================================================

// Yêu cầu Chuyển tiền hoàn chỉnh sau khi đã xác minh xong tài khoản nhận
struct TransferRequest {
    std::string sender_stk;
    std::string receiver_stk;
    std::string receiver_name;    // Tên người nhận đã được xác thực ở Bước 1
    std::string target_bank = "OOPBANK";
    double amount = 0.0;          // Số tiền chuyển
    std::string description;      // Lời nhắn / Mô tả
    std::string pin_input;        // Mã PIN giao dịch xác nhận
    std::string otp_input;        // Mã OTP xác thực (nếu có)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        TransferRequest, 
        sender_stk, 
        receiver_stk, 
        receiver_name,
        target_bank, 
        amount, 
        description, 
        pin_input, 
        otp_input
    )
};

// =============================================================
// 4. STRUCT TRUYỀN TỪ SERVICE -> BRIDGE / UI (RESPONSE DTOs)
// =============================================================

// Kết quả biên lai trả về sau khi thực thi giao dịch thành công/thất bại
struct TransactionResult {
    bool success = false;
    std::string message;            // Thông báo lỗi hoặc thành công
    std::string transaction_ref;    // Mã tham chiếu (FT2026...)
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

// Kết quả tra cứu lịch sử sao kê giao dịch từ Service
struct TransactionHistoryResult {
    bool success = false;
    std::string message;
    std::string stk;                // Số tài khoản tra cứu
    size_t total_records = 0;       // Tổng số bản ghi tìm thấy
    std::vector<TransactionDTO> transactions; // Danh sách biến động số dư

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        TransactionHistoryResult,
        success,
        message,
        stk,
        total_records,
        transactions
    )
};

} // namespace Banking

#endif // BANKING_DTOS_HPP
