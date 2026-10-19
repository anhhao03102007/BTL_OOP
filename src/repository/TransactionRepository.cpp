#include "repository/TransactionRepository.hpp"

namespace Banking {

TransactionRepository::TransactionRepository(TursoDB& tursoDb) : m_tursoDb(tursoDb) {}

std::string TransactionRepository::extractCell(const json& cell) {
    if (cell.is_object() && cell.contains("value") && !cell["value"].is_null()) {
        return cell["value"].get<std::string>();
    }
    return "";
}

// =========================================================================
// THỰC THI CÁC HÀM TRA CỨU SỐ DƯ PHỤC VỤ GIAO DỊCH
// =========================================================================

// 1. Lấy số dư khả dụng thực tế từ CSDL
int64_t TransactionRepository::getAccountBalance(const std::string& accountId) {
    std::string sql = "SELECT balance_vnd FROM accounts WHERE account_id = ? LIMIT 1;";
    json args = json::array({{{"type", "text"}, {"value", accountId}}});

    json res = m_tursoDb.execute(sql, args);
    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                std::string valStr = extractCell(rows[0][0]);
                return valStr.empty() ? 0 : std::stoll(valStr);
            }
        }
    } catch (...) {}
    return 0;
}

// 2. Kiểm tra tài khoản có đủ số dư để chi trả
bool TransactionRepository::hasEnoughBalance(const std::string& accountId, int64_t requiredAmountVnd) {
    if (requiredAmountVnd <= 0) return false;

    std::string sql = "SELECT COUNT(*) FROM accounts WHERE account_id = ? AND balance_vnd >= ? LIMIT 1;";
    json args = json::array({
        {{"type", "text"},    {"value", accountId}},
        {{"type", "numeric"}, {"value", requiredAmountVnd}}
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

// 3. Tra cứu số dư của tài khoản còn đang ACTIVE
std::optional<int64_t> TransactionRepository::getActiveAccountBalance(const std::string& accountId) {
    std::string sql = "SELECT balance_vnd FROM accounts WHERE account_id = ? AND status = 'ACTIVE' LIMIT 1;";
    json args = json::array({{{"type", "text"}, {"value", accountId}}});

    json res = m_tursoDb.execute(sql, args);
    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                std::string valStr = extractCell(rows[0][0]);
                return std::stoll(valStr.empty() ? "0" : valStr);
            }
        }
    } catch (...) {}
    return std::nullopt;
}

// =========================================================================
// THỰC THI CÁC HÀM LƯU GIAO DỊCH
// =========================================================================

