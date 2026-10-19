#ifndef TRANSACTION_SERVICE_HPP
#define TRANSACTION_SERVICE_HPP

#include <string>
#include <ctime>
#include <core/json.hpp>

#include "dtos/TransactionDTOs.hpp"
#include "repository/TransactionRepository.hpp"
#include "repository/UserRepository.hpp"
#include "repository/AccountRepository.hpp"
#include "repository/BillRepository.hpp"
#include "repository/AuditRepository.hpp"
#include "core/SessionManager.hpp"
#include "model/TransactionModel.hpp"

namespace Banking {

using json = nlohmann::json;

class TransactionService {
private:
    TransactionRepository& m_txnRepo;
    UserRepository&        m_userRepo;
    AccountRepository&     m_accRepo;
    BillRepository&        m_billRepo;
    AuditRepository&       m_auditRepo;
    SessionManager&        m_sessionManager;

    // Hạn mức bảo mật hệ thống
    static constexpr int64_t SINGLE_TXN_LIMIT_VND = 500000000;   // Hạn mức 1 lần: 500 triệu VNĐ
    static constexpr int64_t DAILY_TXN_LIMIT_VND  = 1000000000;  // Hạn mức ngày: 1 tỷ VNĐ
    static constexpr int64_t OTP_THRESHOLD_VND    = 50000000;    // Ngưỡng bắt buộc OTP: > 50 triệu VNĐ

    // Helpers
    static std::string getCurrentDateStr();
    bool verifyOtp(const std::string& otp) const;

public:
    TransactionService(TransactionRepository& txnRepo, UserRepository& userRepo,
                       AccountRepository& accRepo, BillRepository& billRepo,
                       AuditRepository& auditRepo, SessionManager& sessionManager);

    // 1. Luồng Chuyển tiền (TransferTransaction)
    TransactionResult executeTransfer(const TransferRequest& req);

    // 2. Luồng Rút tiền (WithdrawTransaction)
    TransactionResult executeWithdraw(const std::string& sessionToken, const std::string& pin,
                                      const std::string& otp, int64_t amountVnd,
                                      const std::string& desc, bool isAtmOffNet);

    // 3. Luồng Thanh toán hóa đơn (BillPaymentTransaction)
    TransactionResult executeBillPayment(const std::string& sessionToken, const std::string& pin,
                                          const std::string& billId, const std::string& desc);

    // 4. Luồng Nạp tiền mặt tại quầy (DepositTransaction)
    TransactionResult executeDeposit(const std::string& tellerSessionToken, const std::string& targetAccId,
                                     int64_t amountVnd, const std::string& desc);
};

} // namespace Banking

#endif // TRANSACTION_SERVICE_HPP