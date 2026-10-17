#ifndef LOANS_HPP
#define LOANS_HPP

#include<string>
#include<vector>
#include"Users/User.hpp"
#include "../../database/TursoDB.hpp" 

class LoanModel{
  private:
    std::string m_loanId;
    std::string m_accountId;      // Tài khoản thanh toán thụ hưởng / trừ nợ
    std::string m_borrowerUserId; // ID của khách hàng vay
    double m_loanAmount;          // Số tiền vay gốc
    double m_remainingAmount;     // Dư nợ còn lại
    double m_interestRate;        // Lãi suất %/năm
    int m_termMonths;             // Kỳ hạn (tháng)
    std::string m_status;         // "PENDING", "APPROVED", "REJECTED", "PAID_OFF"
    std::string m_startDate;
    std::string m_endDate;
    public:
    LoanModel(std::string loanId, std::string accountId, std::string borrowerUserId,
              double loanAmount, double interestRate, int termMonths,
              double remainingAmount = 0.0, std::string status = "PENDING",
              std::string startDate = "", std::string endDate = "")
        : m_loanId(loanId), m_accountId(accountId), m_borrowerUserId(borrowerUserId),
          m_loanAmount(loanAmount), m_interestRate(interestRate), m_termMonths(termMonths),
          m_remainingAmount(remainingAmount == 0.0 ? loanAmount : remainingAmount),
          m_status(status), m_startDate(startDate), m_endDate(endDate) {}
        bool approve(const UserModel& operatorUser);
        bool reject(const UserModel& operatorUser);
        bool payInstallment(double payAmount);
        double calculateMonthlyPayment() const;

    std::string getLoanId() const { return m_loanId; }
    std::string getAccountId() const { return m_accountId; }
    std::string getBorrowerUserId() const { return m_borrowerUserId; }
    double getLoanAmount() const { return m_loanAmount; }
    double getRemainingAmount() const { return m_remainingAmount; }
    double getInterestRate() const { return m_interestRate; }
    int getTermMonths() const { return m_termMonths; }
    std::string getStatus() const { return m_status; }
    };

#endif