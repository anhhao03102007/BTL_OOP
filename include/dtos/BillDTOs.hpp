#ifndef BILL_DTOS_HPP
#define BILL_DTOS_HPP

#include <string>
#include <Core_Backend/database/json.hpp>

namespace Banking {

using json = nlohmann::json;

// 1. DTO Yêu cầu tra cứu hóa đơn
struct QueryBillRequest {
    std::string session_token;
    std::string provider_id;
    std::string customer_code;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(QueryBillRequest, session_token, provider_id, customer_code)
};

// 2. DTO Thông tin chi tiết hóa đơn
struct BillDTO {
    std::string bill_id;
    std::string provider_id;
    std::string provider_name;
    std::string customer_code;
    std::string customer_name;
    double amount = 0.0;
    std::string service_type;
    std::string status;
    std::string due_date;
    std::string period;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BillDTO, bill_id, provider_id, provider_name, customer_code, customer_name, amount, service_type, status, due_date, period)
};

// 3. DTO Kết quả tra cứu
struct QueryBillResult {
    bool success = false;
    std::string message;
    BillDTO bill;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(QueryBillResult, success, message, bill)
};

// 4. DTO Yêu cầu thanh toán hóa đơn
struct PayBillRequest {
    std::string session_token;
    std::string bill_id;
    std::string pin; // Xác nhận mã PIN 6 số

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PayBillRequest, session_token, bill_id, pin)
};

// 5. DTO Kết quả thanh toán
struct PayBillResult {
    bool success = false;
    std::string message;
    std::string transaction_id;
    double post_balance = 0.0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PayBillResult, success, message, transaction_id, post_balance)
};

} // namespace Banking

#endif // BILL_DTOS_HPP