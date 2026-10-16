
#ifndef SAVINGS_DTOS_HPP
#define SAVINGS_DTOS_HPP

#include <string>
#include <vector>
#include <Core ( Backend )/database/json.hpp>

namespace Banking {

// =============================================================
// 1. DTO CHI TIẾT SỔ TIẾT KIỆM (SAVINGS BOOK ITEM DTO)
// =============================================================
struct SavingsBookDTO {
    std::string savings_id;        // Mã sổ tiết kiệm (VD: "SAV1729080000")
    std::string user_id;           // Mã chủ sở hữu
    std::string source_stk;        // Số tài khoản thanh toán liên kết
    double deposit_amount = 0.0;   // Số tiền gốc gửi
    double interest_rate = 0.0;    // Lãi suất %/năm (VD: 0.055 cho 5.5%)
    int term_months = 0;           // Kỳ hạn gửi (tháng)
    std::string maturity_option;   // "ROLLOVER_ALL", "CLOSE_TO_CHECKING"
    std::string status;            // "ACTIVE", "CLOSED", "MATURED"
    std::string start_date;        // Ngày mở sổ (YYYY-MM-DD)
    std::string maturity_date;     // Ngày đáo hạn (YYYY-MM-DD)
    double expected_interest = 0.0;// Lãi dự kiến nhận được khi đáo hạn

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SavingsBookDTO,
        savings_id,
        user_id,
        source_stk,
        deposit_amount,
        interest_rate,
        term_months,
        maturity_option,
        status,
        start_date,
        maturity_date,
        expected_interest
    )
};

// =============================================================
// 2. STRUCT TRUYỀN TỪ UI -> SERVICE (REQUEST DTOs)
// =============================================================

// Yêu cầu Mở Sổ Tiết Kiệm từ UI
struct OpenSavingsRequest {
    std::string user_id;
    std::string source_stk;      // Tài khoản thanh toán trích tiền
    double deposit_amount = 0.0;
    int term_months = 6;
    double interest_rate = 0.055;
    std::string maturity_option; // "ROLLOVER_ALL", "CLOSE_TO_CHECKING"
    std::string pin_input;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        OpenSavingsRequest,
        user_id,
        source_stk,
        deposit_amount,
        term_months,
        interest_rate,
        maturity_option,
        pin_input
    )
};

// Yêu cầu Tất toán / Rút Sổ Tiết Kiệm từ UI
struct WithdrawSavingsRequest {
    std::string user_id;
    std::string savings_id;
    std::string destination_stk; // Tài khoản nhận tiền sau tất toán
    std::string pin_input;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        WithdrawSavingsRequest,
        user_id,
        savings_id,
        destination_stk,
        pin_input
    )
};

// Yêu cầu Tra cứu danh sách Sổ Tiết Kiệm
struct GetSavingsRequest {
    std::string user_id;
    std::string status_filter = "ALL"; // Lọc trạng thái: "ALL", "ACTIVE", "CLOSED"

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        GetSavingsRequest,
        user_id,
        status_filter
    )
};

// =============================================================
// 3. STRUCT TRUYỀN TỪ SERVICE -> BRIDGE / UI (RESPONSE DTOs)
// =============================================================

// Kết quả trả về danh sách Sổ Tiết Kiệm
struct SavingsListResult {
    bool success = false;
    std::string message;
    size_t total_count = 0;
    std::vector<SavingsBookDTO> savings_list; // Danh sách sổ tiết kiệm

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        SavingsListResult,
        success,
        message,
        total_count,
        savings_list
    )
};

struct WithdrawSavingsRequest {
    std::string user_id;
    std::string savings_id;
    std::string destination_stk;
    std::string pin_input;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        WithdrawSavingsRequest,
        user_id,
        savings_id,
        destination_stk,
        pin_input
    )
};
}
#endif // SAVINGS_DTOS_HPP