#include "services/LoansService.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>

namespace Banking {

LoanService::LoanService(LoanRepository& loanRepo, AccountRepository& accRepo,
                         UserRepository& userRepo, AuditRepository& auditRepo,
                         SessionManager& sessionManager, TursoDB& tursoDb)
    : m_loanRepo(loanRepo), m_accRepo(accRepo), m_userRepo(userRepo),
      m_auditRepo(auditRepo), m_sessionManager(sessionManager), m_tursoDb(tursoDb) {}

// Đọc các khoản vay thuộc session hiện tại để nạp vào giao diện khách hàng.
std::vector<LoanModel> LoanService::getLoansForSession(const std::string& sessionToken) {
    const auto session = m_sessionManager.getSession(sessionToken);
    if (!session.has_value()) return {};
    return m_loanRepo.findByUserId(session->user_id);
}

StaffLoanApplicationsResult LoanService::getApplicationsForStaff(const std::string& sessionToken) {
    StaffLoanApplicationsResult result;
    const auto session = m_sessionManager.getSession(sessionToken);
    if (!session.has_value() || (session->role != "TELLER" && session->role != "ADMIN")) {
        result.message = "Bạn không có quyền xem danh sách hồ sơ vay.";
        return result;
    }

    for (const LoanModel& loan : m_loanRepo.findAll()) {
        StaffLoanApplicationDTO item;
        item.loan_id = loan.getLoanId();
        item.user_id = loan.getUserId();
        item.amount_vnd = loan.getOriginalAmountVnd();
        item.interest_rate = loan.getAnnualInterestRate();
        item.term_months = loan.getTermMonths();
        item.calc_method = loan.getCalcMethod();
        item.collateral_info = loan.getCollateralInfo();
        item.status = loan.getStatus();
        item.created_at = loan.getCreatedAt();
        const auto customer = m_userRepo.findById(loan.getUserId());
        item.customer_name = customer ? customer->getFullName() : loan.getUserId();
        item.customer_phone = customer ? customer->getPhone() : "";
        result.applications.push_back(std::move(item));
    }

    result.success = true;
    result.message = "Đã tải danh sách hồ sơ vay.";
    return result;
}

json LoanService::getAvailableTerms() {
    const json defaults = json::array({
        {{"months", 3}, {"enabled", true}, {"rate", 10.5}},
        {{"months", 6}, {"enabled", true}, {"rate", 11.0}},
        {{"months", 12}, {"enabled", true}, {"rate", 12.0}}
    });
    const json response = m_tursoDb.execute(
        "SELECT value FROM system_config WHERE key = ? LIMIT 1;",
        json::array({{{"type", "text"}, {"value", "loan.terms"}}}));
    try {
        const auto rows = response["results"][0]["response"]["result"]["rows"];
        if (!rows.is_array() || rows.empty() || !rows[0].is_array() || rows[0].empty()) return defaults;
        const auto& cell = rows[0][0];
        if (!cell.is_object() || !cell.contains("value") || cell["value"].is_null()) return defaults;
        const std::string encoded = cell["value"].is_string()
            ? cell["value"].get<std::string>() : cell["value"].dump();
        const json terms = json::parse(encoded);
        if (!terms.is_array() || terms.empty()) return defaults;
        return terms;
    } catch (...) {
        return defaults;
    }
}

// 1. TẠO ĐƠN XIN VAY
LoanOperationResult LoanService::applyLoan(const ApplyLoanRequest& req) {
    LoanOperationResult res;
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc hết hạn!";
        return res;
    }
    const auto& sess = sessOpt.value();

    if (sess.role != "CUSTOMER") {
        res.message = "Chỉ khách hàng mới có thể nộp đơn xin vay.";
        return res;
    }
    if (req.amount_vnd <= 0 || req.term_months < 1 || req.term_months > 360 ||
        (req.calc_method != "FLAT" && req.calc_method != "DECREASING")) {
        res.message = "Số tiền, kỳ hạn hoặc phương thức tính khoản vay không hợp lệ.";
        return res;
    }

    static std::atomic<std::uint64_t> loanSequence{0};
    const auto nowMicros = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string loanId = "LOAN_" + std::to_string(nowMicros) + "_" +
                         std::to_string(loanSequence.fetch_add(1, std::memory_order_relaxed));
    const json terms = getAvailableTerms();
    double configuredRate = 0.0;
    bool termEnabled = false;
    for (const auto& term : terms) {
        if (!term.is_object() || term.value("months", 0) != req.term_months) continue;
        termEnabled = term.value("enabled", false);
        configuredRate = term.value("rate", 0.0);
        break;
    }
    if (!termEnabled || configuredRate <= 0.0 || configuredRate > 50.0) {
        res.message = "Kỳ hạn vay này hiện không được mở hoặc có cấu hình lãi suất không hợp lệ.";
        return res;
    }

    std::string sqlLoan = 
        "INSERT INTO loans (loan_id, user_id, amount_vnd, interest_rate, term_months, "
        "calc_method, collateral_info, status, created_by, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 'PENDING', ?, CURRENT_TIMESTAMP);";

    json argsLoan = json::array({
        {{"type", "text"},    {"value", loanId}},
        {{"type", "text"},    {"value", sess.user_id}},
        {{"type", "numeric"}, {"value", req.amount_vnd}},
        {{"type", "numeric"}, {"value", configuredRate}},
        {{"type", "numeric"}, {"value", req.term_months}},
        {{"type", "text"},    {"value", req.calc_method}},
        {{"type", "text"},    {"value", req.collateral_info}},
        {{"type", "text"},    {"value", sess.user_id}}
    });

