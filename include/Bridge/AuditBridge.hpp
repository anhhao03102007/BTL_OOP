#ifndef AUDIT_BRIDGE_HPP
#define AUDIT_BRIDGE_HPP

#include <QObject>
#include <QString>

namespace Banking {

class AuditService;

class AuditBridge : public QObject {
    Q_OBJECT

private:
    AuditService& m_service;

public:
    explicit AuditBridge(AuditService& service, QObject* parent = nullptr);

    // API tra cứu Audit Log
    Q_INVOKABLE QString getAuditLogs(const QString& jsonRequest);
};

} // namespace Banking

#endif // AUDIT_BRIDGE_HPP