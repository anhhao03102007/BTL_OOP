#ifndef USER_HPP
#define USER_HPP

#include<string>

class UserModel {
    protected:
        std::string m_userId;          // Mã định danh duy nhất (VD: "USR1001")
        std::string m_fullName;        // Họ và tên
        std::string m_cccdNumber;      // Số CCCD / CMND (12 chữ số)
        std::string m_phoneNumber;     // Số điện thoại
        std::string m_email;           // Địa chỉ Email
        std::string m_passwordHash;    // Chuỗi Hash SHA-256 của Mật khẩu
        std::string m_pinHash;         // Chuỗi Hash SHA-256 của Mã PIN (6 số)
        std::string m_role;            // Vai trò hệ thống: "CUSTOMER", "TELLER", "ADMIN"
        std::string m_status;          // Trạng thái tài khoản: "ACTIVE", "BLOCKED"

    public :
      UserModel() = default;

    // Constructor phục vụ Hydrate dữ liệu từ CSDL (Repository)
    UserModel(std::string userId, std::string fullName, std::string cccd,
              std::string phone, std::string email, std::string passHash,
              std::string pinHash, std::string role, std::string status)
        : m_userId(std::move(userId)), m_fullName(std::move(fullName)),
          m_cccdNumber(std::move(cccd)), m_phoneNumber(std::move(phone)),
          m_email(std::move(email)), m_passwordHash(std::move(passHash)),
          m_pinHash(std::move(pinHash)), m_role(std::move(role)),
          m_status(std::move(status)) {}

        // =========================================================================
        // DOMAIN INVARIANTS & BUSINESS LOGIC (Thực thi quy tắc nghiệp vụ trên RAM)
        // =========================================================================
       bool verifyPassword(const std::string& rawPassword) const;
       bool changePassword(const std::string& oldRawPass, const std::string& newRawPass);
       bool changePin(const std::string& oldRawPin, const std::string& newRawPin);
       void blockUser() { m_status = "BLOCKED"; }
       void activateUser() { m_status = "ACTIVE"; }

       // =========================================================================
    // READ-ONLY GETTERS & SETTERS (Tính Đóng Gói / Encapsulation)
    // =========================================================================
    [[nodiscard]] const std::string& getUserId() const { return m_userId; }
    [[nodiscard]] const std::string& getFullName() const { return m_fullName; }
    [[nodiscard]] const std::string& getCccdNumber() const { return m_cccdNumber; }
    [[nodiscard]] const std::string& getPhoneNumber() const { return m_phoneNumber; }
    [[nodiscard]] const std::string& getEmail() const { return m_email; }
    [[nodiscard]] const std::string& getRole() const { return m_role; }
    [[nodiscard]] const std::string& getStatus() const { return m_status; }

    // Dành riêng cho Mapper/Repository lưu vào CSDL
    [[nodiscard]] const std::string& getPasswordHash() const { return m_passwordHash; }
    [[nodiscard]] const std::string& getPinHash() const { return m_pinHash; }

    void updateProfile(std::string fullName, std::string phone, std::string email) {
        m_fullName = std::move(fullName);
        m_phoneNumber = std::move(phone);
        m_email = std::move(email);
    }
};

class Customer : public UserModel {
    private :
        
         

};


class Teller : public UserModel {
    private:

};

class Admin : public UserModel {
    private:
};


#endif