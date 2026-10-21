#include "services/BillService.hpp"
#include "model/TransactionModel.hpp"
#include "model/AccountModel.hpp"
#include "model/BillModel.hpp"

namespace Banking {

BillService::BillService(BillRepository& billRepo, UserRepository& userRepo,
                         AccountRepository& accRepo, TransactionRepository& txnRepo,
                         AuditRepository& auditRepo, SessionManager& sessionManager)
    : m_billRepo(billRepo), m_userRepo(userRepo), m_accRepo(accRepo),
      m_txnRepo(txnRepo), m_auditRepo(auditRepo), m_sessionManager(sessionManager) {}

GetBillProvidersResult BillService::getProviders(const std::string& sessionToken) {
    GetBillProvidersResult result;
    if (!m_sessionManager.getSession(sessionToken).has_value()) {
        result.message = "Phiên làm việc không hợp lệ hoặc đã hết hạn!";
        return result;
    }
    for (const auto& provider : m_billRepo.findProviders()) {
        result.providers.push_back({provider.providerId, provider.providerName, provider.serviceType});
    }
    result.success = true;
    result.message = "Đã tải nhà cung cấp hóa đơn.";
    return result;
}

// BƯỚC 1: TRA CỨU CƯỚC HÓA ĐƠN
QueryBillResult BillService::queryBill(const QueryBillRequest& req) {
    QueryBillResult res;
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc không hợp lệ hoặc đã hết hạn!";
        return res;
    }

    auto billOpt = m_billRepo.findUnpaidBill(req.provider_id, req.customer_code);
    if (!billOpt.has_value()) {
        res.message = "Không tìm thấy hóa đơn chưa thanh toán!";
        return res;
    }

    const auto& bill = billOpt.value();
    res.success = true;
    res.message = "Tra cứu cước thành công!";
    res.bill = BillDTO{
        bill.getBillId(), bill.getProviderId(), bill.getProviderName(),
        bill.getCustomerCode(), bill.getCustomerName(), bill.getAmountVnd(),
        bill.getServiceType(), bill.getStatus(), bill.getDueDate(), bill.getPeriod()
    };
    return res;
}

// BƯỚC 2 & 3: XÁC THỰC PIN & THANH TOÁN
PayBillResult BillService::payBill(const PayBillRequest& req) {
    PayBillResult res;
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc đã hết hạn!";
        return res;
    }
    const auto& sess = sessOpt.value();

    // 1. Xác thực PIN trên RAM Cache
    auto userOpt = m_userRepo.findById(sess.user_id);
    if (!userOpt|| !userOpt->verifyPin(req.pin)) {
        m_auditRepo.logAction(sess.user_id, "PAY_BILL_FAILED", req.bill_id, "Invalid PIN");
        res.message = "Mã PIN giao dịch không chính xác!";
        return res;
    }

    // 2. Kiểm tra trạng thái Hóa đơn
    auto billOpt = m_billRepo.findById(req.bill_id);
    if (!billOpt.has_value() || !billOpt->isPayable()) {
        res.message = "Hóa đơn không hợp lệ hoặc đã được thanh toán trước đó!";
        return res;
    }
    BillModel bill = billOpt.value(); // Lấy bản sao để xử lý

    // 3. Truy vấn cụ thể Model Tài khoản từ Repository
    auto accOpt = m_accRepo.findCheckingAccountById(sess.primary_account_id);
    if (!accOpt.has_value()) {
        res.message = "Không tìm thấy tài khoản thanh toán hợp lệ!";
        return res;
    }
    CheckingAccount payerAcc = accOpt.value();

    // 4. Kiểm tra số dư
    if (payerAcc.getBalanceVnd() < bill.getAmountVnd()) {
        res.message = "Số dư tài khoản không đủ để thanh toán!";
        return res;
    }

    // 5. Khởi tạo đối tượng Đa hình BillPaymentTransaction
    std::string desc = "Thanh toan cuoc " + bill.getServiceType() + " HD " + bill.getBillId();
    BillPaymentTransaction txn(payerAcc, bill, desc);

    if (!txn.process()) {
        res.message = "Lỗi xử lý logic hóa đơn (Số dư không hợp lệ)!";
        return res;
    }

    // 6. Lưu giao dịch theo Batch Atomic vào Turso DB
    if (m_txnRepo.saveBillPaymentBatch(txn, payerAcc.getBalanceVnd())) {
        res.success = true;
        res.message = "Thanh toán hóa đơn thành công!";
        res.transaction_id = txn.getTransactionId();
        res.post_balance_vnd = payerAcc.getBalanceVnd();
        m_auditRepo.logAction(sess.user_id, "PAY_BILL_SUCCESS", bill.getBillId(), 
                              "Amount: " + std::to_string(bill.getAmountVnd()) + " VND");
    } else {
        res.message = "Lỗi CSDL khi thực thi thanh toán!";
    }

    return res;
}

} // namespace Banking