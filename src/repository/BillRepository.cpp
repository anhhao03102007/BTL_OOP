#include "repository/BillRepository.hpp"

namespace Banking {

BillRepository::BillRepository(TursoDB& tursoDb) : m_tursoDb(tursoDb) {}

std::string BillRepository::extractCell(const json& cell) {
    if (cell.is_object() && cell.contains("value") && !cell["value"].is_null()) {
        return cell["value"].get<std::string>();
    }
    return "";
}

// 1. Tra cứu hóa đơn chưa thanh toán theo nhà cung cấp và mã khách hàng
std::optional<BillModel> BillRepository::findUnpaidBill(const std::string& providerId, const std::string& customerCode) {
    std::string sql = 
        "SELECT bill_id, provider_id, provider_name, customer_code, customer_name, "
        "amount_vnd, service_type, status, due_date, period "
        "FROM bills WHERE provider_id = ? AND customer_code = ? AND status = 'UNPAID' LIMIT 1;";

    json args = json::array({
        {{"type", "text"}, {"value", providerId}},
        {{"type", "text"}, {"value", customerCode}}
    });

    json res = m_tursoDb.execute(sql, args);
    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                auto r = rows[0];
                return BillModel(
                    extractCell(r[0]), extractCell(r[1]), extractCell(r[2]),
                    extractCell(r[3]), extractCell(r[4]), std::stoll(extractCell(r[5])),
                    extractCell(r[6]), extractCell(r[7]), extractCell(r[8]), extractCell(r[9])
                );
            }
        }
    } catch (...) {}
    return std::nullopt;
}

// 2. Tra cứu hóa đơn theo Mã hóa đơn
std::optional<BillModel> BillRepository::findById(const std::string& billId) {
    std::string sql = 
        "SELECT bill_id, provider_id, provider_name, customer_code, customer_name, "
        "amount_vnd, service_type, status, due_date, period "
        "FROM bills WHERE bill_id = ? LIMIT 1;";

    json args = json::array({{{"type", "text"}, {"value", billId}}});
    json res = m_tursoDb.execute(sql, args);
    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                auto r = rows[0];
                return BillModel(
                    extractCell(r[0]), extractCell(r[1]), extractCell(r[2]),
                    extractCell(r[3]), extractCell(r[4]), std::stoll(extractCell(r[5])),
                    extractCell(r[6]), extractCell(r[7]), extractCell(r[8]), extractCell(r[9])
                );
            }
        }
    } catch (...) {}
    return std::nullopt;
}

// 3. Cập nhật trạng thái hóa đơn sang "PAID"
bool BillRepository::markAsPaid(const std::string& billId) {
    std::string sql = "UPDATE bills SET status = 'PAID' WHERE bill_id = ? AND status = 'UNPAID';";
    json args = json::array({{{"type", "text"}, {"value", billId}}});
    json res = m_tursoDb.execute(sql, args);
    return res.contains("results") && !res["results"].empty() && res["results"][0].value("type", "") == "ok";
}

} // namespace Banking