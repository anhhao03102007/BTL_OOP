#include "repository/AccountRepository.hpp"
#include <ctime>

namespace Banking {

AccountRepository::AccountRepository(TursoDB& tursoDb) : m_tursoDb(tursoDb) {}

std::string AccountRepository::extractCell(const json& cell) {
    if (cell.is_object() && cell.contains("value") && !cell["value"].is_null()) {
        return cell["value"].get<std::string>();
    }
    return "";
}

// 1. Lấy tất cả tài khoản thuộc sở hữu của 1 User ID (Dạng DTO)
std::vector<AccountDTO> AccountRepository::getAccountsByUserId(const std::string& userId) {
    std::vector<AccountDTO> list;
    std::string sql = 
        "SELECT account_id, user_id, balance_vnd, account_type, status, "
        "COALESCE(interest_rate, 0.0), created_at "
        "FROM accounts WHERE user_id = ? ORDER BY created_at ASC;";

    json args = json::array({{{"type", "text"}, {"value", userId}}});
    json res = m_tursoDb.execute(sql, args);

    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array()) {
                for (const auto& r : rows) {
                    list.push_back(AccountDTO{
                        extractCell(r[0]),
                        extractCell(r[1]),
                        std::stoll(extractCell(r[2])),
                        extractCell(r[3]),
                        extractCell(r[4]),
                        std::stod(extractCell(r[5]).empty() ? "0.0" : extractCell(r[5])),
                        extractCell(r[6])
                    });
                }
            }
        }
    } catch (...) {}
    return list;
}

// 2. Nạp CheckingAccount Model lên RAM
std::optional<CheckingAccount> AccountRepository::findCheckingAccountById(const std::string& accountId) {
    std::string sql = 
        "SELECT account_id, user_id, balance_vnd, status, created_at "
        "FROM accounts WHERE account_id = ? AND account_type = 'CHECKING' LIMIT 1;";

    json args = json::array({{{"type", "text"}, {"value", accountId}}});
    json res = m_tursoDb.execute(sql, args);

    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                auto r = rows[0];
                return CheckingAccount(
                    extractCell(r[0]),
                    extractCell(r[1]),
                    std::stoll(extractCell(r[2])),
                    extractCell(r[3]),
                    extractCell(r[4])
                );
            }
        }
    } catch (...) {}
    return std::nullopt;
}

// 3. Thực thi Atomic Batch Mở tài khoản tiết kiệm mới & Trừ tiền tài khoản thanh toán
bool AccountRepository::executeOpenSavingsBatch(const std::string& senderAccId, int64_t senderPostBalance,
                                              const std::string& newSavingsAccId, const std::string& userId,
                                              int64_t depositAmount, double interestRate, const std::string& createdAt) {
    std::vector<std::pair<std::string, json>> batch;

    // 3a. Trừ tiền tài khoản thanh toán
    batch.push_back({
        "UPDATE accounts SET balance_vnd = ? WHERE account_id = ? AND balance_vnd >= ?;",
        json::array({
            {{"type", "numeric"}, {"value", senderPostBalance}},
            {{"type", "text"},    {"value", senderAccId}},
            {{"type", "numeric"}, {"value", depositAmount}}
        })
    });

    // 3b. Tạo mới tài khoản tiết kiệm
    batch.push_back({
        "INSERT INTO accounts (account_id, user_id, balance_vnd, account_type, status, interest_rate, created_at) "
        "VALUES (?, ?, ?, 'SAVINGS', 'ACTIVE', ?, ?);",
        json::array({
            {{"type", "text"},    {"value", newSavingsAccId}},
            {{"type", "text"},    {"value", userId}},
            {{"type", "numeric"}, {"value", depositAmount}},
            {{"type", "numeric"}, {"value", interestRate}},
            {{"type", "text"},    {"value", createdAt}}
        })
    });

    json res = m_tursoDb.executeBatch(batch);
    return res.contains("results") && !res["results"].empty();
}

// 4. Cập nhật trạng thái tài khoản (ACTIVE / LOCKED)
bool AccountRepository::updateAccountStatus(const std::string& accountId, const std::string& newStatus) {
    std::string sql = "UPDATE accounts SET status = ? WHERE account_id = ?;";
    json args = json::array({
        {{"type", "text"}, {"value", newStatus}},
        {{"type", "text"}, {"value", accountId}}
    });

    json res = m_tursoDb.execute(sql, args);
    return res.contains("results") && !res["results"].empty() && res["results"][0].value("type", "") == "ok";
}

// 5. Kiểm tra quyền sở hữu tài khoản
bool AccountRepository::isAccountOwnedByUser(const std::string& accountId, const std::string& userId) {
    std::string sql = "SELECT COUNT(*) FROM accounts WHERE account_id = ? AND user_id = ? LIMIT 1;";
    json args = json::array({
        {{"type", "text"}, {"value", accountId}},
        {{"type", "text"}, {"value", userId}}
    });

    json res = m_tursoDb.execute(sql, args);
    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                return std::stoi(extractCell(rows[0][0])) > 0;
            }
        }
    } catch (...) {}
    return false;
}

