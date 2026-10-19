#ifndef LOAN_BRIDGE_HPP
#define LOAN_BRIDGE_HPP

#include <QObject>
#include <QString>

namespace Banking {

class LoanService;

class LoanBridge : public QObject {
    Q_OBJECT

private:
    LoanService& m_loanService;

public:
    explicit LoanBridge(LoanService& loanService, QObject* parent = nullptr);

    // 1. API Nộp đơn xin vay vốn
    Q_INVOKABLE QString applyLoan(const QString& jsonRequest);

    // 2. API Phê duyệt khoản vay (Teller / Admin)
    Q_INVOKABLE QString approveLoan(const QString& jsonRequest);

    // 3. API Giải ngân khoản vay
    Q_INVOKABLE QString disburseLoan(const QString& jsonRequest);
};

} // namespace Banking

#endif // LOAN_BRIDGE_HPP