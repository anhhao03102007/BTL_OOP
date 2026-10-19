#include "bridge/BillBridge.hpp"
#include "services/BillService.hpp"
#include "dtos/BillDTOs.hpp"
#include <core/json.hpp>

namespace Banking {

using json = nlohmann::json;

BillBridge::BillBridge(BillService& billService, QObject* parent)
    : QObject(parent), m_billService(billService) {}

// 1. API Tra cứu hóa đơn cước
QString BillBridge::queryBill(const QString& jsonRequest) {
    try {
        auto req = json::parse(jsonRequest.toStdString()).get<QueryBillRequest>();
        QueryBillResult res = m_billService.queryBill(req);
        json resJson = res;
        return QString::fromStdString(resJson.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Yêu cầu Tra cứu Hóa đơn không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

// 2. API Thanh toán hóa đơn
QString BillBridge::payBill(const QString& jsonRequest) {
    try {
        auto req = json::parse(jsonRequest.toStdString()).get<PayBillRequest>();
        PayBillResult res = m_billService.payBill(req);
        json resJson = res;
        return QString::fromStdString(resJson.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Yêu cầu Thanh toán không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

} // namespace Banking