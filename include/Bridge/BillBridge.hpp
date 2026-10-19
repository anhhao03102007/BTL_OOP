#ifndef BILL_BRIDGE_HPP
#define BILL_BRIDGE_HPP

#include <QObject>
#include <QString>

namespace Banking {

class BillService;

class BillBridge : public QObject {
    Q_OBJECT

private:
    BillService& m_billService;

public:
    explicit BillBridge(BillService& billService, QObject* parent = nullptr);

    // 1. API Tra cứu hóa đơn cước
    Q_INVOKABLE QString queryBill(const QString& jsonRequest);

    // 2. API Thanh toán hóa đơn
    Q_INVOKABLE QString payBill(const QString& jsonRequest);
};

} // namespace Banking

#endif // BILL_BRIDGE_HPP