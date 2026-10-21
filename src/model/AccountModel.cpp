#include "model/AccountModel.hpp"
#include <limits>
#include <utility>

namespace Banking {

// =========================================================================
// 1. BASE CLASS: Account
// =========================================================================
Account::Account(std::string accountId, std::string userId, int64_t balanceVnd,
                 std::string status, std::string createdAt)
    : m_accountId(std::move(accountId)), m_userId(std::move(userId)),
      m_balanceVnd(balanceVnd), m_status(std::move(status)),
      m_createdAt(std::move(createdAt)) {}

bool Account::isActive() const { return m_status == "ACTIVE"; }
const std::string& Account::getAccountId() const { return m_accountId; }
const std::string& Account::getUserId() const { return m_userId; }
int64_t Account::getBalanceVnd() const { return m_balanceVnd; }

// =========================================================================
// 2. CHECKING ACCOUNT
// =========================================================================
CheckingAccount::CheckingAccount(std::string accountId, std::string userId, int64_t balanceVnd,
                                 std::string status, std::string createdAt, int64_t overdraftLimit)
    : Account(std::move(accountId), std::move(userId), balanceVnd, std::move(status), std::move(createdAt)),
      m_overdraftLimitVnd(overdraftLimit > 0 ? overdraftLimit : 0) {}

bool CheckingAccount::canWithdraw(int64_t amountVnd) const {
    if (!isActive() || amountVnd <= 0) return false;
    // amount - limit is safe because both values are non-negative and amount > 0.
    return m_balanceVnd >= amountVnd - m_overdraftLimitVnd;
}

bool CheckingAccount::withdraw(int64_t amountVnd) {
    if (!canWithdraw(amountVnd)) return false;
    m_balanceVnd -= amountVnd;
    return true;
}

bool CheckingAccount::deposit(int64_t amountVnd) {
    if (!isActive() || amountVnd <= 0) return false;
    if (m_balanceVnd > 0 && amountVnd > std::numeric_limits<int64_t>::max() - m_balanceVnd) {
        return false;
    }
    m_balanceVnd += amountVnd;
    return true;
}

std::string CheckingAccount::getAccountType() const { return "CHECKING"; }

// =========================================================================
// 3. DEMAND SAVINGS ACCOUNT
// =========================================================================
DemandSavingsAccount::DemandSavingsAccount(std::string accountId, std::string userId, int64_t balanceVnd,
                                           std::string status, std::string createdAt)
    : Account(std::move(accountId), std::move(userId), balanceVnd, std::move(status), std::move(createdAt)) {}

bool DemandSavingsAccount::canWithdraw(int64_t amountVnd) const {
    if (!isActive() || amountVnd <= 0) return false;
    return m_balanceVnd >= amountVnd;
}

bool DemandSavingsAccount::withdraw(int64_t amountVnd) {
    if (!canWithdraw(amountVnd)) return false;
    m_balanceVnd -= amountVnd;
    return true;
}

bool DemandSavingsAccount::deposit(int64_t amountVnd) {
    if (!isActive() || amountVnd <= 0) return false;
    if (m_balanceVnd > 0 && amountVnd > std::numeric_limits<int64_t>::max() - m_balanceVnd) {
        return false;
    }
    m_balanceVnd += amountVnd;
    return true;
}

int64_t DemandSavingsAccount::calculateDailyInterest() const {
    if (m_balanceVnd <= 0) return 0;
    return static_cast<int64_t>((m_balanceVnd * (m_annualRate / 100.0)) / 365.0);
}

std::string DemandSavingsAccount::getAccountType() const { return "DEMAND_SAVINGS"; }

// =========================================================================
// 4. TERM SAVINGS ACCOUNT
// =========================================================================
TermSavingsAccount::TermSavingsAccount(std::string accountId, std::string userId, int64_t principalVnd,
                                       double rate, int termMonths, std::string maturityDate,
                                       std::string option, std::string status, std::string createdAt)
    : Account(std::move(accountId), std::move(userId), principalVnd, std::move(status), std::move(createdAt)),
      m_interestRate(rate), m_termMonths(termMonths),
      m_maturityDate(std::move(maturityDate)), m_maturityOption(std::move(option)) {}

bool TermSavingsAccount::canWithdraw(int64_t amountVnd) const {
    return isActive() && amountVnd == m_balanceVnd;
}

bool TermSavingsAccount::withdraw(int64_t amountVnd) {
    if (!canWithdraw(amountVnd)) return false;
    m_balanceVnd = 0;
    m_status = "CLOSED";
    return true;
}

bool TermSavingsAccount::deposit(int64_t) { return false; }

int64_t TermSavingsAccount::calculateMaturityInterest() const {
    if (m_balanceVnd <= 0 || m_interestRate <= 0.0 || m_termMonths <= 0) return 0;
    return static_cast<int64_t>(m_balanceVnd * (m_interestRate / 100.0) * (m_termMonths / 12.0));
}

int64_t TermSavingsAccount::calculateEarlyWithdrawalInterest(int actualDaysHeld) const {
    if (actualDaysHeld <= 0 || m_balanceVnd <= 0) {
        return 0;
    }

    constexpr double DEMAND_RATE_ANNUAL = 0.1;
    double interest = static_cast<double>(m_balanceVnd) 
                    * (DEMAND_RATE_ANNUAL / 100.0) 
                    * (static_cast<double>(actualDaysHeld) / 365.0);

    return static_cast<int64_t>(interest);
}

std::string TermSavingsAccount::getAccountType() const { return "TERM_SAVINGS"; }

// =========================================================================
// 5. CREDIT ACCOUNT
// =========================================================================
CreditAccount::CreditAccount(std::string accountId, std::string userId, int64_t usedAmountVnd,
                             int64_t creditLimitVnd, double rate, std::string status, std::string createdAt)
    : Account(std::move(accountId), std::move(userId), usedAmountVnd, std::move(status), std::move(createdAt)),
      m_creditLimitVnd(creditLimitVnd), m_interestRate(rate) {}

bool CreditAccount::canWithdraw(int64_t amountVnd) const {
    if (!isActive() || amountVnd <= 0 || m_balanceVnd < 0 || m_creditLimitVnd < 0) return false;
    return m_balanceVnd <= m_creditLimitVnd &&
           amountVnd <= m_creditLimitVnd - m_balanceVnd;
}

bool CreditAccount::withdraw(int64_t amountVnd) {
    if (!canWithdraw(amountVnd)) return false;
    m_balanceVnd += amountVnd;
    return true;
}

bool CreditAccount::deposit(int64_t amountVnd) {
    if (!isActive() || amountVnd <= 0) return false;
    m_balanceVnd = amountVnd >= m_balanceVnd ? 0 : m_balanceVnd - amountVnd;
    return true;
}

std::string CreditAccount::getAccountType() const { return "CREDIT"; }

int64_t CreditAccount::getAvailableCreditVnd() const {
    return m_creditLimitVnd - m_balanceVnd;
}

} // namespace Banking
