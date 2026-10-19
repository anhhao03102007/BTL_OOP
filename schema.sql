-- =========================================================================
-- SYSTEME OOP BANK - SCHEMA COMPLET DE LA BASE DE DONNEES (TursoDB / SQLite)
-- =========================================================================

-- 1. Table des Utilisateurs (users)
CREATE TABLE IF NOT EXISTS users (
    user_id TEXT PRIMARY KEY,
    user_name TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    salt TEXT NOT NULL,
    full_name TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    id_card_number TEXT UNIQUE NOT NULL,
    phone_number TEXT UNIQUE NOT NULL,
    role TEXT NOT NULL CHECK(role IN ('CUSTOMER', 'TELLER', 'ADMIN')),
    status TEXT NOT NULL CHECK(status IN ('ACTIVE', 'BLOCKED')),
    pin_hash TEXT DEFAULT '',
    failed_attempts INTEGER DEFAULT 0
);

-- 2. Table des Comptes Bancaires (accounts)
CREATE TABLE IF NOT EXISTS accounts (
    account_id TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    balance_vnd INTEGER NOT NULL DEFAULT 0 CHECK(balance_vnd >= 0),
    account_type TEXT NOT NULL CHECK(account_type IN ('CHECKING', 'SAVINGS', 'LOAN', 'CREDIT')),
    status TEXT NOT NULL CHECK(status IN ('ACTIVE', 'BLOCKED', 'CLOSED')),
    created_at TEXT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

-- 3. Table des Transactions Financières (transactions)
CREATE TABLE IF NOT EXISTS transactions (
    transaction_id TEXT PRIMARY KEY,
    sender_account_id TEXT NOT NULL,
    receiver_account_id TEXT NOT NULL,
    amount_vnd INTEGER NOT NULL CHECK(amount_vnd > 0),
    fee_vnd INTEGER NOT NULL DEFAULT 0 CHECK(fee_vnd >= 0),
    type TEXT NOT NULL,
    status TEXT NOT NULL CHECK(status IN ('PENDING', 'SUCCESS', 'FAILED')),
    description TEXT,
    post_balance_vnd INTEGER NOT NULL,
    created_at TEXT NOT NULL
);

-- 4. Table du Journal d'Audit (audit_logs - Append Only)
CREATE TABLE IF NOT EXISTS audit_logs (
    log_id TEXT PRIMARY KEY,
    operator_id TEXT NOT NULL,
    action TEXT NOT NULL,
    target_id TEXT NOT NULL,
    details TEXT,
    created_at TEXT NOT NULL
);

-- 5. Table de Configuration Système (system_config)
CREATE TABLE IF NOT EXISTS system_config (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at TEXT NOT NULL
);

-- 6. Table Clés d'Idempotence (idempotency_keys)
CREATE TABLE IF NOT EXISTS idempotency_keys (
    idempotency_key TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    action TEXT NOT NULL,
    response_json TEXT NOT NULL,
    created_at TEXT NOT NULL
);

-- 7. Table des Limites Journalières de Transaction (daily_transaction_limits)
CREATE TABLE IF NOT EXISTS daily_transaction_limits (
    user_id TEXT NOT NULL,
    txn_date TEXT NOT NULL, -- Format: "YYYY-MM-DD"
    total_amount_vnd INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (user_id, txn_date)
);

-- 8. Table des Factures de Services (bills)
CREATE TABLE IF NOT EXISTS bills (
    bill_id TEXT PRIMARY KEY,
    provider_id TEXT NOT NULL,
    provider_name TEXT NOT NULL,
    customer_code TEXT NOT NULL,
    customer_name TEXT NOT NULL,
    amount_vnd INTEGER NOT NULL CHECK(amount_vnd > 0),
    service_type TEXT NOT NULL CHECK(service_type IN ('ELECTRICITY', 'WATER', 'INTERNET', 'MOBILE_POSTPAID', 'TUITION')),
    status TEXT NOT NULL CHECK(status IN ('UNPAID', 'PAID', 'EXPIRED')),
    due_date TEXT NOT NULL,
    period TEXT NOT NULL
);

-- 9. Table des Comptes d'Épargne (savings_accounts)
CREATE TABLE IF NOT EXISTS savings_accounts (
    savings_id TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    source_account_id TEXT NOT NULL,
    principal_vnd INTEGER NOT NULL CHECK(principal_vnd >= 1000000),
    interest_rate REAL NOT NULL CHECK(interest_rate >= 0),
    term_months INTEGER NOT NULL CHECK(term_months > 0),
    maturity_option TEXT NOT NULL CHECK(maturity_option IN ('RENEW_ALL', 'RENEW_PRINCIPAL', 'CLOSE_TO_ACCOUNT')),
    status TEXT NOT NULL CHECK(status IN ('ACTIVE', 'CLOSED', 'EARLY_WITHDRAWN')),
    start_date TEXT NOT NULL,
    maturity_date TEXT NOT NULL,
    created_at TEXT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id),
    FOREIGN KEY (source_account_id) REFERENCES accounts(account_id)
);

