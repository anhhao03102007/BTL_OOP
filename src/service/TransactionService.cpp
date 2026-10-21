#include "services/TransactionService.hpp"

#include <string>

namespace Banking {
namespace {

int64_t readFeeConfig(TursoDB& database, const std::string& key, int64_t fallback) {
    const json response = database.execute(
        "SELECT value FROM system_config WHERE key = ? LIMIT 1;",
        json::array({{{"type", "text"}, {"value", key}}}));
    try {
        const auto cell = response["results"][0]["response"]["result"]["rows"][0][0];
        if (!cell.is_object() || !cell.contains("value") || cell["value"].is_null()) return fallback;
        const auto& value = cell["value"];
        const std::string text = value.is_string() ? value.get<std::string>() : value.dump();
        const int64_t parsed = std::stoll(text);
        return parsed >= 0 && parsed <= 1000000000LL ? parsed : fallback;
    } catch (...) {
        return fallback;
    }
}

} // namespace

TransactionService::TransactionService(TransactionRepository& txnRepo, UserRepository& userRepo,
                                       AccountRepository& accRepo, BillRepository& billRepo,
                                       AuditRepository& auditRepo, SessionManager& sessionManager, TursoDB& tursoDb)
    : m_txnRepo(txnRepo), m_userRepo(userRepo), m_accRepo(accRepo),
      m_billRepo(billRepo), m_auditRepo(auditRepo), m_sessionManager(sessionManager), m_tursoDb(tursoDb) {}

std::string TransactionService::getCurrentDateStr() {
    std::time_t now = std::time(nullptr);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::gmtime(&now));
    return std::string(buf);
}

bool TransactionService::verifyOtp(const std::string&) const {
    // No trusted OTP provider is configured yet. Never accept a fixed/demo code.
    return false;
}

// =========================================================================
// 1. LUỒNG CHUYỂN TIỀN (TransferTransaction)
// =========================================================================
TransactionResult TransactionService::executeTransfer(const TransferRequest& req) {
    TransactionResult res;

    // 1. Check Idempotency Key (Chống bấm lặp nút)
    if (!req.idempotency_key.empty()) {
        auto cached = m_txnRepo.getIdempotentResponse(req.idempotency_key);
        if (cached.has_value() && !cached->empty()) {
            return json::parse(cached.value()).get<TransactionResult>();
        }
    }

    // 2. Xác thực Session RAM
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên đăng nhập không hợp lệ hoặc đã hết hạn!";
        return res;
    }
    const auto& sess = sessOpt.value();

    // 3. Chặn chuyển tiền cho chính tài khoản nguồn
    if (sess.primary_account_id == req.receiver_account_id) {
        res.message = "Tài khoản thụ hưởng không được trùng với tài khoản chuyển!";
        return res;
    }

    // 4. XÁC THỰC MÃ PIN GIAO DỊCH (6 số)
    auto userOpt = m_userRepo.findById(sess.user_id);
    if (!userOpt || !userOpt->verifyPin(req.pin)) {
        m_auditRepo.logAction(sess.user_id, "TRANSFER_FAILED", sess.primary_account_id, "Sai mã PIN giao dịch");
        res.message = "Mã PIN giao dịch không chính xác!";
        return res;
    }

    // 5. Kiểm tra hạn mức giao dịch
    if (req.amount_vnd <= 0 || req.amount_vnd > SINGLE_TXN_LIMIT_VND) {
        res.message = "Số tiền vượt quá hạn mức giao dịch tối đa cho 1 lần (500.000.000 VNĐ)!";
        return res;
    }

    std::string today = getCurrentDateStr();
    int64_t currentDailyTotal = m_txnRepo.getDailyTotalVnd(sess.user_id, today);
    if (currentDailyTotal + req.amount_vnd > DAILY_TXN_LIMIT_VND) {
        res.message = "Giao dịch thất bại! Đã vượt hạn mức cộng dồn trong ngày (1.000.000.000 VNĐ).";
        return res;
    }

    // 6. XÁC THỰC MÃ OTP (NẾU GIAO DỊCH > 50 TRIỆU VNĐ)
    if (req.amount_vnd > OTP_THRESHOLD_VND) {
        if (!verifyOtp(req.otp)) {
            m_auditRepo.logAction(sess.user_id, "TRANSFER_FAILED", sess.primary_account_id, "Sai mã OTP");
            res.message = "Mã OTP xác thực giao dịch giá trị lớn không chính xác!";
            return res;
        }
    }

    // 7. Nạp Models từ DB lên RAM
    auto senderOpt = m_accRepo.findCheckingAccountById(sess.primary_account_id);
    auto receiverOpt = m_accRepo.findCheckingAccountById(req.receiver_account_id);

    if (!senderOpt || !receiverOpt) {
        res.message = "Thông tin tài khoản giao dịch không tồn tại!";
        return res;
    }

    CheckingAccount sender = senderOpt.value();
    CheckingAccount receiver = receiverOpt.value();

    // 8. Khởi tạo Command & Thực thi biến đổi trên RAM
    const int64_t interbankFee = readFeeConfig(m_tursoDb, "admin.interbank_fee_vnd", 9900);
    TransferTransaction txn(sender, receiver, req.amount_vnd, req.description, req.is_interbank, interbankFee);
    if (!txn.process()) {
        res.message = "Số dư tài khoản không đủ để thực hiện giao dịch (bao gồm phí)!";
        return res;
    }

    int64_t newBalance = sender.getBalanceVnd();

    // 9. Đóng gói kết quả & Lưu Atomic Batch xuống TursoDB
    res.success = true;
    res.message = "Chuyển tiền thành công!";
    res.transaction_id = txn.getTransactionId();
    res.post_balance_vnd = newBalance;

    std::string responseJsonStr = json(res).dump();
    bool ok = m_txnRepo.executeAtomicTransfer(txn, sess.user_id, today, req.idempotency_key, newBalance, responseJsonStr);

    if (ok) {
        m_auditRepo.logAction(sess.user_id, "TRANSFER_SUCCESS", txn.getTransactionId(), 
                              "Amount: " + std::to_string(req.amount_vnd) + " VND to " + req.receiver_account_id);
    } else {
        res = TransactionResult{false, "Lỗi CSDL khi thực thi giao dịch!", "", 0};
    }

    return res;
}

