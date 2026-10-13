#ifndef SAVINGS_BOOKS_HPP
#define SAVINGS_BOOKS_HPP

#include "../Account/Account.hpp"
#include "../../database/TursoDB.hpp" 

class Savings  {
    private:
        std::string stk_lien_ket; 
        double so_tien_gui;
        double lai_suat_nam;
        int ky_han_thang;
        std::string ngay_dao_han;
        std::string trang_thai;
    public:
    Savings(std::string stk_source, double amount, double rate, int months)
 : 
          stk_lien_ket(std::move(stk_source)), 
          so_tien_gui(amount), 
          lai_suat_nam(rate), 
          ky_han_thang(months),
          trang_thai("ACTIVE") {}
};
#endif