#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include<string>
#include "C:\Users\Thinkpad\OneDrive\文档\BTL_OOP\Core ( Backend )\database\TursoDB.hpp"
#include<map>
class Account {
    private : 
        std::string user_id;
        std::string account_id;
        double balance;
        std::string currency;
        std::string account_type;
        int status;
};

class Savings_Account :public Account{
    private :
        double Interest_rate;
    
};

class Term_Savings_Account : public Account{
    private :
       int term_month;
       int savings_id;
       std::map<int,double> Deposit_Term_AND_Interest_Rate;
       std::string maturity_option;
    public :

};

class CheckingAccount: public Account{
    private :
        double overdraft_limit;
        double daily_transfer_limit;
        double current_spent_today;

};

class Credit_Account : public Account {
    private : 
        double limit;
        double current_debt_balance;
        double overdue_interest;
    public :
};



#endif