// =========================================================================
// 2. LUỒNG RÚT TIỀN (WithdrawTransaction)
// =========================================================================
TransactionResult TransactionService::executeWithdraw(const std::string& sessionToken, const std::string& pin,
                                                       const std::string& otp, int64_t amountVnd,
                                                       const std::string& desc, bool isAtmOffNet) {
    TransactionResult res;

    // 1. Xác thực Session
    auto sessOpt = m_sessionManager.getSession(sessionToken);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc đã hết hạn!";
        return res;
    }
    const auto& sess = sessOpt.value();

    // 2. XÁC THỰC MÃ PIN
    auto userOpt = m_userRepo.findById(sess.user_id);
    if (!userOpt || !userOpt->verifyPin(pin)) {
        m_auditRepo.logAction(sess.user_id, "WITHDRAW_FAILED", sess.primary_account_id, "Sai mã PIN");
        res.message = "Mã PIN giao dịch không chính xác!";
        return res;
    }

    // 3. XÁC THỰC OTP NẾU RÚT > 50 TRIỆU
    if (amountVnd > OTP_THRESHOLD_VND && !verifyOtp(otp)) {
        res.message = "Mã OTP xác thực rút tiền giá trị lớn không chính xác!";
        return res;
    }

    // 4. Nạp Model & Thực thi Command
    auto accOpt = m_accRepo.findCheckingAccountById(sess.primary_account_id);
    if (!accOpt) {
        res.message = "Tài khoản thanh toán không tồn tại!";
        return res;
    }

    CheckingAccount sourceAcc = accOpt.value();
    const int64_t atmFee = readFeeConfig(m_tursoDb, "admin.atm_fee_vnd", 3300);
    WithdrawTransaction txn(sourceAcc, amountVnd, desc, isAtmOffNet, atmFee);

    if (!txn.process()) {
        res.message = "Số dư không đủ để rút tiền (bao gồm phí)!";
        return res;
    }

    // 5. Lưu Persistence
    bool ok = m_txnRepo.saveWithdrawBatch(txn, sourceAcc.getBalanceVnd());
    if (ok) {
        res.success = true;
        res.message = "Rút tiền thành công!";
        res.transaction_id = txn.getTransactionId();
        res.post_balance_vnd = sourceAcc.getBalanceVnd();
        m_auditRepo.logAction(sess.user_id, "WITHDRAW_SUCCESS", txn.getTransactionId(), "Rút: " + std::to_string(amountVnd));
    } else {
        res.message = "Lỗi CSDL khi ghi nhận rút tiền!";
    }

    return res;
}

