#ifndef STAFF_DTOS_HPP
#define STAFF_DTOS_HPP

#include <string>
#include <vector>
#include <core/json.hpp>

namespace Banking {

using json = nlohmann::json;

struct ToggleUserStatusRequest {
    std::string session_token;
    std::string target_user_id;
    std::string action; // "LOCK" hoặc "UNLOCK"
    std::string reason;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ToggleUserStatusRequest, session_token, target_user_id, action, reason)
};

struct OpenCounterAccountRequest {
    std::string session_token;
    std::string customer_user_id;
    std::string account_type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(OpenCounterAccountRequest, session_token, customer_user_id, account_type)
};

struct CreateStaffRequest {
    std::string session_token;
    std::string username;
    std::string fullname;
    std::string email;
    std::string phone;
    std::string cccd;
    std::string role; // "TELLER" hoặc "ADMIN"
    std::string password;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(CreateStaffRequest, session_token, username, fullname, email, phone, cccd, role, password)
};

struct AuditLogItemDTO {
    std::string log_id;
    std::string operator_id;
    std::string action;
    std::string target_id;
    std::string details;
    std::string created_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AuditLogItemDTO, log_id, operator_id, action, target_id, details, created_at)
};

struct GetAuditLogsRequest {
    std::string session_token;
    int page = 1;
    int page_size = 20;
    std::string operator_filter;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GetAuditLogsRequest, session_token, page, page_size, operator_filter)
};

struct GetAuditLogsResult {
    bool success = false;
    std::string message;
    int total_items = 0;
    int current_page = 1;
    int total_pages = 1;
    std::vector<AuditLogItemDTO> items;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GetAuditLogsResult, success, message, total_items, current_page, total_pages, items)
};

struct StaffOperationResult {
    bool success = false;
    std::string message;
    std::string created_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(StaffOperationResult, success, message, created_id)
};

} // namespace Banking

#endif // STAFF_DTOS_HPP