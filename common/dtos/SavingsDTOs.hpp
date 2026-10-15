#ifndef SAVINGS_DTOS_HPP
#define SAVINGS_DTOS_HPP

#include <string>
#include <vector>
#include <Core ( Backend )/database/json.hpp>

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
#endif // SAVINGS_DTOS_HPP