bool TransactionRepository::saveTransfer(const TransferTransaction& txn, const std::string& senderUserId,
                                          const std::string& txnDate, const std::string& idempotencyKey,
                                          int64_t postBalanceVnd, const std::string& responseJson) {
    std::vector<std::pair<std::string, json>> batch;

    batch.push_back({
        "UPDATE accounts SET balance_vnd = balance_vnd - ? WHERE account_id = ? AND balance_vnd >= ?;",
        json::array({
            {{"type", "numeric"}, {"value", txn.getTotalDeductVnd()}},
            {{"type", "text"},    {"value", txn.getSenderAccountId()}},
            {{"type", "numeric"}, {"value", txn.getTotalDeductVnd()}}
        })
    });

    batch.push_back({
        "UPDATE accounts SET balance_vnd = balance_vnd + ? WHERE account_id = ?;",
        json::array({
            {{"type", "numeric"}, {"value", txn.getAmountVnd()}},
            {{"type", "text"},    {"value", txn.getReceiverAccountId()}}
        })
    });

    batch.push_back({
        "INSERT INTO transactions (transaction_id, sender_account_id, receiver_account_id, amount_vnd, fee_vnd, type, status, description, post_balance_vnd, created_at) VALUES (?, ?, ?, ?, ?, ?, 'SUCCESS', ?, ?, ?);",
        json::array({
            {{"type", "text"},    {"value", txn.getTransactionId()}},
            {{"type", "text"},    {"value", txn.getSenderAccountId()}},
            {{"type", "text"},    {"value", txn.getReceiverAccountId()}},
            {{"type", "numeric"}, {"value", txn.getAmountVnd()}},
            {{"type", "numeric"}, {"value", txn.getFeeVnd()}},
            {{"type", "text"},    {"value", txn.getTransactionType()}},
            {{"type", "text"},    {"value", txn.getDescription()}},
            {{"type", "numeric"}, {"value", postBalanceVnd}},
            {{"type", "text"},    {"value", txn.getCreatedAt()}}
        })
    });

    batch.push_back({
        "INSERT INTO daily_transaction_limits (user_id, txn_date, total_amount_vnd) VALUES (?, ?, ?) ON CONFLICT(user_id, txn_date) DO UPDATE SET total_amount_vnd = total_amount_vnd + ?;",
        json::array({
            {{"type", "text"},    {"value", senderUserId}},
            {{"type", "text"},    {"value", txnDate}},
            {{"type", "numeric"}, {"value", txn.getAmountVnd()}},
            {{"type", "numeric"}, {"value", txn.getAmountVnd()}}
        })
    });

    if (!idempotencyKey.empty()) {
        batch.push_back({
            "INSERT INTO idempotency_keys (idempotency_key, user_id, action, response_json, created_at) VALUES (?, ?, 'TRANSFER', ?, ?);",
            json::array({
                {{"type", "text"}, {"value", idempotencyKey}},
                {{"type", "text"}, {"value", senderUserId}},
                {{"type", "text"}, {"value", responseJson}},
                {{"type", "text"}, {"value", txn.getCreatedAt()}}
            })
        });
    }

    json res = m_tursoDb.executeBatch(batch);
    return res.contains("results") && !res["results"].empty();
}

bool TransactionRepository::saveBillPayment(const BillPaymentTransaction& txn, int64_t postBalanceVnd) {
    std::vector<std::pair<std::string, json>> batch;

    batch.push_back({
        "UPDATE accounts SET balance_vnd = balance_vnd - ? WHERE account_id = ? AND balance_vnd >= ?;",
        json::array({
            {{"type", "numeric"}, {"value", txn.getTotalDeductVnd()}},
            {{"type", "text"},    {"value", txn.getPayerAccountId()}},
            {{"type", "numeric"}, {"value", txn.getTotalDeductVnd()}}
        })
    });

    batch.push_back({
        "UPDATE bills SET status = 'PAID' WHERE bill_id = ?;",
        json::array({{{"type", "text"}, {"value", txn.getBillId()}}})
    });

    batch.push_back({
        "INSERT INTO transactions (transaction_id, sender_account_id, receiver_account_id, amount_vnd, fee_vnd, type, status, description, post_balance_vnd, created_at) VALUES (?, ?, 'SYSTEM_BILL', ?, ?, ?, 'SUCCESS', ?, ?, ?);",
        json::array({
            {{"type", "text"},    {"value", txn.getTransactionId()}},
            {{"type", "text"},    {"value", txn.getPayerAccountId()}},
            {{"type", "numeric"}, {"value", txn.getAmountVnd()}},
            {{"type", "numeric"}, {"value", txn.getFeeVnd()}},
            {{"type", "text"},    {"value", txn.getTransactionType()}},
            {{"type", "text"},    {"value", txn.getDescription()}},
            {{"type", "numeric"}, {"value", postBalanceVnd}},
            {{"type", "text"},    {"value", txn.getCreatedAt()}}
        })
    });

    json res = m_tursoDb.executeBatch(batch);
    return res.contains("results") && !res["results"].empty();
}

std::optional<std::string> TransactionRepository::getIdempotentResponse(const std::string& key) {
    std::string sql = "SELECT response_json FROM idempotency_keys WHERE idempotency_key = ? LIMIT 1;";
    json args = json::array({{{"type", "text"}, {"value", key}}});
    json res = m_tursoDb.execute(sql, args);
    try {
        if (res.contains("results") && !res["results"].empty()) {
            auto rows = res["results"][0]["response"]["result"]["rows"];
            if (rows.is_array() && !rows.empty()) {
                return extractCell(rows[0][0]);
            }
        }
    } catch (...) {}
    return std::nullopt;
}

