#ifndef USER_HPP
#define USER_HPP

#include<string>

class UserModel {
    protected:
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
       std::string getUserID() { return user_id;}
       std::string getUserName() { return user_name;}

    
       
};

class Customer : public UserModel {
    

};


class Teller : public UserModel {
    

};

class Admin : public UserModel {

};


#endif