#ifndef LOAN_DTOS_HPP
#define LOAN_DTOS_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <core/json.hpp>

namespace Banking {

using json = nlohmann::json;

struct ApplyLoanRequest {
    std::string session_token;
    int64_t     amount_vnd = 0;
    int         term_months = 12;
    std::string calc_method = "DECREASING"; // "FLAT" hoặc "DECREASING"
    std::string collateral_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ApplyLoanRequest, session_token, amount_vnd, term_months, calc_method, collateral_info)
};

struct ApproveLoanRequest {
    std::string session_token;
    std::string loan_id;
    std::string action; // "APPROVE" hoặc "REJECT"
    std::string reason;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ApproveLoanRequest, session_token, loan_id, action, reason)
};

struct DisburseLoanRequest {
    std::string session_token;
    std::string loan_id;
    std::string target_account_id;
    std::string pin;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DisburseLoanRequest, session_token, loan_id, target_account_id, pin)
};

struct PayInstallmentRequest {
    std::string session_token;
    std::string schedule_id;
    std::string pin;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PayInstallmentRequest, session_token, schedule_id, pin)
};

struct LoanDTO {
    std::string loan_id;
    std::string user_id;
    int64_t amount_vnd = 0;
    double interest_rate = 0.0;
    int term_months = 0;
    int64_t monthly_installment_vnd = 0;
    std::string collateral_info;
    std::string status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoanDTO, loan_id, user_id, amount_vnd, interest_rate, term_months, monthly_installment_vnd, collateral_info, status)
};

struct StaffLoanApplicationDTO {
    std::string loan_id;
    std::string user_id;
    std::string customer_name;
    std::string customer_phone;
    int64_t amount_vnd = 0;
    double interest_rate = 0.0;
    int term_months = 0;
    std::string calc_method;
    std::string collateral_info;
    std::string status;
    std::string created_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(StaffLoanApplicationDTO, loan_id, user_id, customer_name, customer_phone, amount_vnd, interest_rate, term_months, calc_method, collateral_info, status, created_at)
};

struct StaffLoanApplicationsResult {
    bool success = false;
    std::string message;
    std::vector<StaffLoanApplicationDTO> applications;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(StaffLoanApplicationsResult, success, message, applications)
};

struct LoanOperationResult {
    bool success = false;
    std::string message;
    std::string loan_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoanOperationResult, success, message, loan_id)
};

} // namespace Banking

#endif // LOAN_DTOS_HPP