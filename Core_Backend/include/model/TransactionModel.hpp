#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <string>
#include <memory>
#include <utility>
#include "security/AuthSecurity.hpp"

enum class BankCode {
    OOPBANK,
    VIETCOMBANK,
    BIDV,
    AGRIBANK,
    TECHCOMBANK,
    MBBANK,
    VPBANK,
    UNKNOWN
};

class BankUtils {
public:
    static std::string toString(BankCode bank) {
        switch (bank) {
            case BankCode::OOPBANK:     return "OOPBANK";
            case BankCode::VIETCOMBANK: return "VIETCOMBANK";
            case BankCode::BIDV:        return "BIDV";
            case BankCode::AGRIBANK:    return "AGRIBANK";
            case BankCode::TECHCOMBANK: return "TECHCOMBANK";
            case BankCode::MBBANK:      return "MBBANK";
            case BankCode::VPBANK:      return "VPBANK";
            default:                    return "UNKNOWN_BANK";
        }
    }

    static BankCode parse(const std::string& bank_str) {
        if (bank_str == "OOPBANK")     return BankCode::OOPBANK;
        if (bank_str == "VIETCOMBANK") return BankCode::VIETCOMBANK;
        if (bank_str == "BIDV")        return BankCode::BIDV;
        if (bank_str == "AGRIBANK")    return BankCode::AGRIBANK;
        if (bank_str == "TECHCOMBANK") return BankCode::TECHCOMBANK;
        if (bank_str == "MBBANK")      return BankCode::MBBANK;
        if (bank_str == "VPBANK")      return BankCode::VPBANK;
        return BankCode::UNKNOWN;
    }
};
// =============================================================
// 1. LỚP CƠ SỞ TRANSACTION (ABSTRACT BASE CLASS)
// =============================================================
class Transaction {
protected:
    std::string transaction_id;
    std::string sender_account_id;
    std::string receiver_account_id;
    double amount = 0.0;
    double fee = 0.0;
    std::string status = "PENDING";
    std::string description;

    std::string generateTxnRef() {
        AuthSecurity authSec;
        return authSec.generateOTP(8);
    }

public:
    Transaction(std::string sender, std::string receiver, double amt, std::string desc)
        : transaction_id(generateTxnRef()), 
          sender_account_id(std::move(sender)),
          receiver_account_id(std::move(receiver)), 
          amount(amt), 
          description(std::move(desc)) {}
    virtual bool execute();

    virtual ~Transaction() = default;

    // Getters Read-Only
    std::string getTransactionId() const { return transaction_id; }
    std::string getSenderAccountId() const { return sender_account_id; }
    std::string getReceiverAccountId() const { return receiver_account_id; }
    double getAmount() const { return amount; }
    double getFee() const { return fee; }
    double getTotalDeduct() const { return amount + fee; } // Tổng tiền trừ = Số tiền + Phí
    std::string getDescription() const { return description; }
    std::string getStatus() const { return status; }
    
    void setStatus(const std::string& new_status) { status = new_status; }

    // --- CÁC HÀM ẢO TẬN DỤNG ĐA HÌNH ---
    virtual std::string getTransactionType() const = 0;
    virtual double calculateFee() const = 0;
};

// =============================================================
// 2. LỚP CHUYỂN TIỀN (TRANSFER)
// =============================================================
class Transfer : public Transaction {
private:
    BankCode target_bank = BankCode::OOPBANK;

public:
    Transfer(std::string sender, std::string receiver, double amt, std::string desc, BankCode bank = BankCode::OOPBANK)
        : Transaction(std::move(sender), std::move(receiver), amt, std::move(desc)),
          target_bank(bank) 
    {
        this->fee = calculateFee(); // Tự động tính phí khi khởi tạo
    }

    bool execute() override ;

    double calculateFee() const override {
        if (target_bank == BankCode::OOPBANK) {
            return 0.0; // Miễn phí chuyển tiền nội bộ
        }
        return 9900.0;  // Phí chuyển tiền liên ngân hàng
    }

    std::string getTransactionType() const override {
        return (target_bank == BankCode::OOPBANK) ? "INTERNAL_TRANSFER" : "INTERBANK_TRANSFER";
    }

    BankCode getTargetBank() const { return target_bank; }
};

// =============================================================
// 3. LỚP RÚT TIỀN (WITHDRAW)
// =============================================================
class WithDraw : public Transaction {
private:
    BankCode atm_bank = BankCode::OOPBANK;

public:
    WithDraw(std::string account_id, double amt, std::string desc, BankCode atm_location = BankCode::OOPBANK)
        : Transaction(std::move(account_id), "ATM_" + BankUtils::toString(atm_location), amt, std::move(desc)),
          atm_bank(atm_location) 
    {
        this->fee = calculateFee(); // Tự động tính phí khi khởi tạo
    }
    bool execute() override ;

    double calculateFee() const override {
        if (atm_bank == BankCode::OOPBANK) {
            return 0.0; // Rút ATM nội mạng miễn phí
        }
        return 3300.0;  // Rút ATM ngoại mạng
    }

    std::string getTransactionType() const override {
        return (atm_bank == BankCode::OOPBANK) ? "ATM_WITHDRAW_INTERNAL" : "ATM_WITHDRAW_OFFNETWORK";
    }

    BankCode getAtmBank() const { return atm_bank; }
};
// =============================================================
// 4. LỚP NẠP TIỀN (DEPOSIT)
// =============================================================

class Deposit : public Transaction {
public:
    Deposit(std::string account_id, double amt, std::string desc)
        : Transaction(std::move(account_id), "DEPOSIT", amt, std::move(desc)) 
    {
        this->fee = calculateFee(); // Tự động tính phí khi khởi tạo
    }

    bool execute() override;

    double calculateFee() const override {
        return 0.0; // Nạp tiền miễn phí
    }

    std::string getTransactionType() const override {
        return "DEPOSIT";
    }
};

#endif // TRANSACTION_HPP