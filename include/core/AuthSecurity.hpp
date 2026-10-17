#ifndef AUTH_SECURITY_HPP
#define AUTH_SECURITY_HPP

#include<string>
#include <sstream>
#include <iomanip>
#include <random>
#include"sha256.hpp"

class AuthSecurity {
    public:
        std::string generateSalt(size_t length = 16);
        std::string hashPin ( const std::string&PIN );
        std::string generateOTP(size_t length = 6);

};
std::string vectorToHexString(const std::vector<uint8_t>& bytes);

#endif