#include "services/SavingsService.hpp"
#include <algorithm>

namespace Banking {

SavingsService::SavingsService(SavingsRepository& savingsRepo, AccountRepository& accRepo,
                               UserRepository& userRepo, AuditRepository& auditRepo,
                               SessionManager& sessionManager, TursoDB& tursoDb)
    : m_savingsRepo(savingsRepo), m_accRepo(accRepo), m_userRepo(userRepo),
      m_auditRepo(auditRepo), m_sessionManager(sessionManager), m_tursoDb(tursoDb) {}

// =========================================================================
// 1. MỞ SỔ TIẾT KIỆM
// =========================================================================
SavingsOperationResult SavingsService::openSavings(const OpenSavingsRequest& req) {
    SavingsOperationResult res;
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc đã hết hạn!";
        return res;
    }
    const auto& sess = sessOpt.value();

    // Xác thực PIN
    auto userOpt = m_userRepo.findById(sess.user_id);
    if (!userOpt || !userOpt->verifyPin(req.pin)) {
        res.message = "Mã PIN giao dịch không chính xác!";
        return res;
    }

    if (req.amount_vnd < 1000000) {
        res.message = "Số tiền gửi tối thiểu phải từ 1.000.000 VNĐ!";
        return res;
    }

    int64_t currentBal = m_accRepo.getBalance(sess.primary_account_id);
    if (currentBal < req.amount_vnd) {
        res.message = "Số dư tài khoản thanh toán không đủ!";
        return res;
    }

    double rate = (req.term_months >= 12) ? 6.1 : (req.term_months >= 6 ? 5.8 : 4.5);
    std::string savingsId = "SAV_" + std::to_string(std::time(nullptr));

    std::vector<std::pair<std::string, json>> batch;

    // Trừ tiền khỏi tài khoản thanh toán
    std::string sqlDeduct = "UPDATE accounts SET balance_vnd = balance_vnd - ? WHERE account_id = ? AND balance_vnd >= ?;";
    json argsDeduct = json::array({
        {{"type", "numeric"}, {"value", req.amount_vnd}},
        {{"type", "text"},    {"value", sess.primary_account_id}},
        {{"type", "numeric"}, {"value", req.amount_vnd}}
    });
    batch.push_back({sqlDeduct, argsDeduct});

    // Thêm sổ tiết kiệm mới
    std::string sqlInsert = 
        "INSERT INTO savings_accounts (savings_id, user_id, source_account_id, principal_vnd, "
        "interest_rate, term_months, maturity_option, status, start_date, maturity_date, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 'ACTIVE', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);";
    json argsInsert = json::array({
        {{"type", "text"},    {"value", savingsId}},
        {{"type", "text"},    {"value", sess.user_id}},
        {{"type", "text"},    {"value", sess.primary_account_id}},
        {{"type", "numeric"}, {"value", req.amount_vnd}},
        {{"type", "numeric"}, {"value", rate}},
        {{"type", "numeric"}, {"value", req.term_months}},
        {{"type", "text"},    {"value", req.maturity_option}}
    });
    batch.push_back({sqlInsert, argsInsert});

    json batchRes = m_tursoDb.executeBatch(batch);
    if (batchRes.contains("results") && !batchRes["results"].empty()) {
        res.success = true;
        res.message = "Mở sổ tiết kiệm thành công!";
        res.savings_id = savingsId;
        m_auditRepo.logAction(sess.user_id, "OPEN_SAVINGS_SUCCESS", savingsId, "Amount: " + std::to_string(req.amount_vnd) + " VND");
    } else {
        res.message = "Lỗi CSDL khi mở sổ tiết kiệm!";
    }

    return res;
}

