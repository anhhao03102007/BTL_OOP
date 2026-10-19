#include "repository/AuditRepository.hpp"
#include <ctime>
#include <cstdlib>

namespace Banking {

AuditRepository::AuditRepository(TursoDB& tursoDb) : m_tursoDb(tursoDb) {}

std::string AuditRepository::extractCell(const nlohmann::json& cell) {
    if (cell.is_object() && cell.contains("value") && !cell["value"].is_null()) {
        return cell["value"].get<std::string>();
    }
    return "";
}

std::string AuditRepository::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&now));
    return std::string(buf);
}

// 1. Ghi nhận vết hành động (Append-Only Transaction)
bool AuditRepository::logAction(const std::string& operatorId, const std::string& action,
                                const std::string& targetId, const std::string& details) {
    std::string logId = "LOG_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(std::rand() % 10000);
    std::string nowStr = getCurrentTimestamp();

    std::string sql = 
        "INSERT INTO audit_logs (log_id, operator_id, action, target_id, details, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    nlohmann::json args = nlohmann::json::array({
        {{"type", "text"}, {"value", logId}},
        {{"type", "text"}, {"value", operatorId}},
        {{"type", "text"}, {"value", action}},
        {{"type", "text"}, {"value", targetId}},
        {{"type", "text"}, {"value", details}},
        {{"type", "text"}, {"value", nowStr}}
    });

    nlohmann::json res = m_tursoDb.execute(sql, args);
    return res.contains("results") && !res["results"].empty() &&
           res["results"][0].value("type", "") == "ok";
}

// 2. Tra cứu danh sách Audit Log có Phân Trang dành cho Admin (LIMIT / OFFSET)
GetAuditLogsResult AuditRepository::getLogsPaginated(int page, int pageSize, const std::string& operatorFilter) {
    GetAuditLogsResult res;
    res.current_page = page;

    int offset = (page - 1) * pageSize;
    if (offset < 0) offset = 0;
    bool hasFilter = !operatorFilter.empty();

    // 2a. Đếm tổng số bản ghi phục vụ tính tổng số trang (total_pages)
    std::string countSql = "SELECT COUNT(*) FROM audit_logs";
    nlohmann::json countArgs = nlohmann::json::array();
    if (hasFilter) {
        countSql += " WHERE operator_id = ?;";
        countArgs.push_back({{"type", "text"}, {"value", operatorFilter}});
    } else {
        countSql += ";";
    }

    nlohmann::json countRes = m_tursoDb.execute(countSql, countArgs);
    try {
        if (countRes.contains("results") && !countRes["results"].empty()) {
            auto rows = countRes["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                res.total_items = std::stoi(extractCell(rows[0][0]));
            }
        }
    } catch (...) {}

    res.total_pages = (res.total_items + pageSize - 1) / pageSize;
    if (res.total_pages == 0) res.total_pages = 1;

    // 2b. Truy vấn dữ liệu phân trang theo thứ tự thời gian mới nhất
    std::string sql = "SELECT log_id, operator_id, action, target_id, details, created_at FROM audit_logs";
    nlohmann::json args = nlohmann::json::array();

    if (hasFilter) {
        sql += " WHERE operator_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?;";
        args.push_back({{"type", "text"}, {"value", operatorFilter}});
    } else {
        sql += " ORDER BY created_at DESC LIMIT ? OFFSET ?;";
    }

    args.push_back({{"type", "numeric"}, {"value", pageSize}});
    args.push_back({{"type", "numeric"}, {"value", offset}});

    nlohmann::json queryRes = m_tursoDb.execute(sql, args);
    try {
        if (queryRes.contains("results") && !queryRes["results"].empty()) {
            auto rows = queryRes["results"][0]["response"]["result"]["rows"];
            if (rows.is_array()) {
                for (const auto& r : rows) {
                    res.items.push_back(AuditLogItemDTO{
                        extractCell(r[0]), extractCell(r[1]), extractCell(r[2]),
                        extractCell(r[3]), extractCell(r[4]), extractCell(r[5])
                    });
                }
            }
        }
    } catch (...) {}

    res.success = true;
    res.message = "Tra cứu nhật ký Audit Log thành công!";
    return res;
}

} // namespace Banking