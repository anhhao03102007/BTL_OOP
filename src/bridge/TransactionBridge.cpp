#include "bridge/TransactionBridge.hpp"
#include "services/TransactionService.hpp"
#include "dtos/TransactionDTOs.hpp"
#include <core/json.hpp>

namespace Banking {

using json = nlohmann::json;

TransactionBridge::TransactionBridge(TransactionService& service, QObject* parent)
    : QObject(parent), m_service(service) {}

// =========================================================================
// 1. API CHUYỂN TIỀN (NỘI BỘ & LIÊN NGÂN HÀNG)
// =========================================================================
QString TransactionBridge::transfer(const QString& jsonRequest) {
    try {
        auto req = json::parse(jsonRequest.toStdString()).get<TransferRequest>();
        TransactionResult res = m_service.executeTransfer(req);
        json jRes = res;
        return QString::fromStdString(jRes.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Định dạng JSON yêu cầu không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

// =========================================================================
// 2. API RÚT TIỀN MẶT (ATM / TẠI QUẦY)
// =========================================================================
QString TransactionBridge::withdraw(const QString& jsonRequest) {
    try {
        json jReq = json::parse(jsonRequest.toStdString());
        std::string sessionToken = jReq.value("session_token", "");
        std::string pin          = jReq.value("pin", "");
        std::string otp          = jReq.value("otp", "");
        int64_t amountVnd        = jReq.value("amount_vnd", 0LL);
        std::string description  = jReq.value("description", "");
        bool isAtmOffNet         = jReq.value("is_atm_offnet", false);

        TransactionResult res = m_service.executeWithdraw(sessionToken, pin, otp, amountVnd, description, isAtmOffNet);
        json jRes = res;
        return QString::fromStdString(jRes.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Định dạng JSON yêu cầu không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

// =========================================================================
// 3. API THANH TOÁN HÓA ĐƠN (ĐIỆN, NƯỚC, INTERNET...)
// =========================================================================
QString TransactionBridge::payBill(const QString& jsonRequest) {
    try {
        json jReq = json::parse(jsonRequest.toStdString());
        std::string sessionToken = jReq.value("session_token", "");
        std::string pin          = jReq.value("pin", "");
        std::string billId       = jReq.value("bill_id", "");
        std::string description  = jReq.value("description", "");

        TransactionResult res = m_service.executeBillPayment(sessionToken, pin, billId, description);
        json jRes = res;
        return QString::fromStdString(jRes.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Định dạng JSON yêu cầu không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

// =========================================================================
// 4. API NẠP TIỀN MẶT TẠI QUẦY (DÀNH CHO TELLER / ADMIN)
// =========================================================================
QString TransactionBridge::depositCounter(const QString& jsonRequest) {
    try {
        json jReq = json::parse(jsonRequest.toStdString());
        std::string sessionToken    = jReq.value("session_token", "");
        std::string targetAccountId = jReq.value("target_account_id", "");
        int64_t amountVnd           = jReq.value("amount_vnd", 0LL);
        std::string description     = jReq.value("description", "");

        TransactionResult res = m_service.executeDeposit(sessionToken, targetAccountId, amountVnd, description);
        json jRes = res;
        return QString::fromStdString(jRes.dump());
    } catch (...) {
        json err = {{"success", false}, {"message", "Định dạng JSON yêu cầu không hợp lệ!"}};
        return QString::fromStdString(err.dump());
    }
}

} // namespace Banking