int64_t TransactionRepository::getDailyTotalVnd(const std::string& userId, const std::string& txnDate) {
    std::string sql = "SELECT total_amount_vnd FROM daily_transaction_limits WHERE user_id = ? AND txn_date = ? LIMIT 1;";
    json args = json::array({
        {{"type", "text"}, {"value", userId}},
        {{"type", "text"}, {"value", txnDate}}
    });
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

bool TransactionRepository::executeAtomicTransfer(const TransferTransaction& txn, const std::string& senderUserId,
                                                  const std::string& txnDate, const std::string& idempotencyKey,
                                                  int64_t postBalanceVnd, const std::string& responseJson) {
    return saveTransfer(txn, senderUserId, txnDate, idempotencyKey, postBalanceVnd, responseJson);
}

bool TransactionRepository::saveWithdrawBatch(const WithdrawTransaction& txn, int64_t postBalanceVnd) {
    std::vector<std::pair<std::string, json>> batch;
    batch.push_back({
        "UPDATE accounts SET balance_vnd = balance_vnd - ? WHERE account_id = ? AND balance_vnd >= ?;",
        json::array({
            {{"type", "numeric"}, {"value", txn.getTotalDeductVnd()}},
            {{"type", "text"},    {"value", txn.getSourceAccountId()}},
            {{"type", "numeric"}, {"value", txn.getTotalDeductVnd()}}
        })
    });
    batch.push_back({
        "INSERT INTO transactions (transaction_id, sender_account_id, receiver_account_id, amount_vnd, fee_vnd, type, status, description, post_balance_vnd, created_at) VALUES (?, ?, 'ATM_COUNTER', ?, ?, ?, 'SUCCESS', ?, ?, ?);",
        json::array({
            {{"type", "text"},    {"value", txn.getTransactionId()}},
            {{"type", "text"},    {"value", txn.getSourceAccountId()}},
            {{"type", "numeric"}, {"value", txn.getAmountVnd()}},
            {{"type", "numeric"}, {"value", txn.getFeeVnd()}},
            {{"type", "text"},    {"value", txn.getTransactionType()}},
            {{"type", "text"},    {"value", txn.getDescription()}},
            {{"type", "numeric"}, {"value", postBalanceVnd}},
            {{"type", "text"},    {"value", txn.getCreatedAt()}}
        })
    });
    json res = m_tursoDb.executeBatch(batch);
    return res.contains("results") && !res["results"].empty();
}

bool TransactionRepository::saveDepositBatch(const DepositTransaction& txn, int64_t postBalanceVnd) {
    std::vector<std::pair<std::string, json>> batch;
    batch.push_back({
        "UPDATE accounts SET balance_vnd = balance_vnd + ? WHERE account_id = ?;",
        json::array({
            {{"type", "numeric"}, {"value", txn.getAmountVnd()}},
            {{"type", "text"},    {"value", txn.getTargetAccountId()}}
        })
    });
    batch.push_back({
        "INSERT INTO transactions (transaction_id, sender_account_id, receiver_account_id, amount_vnd, fee_vnd, type, status, description, post_balance_vnd, created_at) VALUES (?, 'COUNTER_CASH', ?, ?, 0, 'DEPOSIT_COUNTER', 'SUCCESS', ?, ?, ?);",
        json::array({
            {{"type", "text"},    {"value", txn.getTransactionId()}},
            {{"type", "text"},    {"value", txn.getTargetAccountId()}},
            {{"type", "numeric"}, {"value", txn.getAmountVnd()}},
            {{"type", "text"},    {"value", txn.getDescription()}},
            {{"type", "numeric"}, {"value", postBalanceVnd}},
            {{"type", "text"},    {"value", txn.getCreatedAt()}}
        })
    });
    json res = m_tursoDb.executeBatch(batch);
    return res.contains("results") && !res["results"].empty();
}

bool TransactionRepository::saveBillPaymentBatch(const BillPaymentTransaction& txn, int64_t postBalanceVnd) {
    return saveBillPayment(txn, postBalanceVnd);
}

} // namespace Banking