// =========================================================================
// 2. TẤT TOÁN SỔ TIẾT KIỆM
// =========================================================================
SavingsOperationResult SavingsService::closeSavings(const CloseSavingsRequest& req) {
    SavingsOperationResult res;
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc hết hạn!";
        return res;
    }
    const auto& sess = sessOpt.value();

    // Xác thực PIN trước khi tất toán
    auto userOpt = m_userRepo.findById(sess.user_id);
    if (!userOpt || !userOpt->verifyPin(req.pin)) {
        res.message = "Mã PIN giao dịch không chính xác!";
        return res;
    }

    auto savingsOpt = m_savingsRepo.findById(req.savings_id);
    if (!savingsOpt || !savingsOpt->isActive() || savingsOpt->getUserId() != sess.user_id) {
        res.message = "Sổ tiết kiệm không tồn tại hoặc đã bị tất toán!";
        return res;
    }
    const auto& sav = savingsOpt.value();

    int64_t interestVnd = sav.calculateMaturityInterest();
    int64_t totalPayout = sav.getOriginalAmountVnd() + interestVnd;

    std::vector<std::pair<std::string, json>> batch;

    // Cộng tổng tiền (Gốc + Lãi) vào tài khoản thanh toán
    std::string sqlCredit = "UPDATE accounts SET balance_vnd = balance_vnd + ? WHERE account_id = ?;";
    json argsCredit = json::array({
        {{"type", "numeric"}, {"value", totalPayout}},
        {{"type", "text"},    {"value", sess.primary_account_id}}
    });
    batch.push_back({sqlCredit, argsCredit});

    // Cập nhật trạng thái sổ thành CLOSED
    std::string sqlClose = "UPDATE savings_accounts SET status = 'CLOSED' WHERE savings_id = ?;";
    json argsClose = json::array({{{"type", "text"}, {"value", sav.getAccountId()}}});
    batch.push_back({sqlClose, argsClose});

    json batchRes = m_tursoDb.executeBatch(batch);
    if (batchRes.contains("results") && !batchRes["results"].empty()) {
        res.success = true;
        res.message = "Tất toán sổ tiết kiệm thành công!";
        res.savings_id = sav.getAccountId();
        res.payout_total_vnd = totalPayout;
        res.interest_earned_vnd = interestVnd;
        m_auditRepo.logAction(sess.user_id, "CLOSE_SAVINGS_SUCCESS", sav.getAccountId(), "Total Payout: " + std::to_string(totalPayout) + " VND");
    } else {
        res.message = "Lỗi CSDL khi tất toán!";
    }

    return res;
}

// =========================================================================
// 3. XEM DANH SÁCH SỔ TIẾT KIỆM (TÍNH TỔNG GỐC VÀ TỔNG LÃI DỰ KIẾN)
// =========================================================================
GetSavingsListResult SavingsService::getSavingsList(const GetSavingsListRequest& req) {
    GetSavingsListResult res;

    // 1. Xác thực phiên làm việc
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc đã hết hạn hoặc không hợp lệ!";
        return res;
    }
    const auto& sess = sessOpt.value();

    // 2. Xác định Target User ID (Tự tra cứu hoặc Teller/Admin tra cứu giúp khách)
    std::string targetUserId = req.target_user_id.empty() ? sess.user_id : req.target_user_id;

    if (sess.role == "CUSTOMER" && targetUserId != sess.user_id) {
        res.message = "Lỗi phân quyền: Khách hàng chỉ được tra cứu sổ tiết kiệm của chính mình!";
        return res;
    }

    std::string filterStatus = req.status_filter.empty() ? "ACTIVE" : req.status_filter;

    // 3. Truy vấn danh sách từ Repository
    SavingsBook book = m_savingsRepo.findSavingsBookByUserId(targetUserId, filterStatus);
    const auto& accounts = book.getAccounts();

    int64_t sumPrincipal = 0;
    int64_t sumExpectedInterest = 0;

    for (const auto& acc : accounts) {
        SavingsDTO dto;
        dto.savings_id = acc.getAccountId();
        dto.user_id = targetUserId;
        dto.source_account_id = sess.primary_account_id;
        dto.principal_vnd = acc.getOriginalAmountVnd();
        dto.interest_rate = acc.getInterestRate();
        dto.term_months = acc.getTermMonths();
        
        // Tính toán lãi dự kiến khi đúng hạn: Gốc * (Lãi suất / 100) * (Số tháng / 12)
        dto.expected_interest_vnd = static_cast<int64_t>(
            dto.principal_vnd * (dto.interest_rate / 100.0) * (dto.term_months / 12.0)
        );
        
        dto.maturity_option = acc.getMaturityOption();
        dto.status = acc.getStatus();
        dto.start_date = ""; // Lấy từ DB nếu cần
        dto.maturity_date = acc.getMaturityDate();

        // Cộng dồn tổng gốc và tổng lãi dự kiến
        sumPrincipal += dto.principal_vnd;
        sumExpectedInterest += dto.expected_interest_vnd;

        res.items.push_back(dto);
    }

    res.success = true;
    res.message = "Truy vấn danh sách sổ tiết kiệm thành công!";
    res.total_principal_vnd = sumPrincipal;
    res.total_expected_interest_vnd = sumExpectedInterest;
    return res;
}

} // namespace Banking