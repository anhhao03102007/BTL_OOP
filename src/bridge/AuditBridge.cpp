#include "bridge/AuditBridge.hpp"
#include "services/AuditService.hpp"
#include "dtos/AuditDTOs.hpp"
#include <core/json.hpp>

namespace Banking {

using json = nlohmann::json;

AuditBridge::AuditBridge(AuditService& service, QObject* parent)
    : QObject(parent), m_service(service) {}

QString AuditBridge::getAuditLogs(const QString& jsonRequest) {
    try {
        auto req = json::parse(jsonRequest.toStdString()).get<GetAuditLogsRequest>();
        GetAuditLogsResult res = m_service.getLogs(req);
        json j = res;
        return QString::fromStdString(j.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Định dạng JSON yêu cầu không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

} // namespace Banking