    json dbRes = m_tursoDb.execute(sqlLoan, argsLoan);
    if (dbRes.contains("results") && !dbRes["results"].empty() &&
        dbRes["results"][0].is_object() && dbRes["results"][0].value("type", "") == "ok") {
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

    if (req.action != "APPROVE" && req.action != "REJECT") {
        res.message = "Quyết định khoản vay không hợp lệ.";
        return res;
    }
    if (loan.getStatus() != "PENDING") {
        res.message = "Khoản vay đã được xử lý trước đó.";
        return res;
    }
    if (req.action == "APPROVE") {
        res.message = "Chưa thể phê duyệt: hệ thống chưa có nơi lưu và kiểm tra tài liệu vay.";
        return res;
    }
    std::string newStatus = "REJECTED";
    std::string sql = "UPDATE loans SET status = ?, approved_by = ? WHERE loan_id = ? AND status = 'PENDING';";
    json args = json::array({
        {{"type", "text"}, {"value", newStatus}},
        {{"type", "text"}, {"value", operatorSess.user_id}},
        {{"type", "text"}, {"value", req.loan_id}}
    });

    json queryRes = m_tursoDb.execute(sql, args);
    if (queryRes.contains("results") && !queryRes["results"].empty() &&
        queryRes["results"][0].is_object() && queryRes["results"][0].value("type", "") == "ok") {
        res.success = true;
        res.message = (newStatus == "APPROVED") ? "Đã phê duyệt khoản vay!" : "Đã từ chối đơn vay.";
        res.loan_id = req.loan_id;
        const std::string details = "Status: " + newStatus +
            (req.reason.empty() ? std::string{} : "; Reason: " + req.reason);
        m_auditRepo.logAction(operatorSess.user_id, "LOAN_DECISION", req.loan_id, details);
    } else {
        res.message = "Lỗi CSDL khi cập nhật trạng thái phê duyệt!";
    }

    return res;
}

// 3. GIẢI NGÂN KHOẢN VAY
LoanOperationResult LoanService::disburseLoan(const DisburseLoanRequest& req) {
    LoanOperationResult res;
    const auto session = m_sessionManager.getSession(req.session_token);
    if (!session.has_value()) {
        res.message = "Phiên làm việc đã hết hạn!";
        return res;
    }
    if (session->role != "CUSTOMER" && session->role != "TELLER" && session->role != "ADMIN") {
        res.message = "Bạn không có quyền thực hiện giải ngân!";
        return res;
    }

    const auto loanOpt = m_loanRepo.findById(req.loan_id);
    if (!loanOpt.has_value() || loanOpt->getStatus() != "APPROVED") {
        res.message = "Khoản vay không tồn tại hoặc chưa được phê duyệt để giải ngân!";
        return res;
    }
    const LoanModel& loan = loanOpt.value();
    if (req.target_account_id.empty()) {
        res.message = "Cần chọn tài khoản nhận tiền giải ngân.";
        return res;
    }

    if (session->role == "CUSTOMER") {
        if (session->user_id != loan.getUserId()) {
            res.message = "Khách hàng chỉ được giải ngân khoản vay của chính mình.";
            return res;
        }
        const auto user = m_userRepo.findById(session->user_id);
        if (!user || !user->verifyPin(req.pin)) {
            res.message = "Mã PIN giao dịch không chính xác!";
            return res;
        }
    }

    const auto accounts = m_accRepo.getAccountsByUserId(loan.getUserId());
    const auto target = std::find_if(accounts.begin(), accounts.end(), [&](const AccountDTO& account) {
        return account.account_id == req.target_account_id &&
               account.account_type == "CHECKING" && account.status == "ACTIVE";
    });
    if (target == accounts.end()) {
        res.message = "Tài khoản nhận phải là tài khoản thanh toán đang hoạt động của người vay.";
        return res;
    }

    std::vector<std::pair<std::string, json>> batch;
    batch.push_back({
        "UPDATE accounts SET balance_vnd = balance_vnd + ? WHERE account_id = ? AND user_id = ? AND account_type = 'CHECKING' AND status = 'ACTIVE';",
        json::array({
            {{"type", "numeric"}, {"value", loan.getOriginalAmountVnd()}},
            {{"type", "text"}, {"value", req.target_account_id}},
            {{"type", "text"}, {"value", loan.getUserId()}}
        })
    });
    batch.push_back({
        "UPDATE loans SET status = 'DISBURSED', disbursed_account_id = ? WHERE loan_id = ? AND status = 'APPROVED';",
        json::array({
            {{"type", "text"}, {"value", req.target_account_id}},
            {{"type", "text"}, {"value", req.loan_id}}
        })
    });

    const json batchRes = m_tursoDb.executeBatch(batch);
    bool succeeded = batchRes.contains("results") && batchRes["results"].is_array() && !batchRes["results"].empty();
    if (succeeded) {
        for (const auto& result : batchRes["results"]) {
            if (!result.is_object() || result.value("type", "") != "ok") {
                succeeded = false;
                break;
            }
        }
    }
    if (succeeded) {
        res.success = true;
        res.message = "Giải ngân tiền vay thành công!";
        res.loan_id = req.loan_id;
        m_auditRepo.logAction(session->user_id, "DISBURSE_LOAN_SUCCESS", req.loan_id,
                              "Disbursed: " + std::to_string(loan.getOriginalAmountVnd()) + " VND");
    } else {
        res.message = "Lỗi CSDL khi thực hiện giải ngân!";
    }
    return res;
}

} // namespace Banking
