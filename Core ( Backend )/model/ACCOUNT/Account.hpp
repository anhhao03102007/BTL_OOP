
#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include <string>
#include "../../database/TursoDB.hpp"

class Account {
private:
    std::string user_id;
    std::string account_id;
    double balance;
    std::string currency;
    std::string account_type;
    int status;
    std::string bank_code;

public:
    Account();

    Account(
        const std::string& user_id,
        const std::string& account_id,
        double balance,
        const std::string& currency,
        const std::string& account_type,
        int status,
        const std::string& bank_code
    );

    std::string getUserId() const;
    std::string getAccountId() const;
    double getBalance() const;
    std::string getCurrency() const;
    std::string getAccountType() const;
    int getStatus() const;
    std::string getBankCode() const;

    void setBalance(double balance);
    void setStatus(int status);

    bool create(TursoDB& db);
    bool read(TursoDB& db);
    bool update(TursoDB& db);
    bool remove(TursoDB& db);
};

class Savings_Account : public Account {
private:
    double Interest_rate;

public:
    Savings_Account();
    double getInterestRate() const;
    void setInterestRate(double rate);
};

class Term_Savings_Account : public Account {
private:
    int term_month;
    int savings_id;
    int Deposit_term;
    double interest_Rate;
    std::string maturity_option;

public:
    Term_Savings_Account();
    int getTermMonth() const;
    double getInterestRate() const;
};

class CheckingAccount : public Account {
private:
    double overdraft_limit;
    double daily_transfer_limit;
    double current_spent_today;

public:
    CheckingAccount();
    double getOverdraftLimit() const;
    double getDailyTransferLimit() const;
};

class Credit_Account : public Account {
private:
    double limit;
    double current_debt_balance;
    double overdue_interest;

public:
    Credit_Account();
    double getLimit() const;
    double getCurrentDebtBalance() const;
};

#endif
