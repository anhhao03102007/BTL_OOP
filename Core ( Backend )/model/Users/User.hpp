#ifndef USER_HPP
#define USER_HPP

#include<string>

class User {
    private :
        std::string user_id;
        std::string user_name;
        std::string password_hash;
        std::string fullname;
        std::string email;
        std::string id_card_number;
        std::string phone_number;
        std::string role;
        std::string status;
    public :
       virtual void input();
};

class Customer : public User {
    private : 
};


class Teller : public User {

};

class Admin : public User {

};


#endif