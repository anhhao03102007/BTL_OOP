#include "bridge/LoanBridge.hpp"
#include "services/LoansService.hpp"
#include "dtos/LoanDTOs.hpp"
#include <core/json.hpp>

namespace Banking {

using json = nlohmann::json;

LoanBridge::LoanBridge(LoanService& loanService, QObject* parent)
    : QObject(parent), m_loanService(loanService) {}

// Danh sách hồ sơ vay dành cho Teller / Admin
QString LoanBridge::getApplications(const QString& jsonRequest) {
    try {
        const json request = json::parse(jsonRequest.toStdString());
        const auto result = m_loanService.getApplicationsForStaff(request.value("session_token", ""));
        return QString::fromStdString(json(result).dump());
    } catch (...) {
        const json error = {{"success", false}, {"message", "Không đọc được danh sách hồ sơ vay."}, {"applications", json::array()}};
        return QString::fromStdString(error.dump());
    }
}

// 1. API Nộp đơn xin vay vốn
QString LoanBridge::applyLoan(const QString& jsonRequest) {
    try {
        auto req = json::parse(jsonRequest.toStdString()).get<ApplyLoanRequest>();
        LoanOperationResult res = m_loanService.applyLoan(req);
        json resJson = res;
        return QString::fromStdString(resJson.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Định dạng Đơn vay không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

// 2. API Phê duyệt khoản vay
QString LoanBridge::approveLoan(const QString& jsonRequest) {
    try {
        auto req = json::parse(jsonRequest.toStdString()).get<ApproveLoanRequest>();
        LoanOperationResult res = m_loanService.approveLoan(req);
        json resJson = res;
        return QString::fromStdString(resJson.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Dữ liệu Phê duyệt không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

// 3. API Giải ngân khoản vay
QString LoanBridge::disburseLoan(const QString& jsonRequest) {
    try {
        auto req = json::parse(jsonRequest.toStdString()).get<DisburseLoanRequest>();
        LoanOperationResult res = m_loanService.disburseLoan(req);
        json resJson = res;
        return QString::fromStdString(resJson.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Dữ liệu Giải ngân không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

} // namespace Banking
