#include "services/LoansService.hpp"

namespace Banking {

LoanService::LoanService(LoanRepository& loanRepo, AccountRepository& accRepo,
                         UserRepository& userRepo, AuditRepository& auditRepo,
                         SessionManager& sessionManager, TursoDB& tursoDb)
    : m_loanRepo(loanRepo), m_accRepo(accRepo), m_userRepo(userRepo),
      m_auditRepo(auditRepo), m_sessionManager(sessionManager), m_tursoDb(tursoDb) {}

// 1. TẠO ĐƠN XIN VAY
LoanOperationResult LoanService::applyLoan(const ApplyLoanRequest& req) {
    LoanOperationResult res;
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc hết hạn!";
        return res;
    }
    const auto& sess = sessOpt.value();

    if (req.amount_vnd <= 0) {
        res.message = "Số tiền xin vay phải lớn hơn 0 VNĐ!";
        return res;
    }

    std::string loanId = "LOAN_" + std::to_string(std::time(nullptr));
    double defaultRate = 8.5;

    std::string sqlLoan = 
        "INSERT INTO loans (loan_id, user_id, amount_vnd, interest_rate, term_months, "
        "calc_method, collateral_info, status, created_by, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 'PENDING', ?, CURRENT_TIMESTAMP);";

    json argsLoan = json::array({
        {{"type", "text"},    {"value", loanId}},
        {{"type", "text"},    {"value", sess.user_id}},
        {{"type", "numeric"}, {"value", req.amount_vnd}},
        {{"type", "numeric"}, {"value", defaultRate}},
        {{"type", "numeric"}, {"value", req.term_months}},
        {{"type", "text"},    {"value", req.calc_method}},
        {{"type", "text"},    {"value", req.collateral_info}},
        {{"type", "text"},    {"value", sess.user_id}}
    });

    json dbRes = m_tursoDb.execute(sqlLoan, argsLoan);
    if (dbRes.contains("results") && !dbRes["results"].empty()) {
        res.success = true;
        res.message = "Tạo đơn xin vay thành công!";
        res.loan_id = loanId;
        m_auditRepo.logAction(sess.user_id, "APPLY_LOAN_SUCCESS", loanId, "Amount: " + std::to_string(req.amount_vnd) + " VND");
    } else {
        res.message = "Lỗi CSDL khi nộp đơn xin vay!";
    }

    return res;
}

// 2. PHÊ DUYỆT KHOẢN VAY (KÈM KIỂM TRA QUYỀN RBAC)
LoanOperationResult LoanService::approveLoan(const ApproveLoanRequest& req) {
    LoanOperationResult res;
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value() || (sessOpt->role != "TELLER" && sessOpt->role != "ADMIN")) {
        res.message = "BẢO MẬT: Bạn không có quyền phê duyệt khoản vay!";
        return res;
    }
    const auto& operatorSess = sessOpt.value();

    auto loanOpt = m_loanRepo.findById(req.loan_id);
    if (!loanOpt.has_value() ) {
        res.message = "Khoản vay không tồn tại hoặc đã được xử lý!";
        return res;
    }
    const auto& loan = loanOpt.value();

    // RBAC Matrix Guard: Teller chỉ được duyệt khoản vay <= 100 Triệu VNĐ
    if (operatorSess.role == "TELLER" && loan.getOriginalAmountVnd() > TELLER_APPROVAL_LIMIT_VND) {
        m_auditRepo.logAction(operatorSess.user_id, "APPROVAL_LIMIT_EXCEEDED", req.loan_id, "Amount > 100M VND");
        res.message = "Teller chỉ được duyệt vay <= 100.000.000 VNĐ. Khoản vay này yêu cầu Admin phê duyệt!";
        return res;
    }

    std::string newStatus = (req.action == "APPROVE") ? "APPROVED" : "REJECTED";
    std::string sql = "UPDATE loans SET status = ?, approved_by = ? WHERE loan_id = ? AND status = 'PENDING';";
    json args = json::array({
        {{"type", "text"}, {"value", newStatus}},
        {{"type", "text"}, {"value", operatorSess.user_id}},
        {{"type", "text"}, {"value", req.loan_id}}
    });

    json queryRes = m_tursoDb.execute(sql, args);
    if (queryRes.contains("results") && !queryRes["results"].empty()) {
        res.success = true;
        res.message = (newStatus == "APPROVED") ? "Đã phê duyệt khoản vay!" : "Đã từ chối đơn vay.";
        res.loan_id = req.loan_id;
        m_auditRepo.logAction(operatorSess.user_id, "LOAN_DECISION", req.loan_id, "Status: " + newStatus);
    } else {
        res.message = "Lỗi CSDL khi cập nhật trạng thái phê duyệt!";
    }

    return res;
}

// 3. GIẢI NGÂN KHOẢN VAY
LoanOperationResult LoanService::disburseLoan(const DisburseLoanRequest& req) {
    LoanOperationResult res;
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value() || (sessOpt->role != "TELLER" && sessOpt->role != "ADMIN")) {
        res.message = "BẢO MẬT: Bạn không có quyền thực hiện giải ngân!";
        return res;
    }

    auto loanOpt = m_loanRepo.findById(req.loan_id);
    if (!loanOpt.has_value()) {
        res.message = "Không tìm thấy hồ sơ khoản vay cần giải ngân!";
        return res;
    }
    const auto& loan = loanOpt.value();

    // ATOMIC BATCH TRANSACTION: Cộng tiền giải ngân vào CheckingAccount + Cập nhật 'DISBURSED'
    std::vector<std::pair<std::string, json>> batch;

    std::string sqlCredit = "UPDATE accounts SET balance_vnd = balance_vnd + ? WHERE account_id = ?;";
    json argsCredit = json::array({
        {{"type", "numeric"}, {"value", loan.getOriginalAmountVnd()}},
        {{"type", "text"},    {"value", req.target_account_id}}
    });
    batch.push_back({sqlCredit, argsCredit});

    std::string sqlUpdateLoan = "UPDATE loans SET status = 'DISBURSED', disbursed_account_id = ? WHERE loan_id = ?;";
    json argsUpdateLoan = json::array({
        {{"type", "text"}, {"value", req.target_account_id}},
        {{"type", "text"}, {"value", req.loan_id}}
    });
    batch.push_back({sqlUpdateLoan, argsUpdateLoan});

    json batchRes = m_tursoDb.executeBatch(batch);
    if (batchRes.contains("results") && !batchRes["results"].empty()) {
        res.success = true;
        res.message = "Giải ngân tiền vay thành công!";
        res.loan_id = req.loan_id;
        m_auditRepo.logAction(sessOpt->user_id, "DISBURSE_LOAN_SUCCESS", req.loan_id, "Disbursed: " + std::to_string(loan.getOriginalAmountVnd()) + " VND");
    } else {
        res.message = "Lỗi CSDL khi thực hiện giải ngân!";
    }

    return res;
}

} // namespace Banking