// 6. Tạo tài khoản thanh toán mặc định
bool AccountRepository::createCheckingAccount(const std::string& accountId, const std::string& userId) {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&now));
    
    return createNewAccount(accountId, userId, "CHECKING", 0, "ACTIVE", std::string(buf));
}

// 7. Tạo tài khoản mới tổng quát
bool AccountRepository::createNewAccount(const std::string& accountId, const std::string& userId,
                                      const std::string& accountType, int64_t initialBalanceVnd,
                                      const std::string& status, const std::string& createdAt) {
    std::string sql = 
        "INSERT INTO accounts (account_id, user_id, balance_vnd, account_type, status, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?);";
        
    json args = json::array({
        {{"type", "text"},    {"value", accountId}},
        {{"type", "text"},    {"value", userId}},
        {{"type", "numeric"}, {"value", initialBalanceVnd}},
        {{"type", "text"},    {"value", accountType}},
        {{"type", "text"},    {"value", status}},
        {{"type", "text"},    {"value", createdAt}}
    });

    json res = m_tursoDb.execute(sql, args);
    return res.contains("results") && !res["results"].empty() && res["results"][0].value("type", "") == "ok";
}

// 8. Lấy số dư tài khoản
int64_t AccountRepository::getBalance(const std::string& accountId) {
    std::string sql = "SELECT balance_vnd FROM accounts WHERE account_id = ? LIMIT 1;";
    json args = json::array({{{"type", "text"}, {"value", accountId}}});
    json res = m_tursoDb.execute(sql, args);
    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                return std::stoll(extractCell(rows[0][0]));
            }
        }
    } catch (...) {}
    return 0;
}

// 9. Rút/Trừ tiền tài khoản
bool AccountRepository::withdraw(const std::string& accountId, int64_t amountVnd) {
    std::string sql = "UPDATE accounts SET balance_vnd = balance_vnd - ? WHERE account_id = ? AND balance_vnd >= ?;";
    json args = json::array({
        {{"type", "numeric"}, {"value", amountVnd}},
        {{"type", "text"},    {"value", accountId}},
        {{"type", "numeric"}, {"value", amountVnd}}
    });
    json res = m_tursoDb.execute(sql, args);
    return res.contains("results") && !res["results"].empty() && res["results"][0].value("type", "") == "ok";
}

// 10. Truy vấn lịch sử giao dịch phân trang
TransactionHistoryResult AccountRepository::getHistoryPaginated(const std::string& accountId, int page, int limit) {
    TransactionHistoryResult res;
    res.current_page = page;

    int offset = (page - 1) * limit;
    if (offset < 0) offset = 0;

    // 1. Tính tổng số dòng lịch sử
    std::string countSql = "SELECT COUNT(*) FROM transactions WHERE sender_account_id = ? OR receiver_account_id = ?;";
    json countArgs = json::array({
        {{"type", "text"}, {"value", accountId}},
        {{"type", "text"}, {"value", accountId}}
    });
    
    json countRes = m_tursoDb.execute(countSql, countArgs);
    try {
        if (countRes.contains("results") && !countRes["results"].empty()) {
            auto rows = countRes["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                res.total_items = std::stoi(extractCell(rows[0][0]));
            }
        }
    } catch (...) {}

    res.total_pages = (res.total_items + limit - 1) / limit;
    if (res.total_pages == 0) res.total_pages = 1;

    // 2. Truy vấn danh sách giao dịch phân trang
    std::string sql = 
        "SELECT transaction_id, sender_account_id, receiver_account_id, amount_vnd, fee_vnd, "
        "type, status, description, post_balance_vnd, created_at "
        "FROM transactions WHERE sender_account_id = ? OR receiver_account_id = ? "
        "ORDER BY created_at DESC LIMIT ? OFFSET ?;";
        
    json args = json::array({
        {{"type", "text"},    {"value", accountId}},
        {{"type", "text"},    {"value", accountId}},
        {{"type", "numeric"}, {"value", limit}},
        {{"type", "numeric"}, {"value", offset}}
    });

    json queryRes = m_tursoDb.execute(sql, args);
    try {
        if (queryRes.contains("results") && !queryRes["results"].empty()) {
            auto rows = queryRes["results"][0]["response"]["result"]["rows"];
            if (rows.is_array()) {
                for (const auto& r : rows) {
                    res.items.push_back(TransactionItemDTO{
                        extractCell(r[0]), extractCell(r[1]), extractCell(r[2]),
                        std::stoll(extractCell(r[3])), std::stoll(extractCell(r[4])),
                        extractCell(r[5]), extractCell(r[6]), extractCell(r[7]),
                        std::stoll(extractCell(r[8])), extractCell(r[9])
                    });
                }
            }
        }
    } catch (...) {}

    res.success = true;
    res.message = "Lấy lịch sử giao dịch thành công!";
    return res;
}

} // namespace Banking