// =========================================================================
// 3. LUỒNG THANH TOÁN HÓA ĐƠN (BillPaymentTransaction)
// =========================================================================
TransactionResult TransactionService::executeBillPayment(const std::string& sessionToken, const std::string& pin,
                                                          const std::string& billId, const std::string& desc) {
    TransactionResult res;

    // 1. Xác thực Session
    auto sessOpt = m_sessionManager.getSession(sessionToken);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc đã hết hạn!";
        return res;
    }
    const auto& sess = sessOpt.value();

    // 2. XÁC THỰC MÃ PIN
    auto userOpt = m_userRepo.findById(sess.user_id);
    if (!userOpt || !userOpt->verifyPin(pin)) {
        m_auditRepo.logAction(sess.user_id, "BILL_PAY_FAILED", billId, "Sai mã PIN");
        res.message = "Mã PIN giao dịch không chính xác!";
        return res;
    }

    // 3. Nạp Model Account & Bill
    auto accOpt = m_accRepo.findCheckingAccountById(sess.primary_account_id);
    auto billOpt = m_billRepo.findById(billId);

    if (!accOpt || !billOpt) {
        res.message = "Tài khoản hoặc hóa đơn không tồn tại!";
        return res;
    }

    CheckingAccount payerAcc = accOpt.value();
    BillModel bill = billOpt.value();

    // 4. Thực thi Command
    BillPaymentTransaction txn(payerAcc, bill, desc);
    if (!txn.process()) {
        res.message = "Hóa đơn đã được thanh toán trước đó hoặc số dư không đủ!";
        return res;
    }

    // 5. Lưu Persistence
    bool ok = m_txnRepo.saveBillPaymentBatch(txn, payerAcc.getBalanceVnd());
    if (ok) {
        res.success = true;
        res.message = "Thanh toán hóa đơn thành công!";
        res.transaction_id = txn.getTransactionId();
        res.post_balance_vnd = payerAcc.getBalanceVnd();
        m_auditRepo.logAction(sess.user_id, "BILL_PAY_SUCCESS", billId, "Bill: " + billId);
    } else {
        res.message = "Lỗi CSDL khi cập nhật hóa đơn!";
    }

    return res;
}

// =========================================================================
// 4. LUỒNG NẠP TIỀN MẶT TẠI QUẦY (DepositTransaction)
// =========================================================================
TransactionResult TransactionService::executeDeposit(const std::string& tellerSessionToken, const std::string& targetAccId,
                                                      int64_t amountVnd, const std::string& desc) {
    TransactionResult res;

    // Xác thực Session Teller/Giao dịch viên
    auto sessOpt = m_sessionManager.getSession(tellerSessionToken);
    if (!sessOpt.has_value() || (sessOpt->role != "TELLER" && sessOpt->role != "ADMIN")) {
        res.message = "BẢO MẬT: Chỉ Giao dịch viên/Admin mới có quyền nạp tiền mặt tại quầy!";
        return res;
    }

    auto accOpt = m_accRepo.findCheckingAccountById(targetAccId);
    if (!accOpt) {
        res.message = "Tài khoản thụ hưởng không tồn tại!";
        return res;
    }

    CheckingAccount targetAcc = accOpt.value();
    DepositTransaction txn(targetAcc, amountVnd, desc);

    if (!txn.process()) {
        res.message = "Thao tác nạp tiền mặt thất bại!";
        return res;
    }

    bool ok = m_txnRepo.saveDepositBatch(txn, targetAcc.getBalanceVnd());
    if (ok) {
        res.success = true;
        res.message = "Nạp tiền mặt tại quầy thành công!";
        res.transaction_id = txn.getTransactionId();
        res.post_balance_vnd = targetAcc.getBalanceVnd();
        m_auditRepo.logAction(sessOpt->user_id, "DEPOSIT_COUNTER_SUCCESS", targetAccId, "Nạp: " + std::to_string(amountVnd));
    } else {
        res.message = "Lỗi CSDL khi nạp tiền!";
    }

    return res;
}

} // namespace Banking
