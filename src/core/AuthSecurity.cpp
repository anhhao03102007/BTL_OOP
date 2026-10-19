#include<iostream>
#include"AuthSecurity.hpp"
using namespace YukiWorkshop::Crypto;

std::string AuthSecurity::generateSalt(size_t length = 16) {
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

        std::string salt;
        salt.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            salt += charset[dist(gen)];
        }
        return salt;
    }
std::string vectorToHexString(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    for (uint8_t b : bytes) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return ss.str();
}
std::string AuthSecurity::hashPin(const std::string& PIN ){
    SHA256 s;
    s.update(PIN);
    std::string hash_str = vectorToHexString(s.finalize());
    return hash_str;
}
std::string AuthSecurity::generateOTP(size_t length = 6) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 9); 

        std::string otp;
        otp.reserve(length);

        for (size_t i = 0; i < length; ++i) {
            otp += std::to_string(dist(gen));
        }

        return otp;
    }

std::string AuthSecurity::generateToken(size_t length) {
        static constexpr char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::random_device rd;
        std::mt19937_64 generator(rd());
        std::uniform_int_distribution<size_t> distribution(0, sizeof(charset) - 2);

        std::string token;
        token.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            token += charset[distribution(generator)];
        }
        return token;
    }
std::string AuthSecurity::hashPasswordPBKDF2(const std::string& password, const std::string& salt){
    SHA256 s;
    s.update(password + salt); // Ví dụ minh họa băm kết hợp salt
    return vectorToHexString(s.finalize());
}
bool AuthSecurity::verifyPassword(const std::string& rawPassword, 
                                  const std::string& storedHash, 
                                  const std::string& salt) {
    if (rawPassword.empty() || storedHash.empty() || salt.empty()) {
        return false;
    }

    // Băm mật khẩu người dùng nhập vào với Salt lưu từ CSDL
    std::string computedHash = hashPasswordPBKDF2(rawPassword, salt);

    // So sánh chuỗi hash tính toán được với hash lưu trong CSDL
    return computedHash == storedHash;
}