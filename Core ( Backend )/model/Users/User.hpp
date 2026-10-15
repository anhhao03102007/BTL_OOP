#ifndef USER_HPP
#define USER_HPP

#include<string>

class UserModel {
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
       std::string getUserID();
};

class Customer : public UserModel {
    private : 

};


class Teller : public UserModel {
    private:

};

class Admin : public UserModel {
    private :

};


#endif