-- 10. Table des Emprunts (loans)
CREATE TABLE IF NOT EXISTS loans (
    loan_id TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    amount_vnd INTEGER NOT NULL CHECK(amount_vnd > 0),
    interest_rate REAL NOT NULL CHECK(interest_rate >= 0),
    term_months INTEGER NOT NULL CHECK(term_months > 0),
    calc_method TEXT NOT NULL CHECK(calc_method IN ('FLAT', 'DECREASING')),
    collateral_info TEXT,
    status TEXT NOT NULL CHECK(status IN ('PENDING', 'APPROVED', 'REJECTED', 'DISBURSED', 'PAID')),
    created_by TEXT NOT NULL,
    approved_by TEXT,
    disbursed_account_id TEXT,
    created_at TEXT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

-- 11. Table des Échéanciers de Remboursement (loan_schedules)
CREATE TABLE IF NOT EXISTS loan_schedules (
    schedule_id TEXT PRIMARY KEY,
    loan_id TEXT NOT NULL,
    period_num INTEGER NOT NULL,
    due_date TEXT NOT NULL,
    principal_due_vnd INTEGER NOT NULL CHECK(principal_due_vnd >= 0),
    interest_due_vnd INTEGER NOT NULL CHECK(interest_due_vnd >= 0),
    penalty_vnd INTEGER NOT NULL DEFAULT 0 CHECK(penalty_vnd >= 0),
    status TEXT NOT NULL CHECK(status IN ('UNPAID', 'PAID', 'OVERDUE')),
    paid_at TEXT,
    FOREIGN KEY (loan_id) REFERENCES loans(loan_id) ON DELETE CASCADE
);

-- =========================================================================
-- INDEXES B-TREE POUR PERFORMANCES O(log N)
-- =========================================================================
CREATE INDEX IF NOT EXISTS idx_acc_user ON accounts(user_id, status);
CREATE INDEX IF NOT EXISTS idx_bills_unpaid ON bills(provider_id, customer_code, status);
CREATE INDEX IF NOT EXISTS idx_savings_user ON savings_accounts(user_id, status);
CREATE INDEX IF NOT EXISTS idx_loans_user ON loans(user_id, status);
CREATE INDEX IF NOT EXISTS idx_schedules_loan ON loan_schedules(loan_id, status);
CREATE INDEX IF NOT EXISTS idx_audit_operator ON audit_logs(operator_id);
CREATE INDEX IF NOT EXISTS idx_audit_created ON audit_logs(created_at);
-- =========================================================================
-- PATCH cho schema.sql hiện tại. Chạy MỘT lần, bằng Turso shell/dashboard
-- (không chạy qua app). Sao lưu trước nếu đã có dữ liệu thật.
-- =========================================================================

-- 1. users: đổi tên cột cho khớp với code (username / cccd / phone)
ALTER TABLE users RENAME COLUMN user_name TO username;
ALTER TABLE users RENAME COLUMN id_card_number TO cccd;
ALTER TABLE users RENAME COLUMN phone_number TO phone;

-- Khóa tạm do nhập sai mật khẩu / PIN (dùng thời điểm hết khóa, KHÔNG đổi status sang BLOCKED)
ALTER TABLE users ADD COLUMN locked_until INTEGER NOT NULL DEFAULT 0;
ALTER TABLE users ADD COLUMN pin_failed_attempts INTEGER NOT NULL DEFAULT 0;
ALTER TABLE users ADD COLUMN pin_locked_until INTEGER NOT NULL DEFAULT 0;

-- 2. accounts: code đọc cột interest_rate
ALTER TABLE accounts ADD COLUMN interest_rate REAL NOT NULL DEFAULT 0;

-- 3. loans: LoanDTO có monthly_installment_vnd
ALTER TABLE loans ADD COLUMN monthly_installment_vnd INTEGER NOT NULL DEFAULT 0;

-- 4. Bảng "chốt chặn" cho TursoDB::requireRowChanged()
CREATE TABLE IF NOT EXISTS _guard (v INTEGER NOT NULL);

-- 5. Idempotency theo từng người dùng (tránh người này dùng key của người kia)
DROP TABLE IF EXISTS idempotency_keys;
CREATE TABLE idempotency_keys (
    idempotency_key TEXT NOT NULL,
    user_id TEXT NOT NULL,
    action TEXT NOT NULL,
    response_json TEXT NOT NULL,
    created_at TEXT NOT NULL,
    PRIMARY KEY (user_id, idempotency_key)
);

-- 6. Hạn mức ngày được ép ngay trong DB: vượt 1 tỷ thì UPSERT lỗi và cả batch rollback
DROP TABLE IF EXISTS daily_transaction_limits;
CREATE TABLE daily_transaction_limits (
    user_id TEXT NOT NULL,
    txn_date TEXT NOT NULL,
    total_amount_vnd INTEGER NOT NULL DEFAULT 0 CHECK(total_amount_vnd <= 1000000000),
    PRIMARY KEY (user_id, txn_date)
);

-- 7. Index cho lịch sử giao dịch (truy vấn lọc theo người gửi / người nhận)
CREATE INDEX IF NOT EXISTS idx_txn_sender   ON transactions(sender_account_id, created_at);
CREATE INDEX IF NOT EXISTS idx_txn_receiver ON transactions(receiver_account_id, created_at);

-- 8. audit_logs chỉ được thêm, không sửa/xóa
CREATE TRIGGER IF NOT EXISTS trg_audit_no_update BEFORE UPDATE ON audit_logs
BEGIN SELECT RAISE(ABORT, 'audit_logs is append-only'); END;
CREATE TRIGGER IF NOT EXISTS trg_audit_no_delete BEFORE DELETE ON audit_logs
BEGIN SELECT RAISE(ABORT, 'audit_logs is append-only'); END;