#include<iostream>
#include"AuthSecurity.hpp"
using namespace YukiWorkshop::Crypto;

static std::string generateSalt(size_t length = 16) {
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
static std::string generateOTP(size_t length = 6) {
        // static giúp nguồn entropy phần cứng chỉ khởi tạo 1 lần
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 9); // Sinh từng chữ số từ 0 đến 9

        std::string otp;
        otp.reserve(length);

        for (size_t i = 0; i < length; ++i) {
            otp += std::to_string(dist(gen));
        }

        return otp;
    }