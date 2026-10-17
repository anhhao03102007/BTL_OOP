
#ifndef ACCOUNT_SERVICE_HPP
#define ACCOUNT_SERVICE_HPP

#include "dtos/AccountDTOs.hpp"
#include "model/Users/User.hpp"
#include "Repository/AccountRepository.hpp"
#include "Repository/UserRepository.hpp"
#include "dtos/TransactionDTOs.hpp"

namespace Banking {

class AccountService {
private:
    AccountRepository& m_accountRepo;
    UserRepository&    m_userRepo;

public:
    AccountService(AccountRepository& accountRepo, UserRepository& userRepo)
        : m_accountRepo(accountRepo), m_userRepo(userRepo) {}

    // =========================================================================
    // TÍNH NĂNG 1: XEM DANH SÁCH & SỐ DƯ TÀI KHOẢN (RBAC ENFORCEMENT)
    // =========================================================================
    AccountListResult getAccounts(const std::string& operatorUserId, const std::string& targetUserId) {
        AccountListResult result;

        // Nạp thông tin người thực hiện thao tác
        auto operatorUserOpt = m_userRepo.findUserById(operatorUserId);
        if (!operatorUserOpt.has_value()) {
            result.success = false;
            result.message = "Người dùng thực hiện không tồn tại!";
            return result;
        }

        const auto& operatorUser = operatorUserOpt.value();

        // Kiểm tra quyền: Customer CỦA CHÍNH MÌNH hoặc GDV / Admin
        if (operatorUser->getRole() == "CUSTOMER" && operatorUserId != targetUserId) {
            result.success = false;
            result.message = "Lỗi phân quyền: Khách hàng chỉ được phép xem tài khoản chính chủ!";
            return result;
        }

        // Truy vấn dữ liệu từ Repository
        std::vector<AccountDTO> dtos = m_accountRepo.findAccountsByUserId(targetUserId);
        result.success = true;
        result.message = "Truy vấn thành công!";
        result.total_count = dtos.size();
        result.accounts = dtos;

        return result;
    }

    // =========================================================================
    // TÍNH NĂNG 2: TRA CỨU LỊCH SỬ GIAO DỊCH / IN SAO KÊ
    // =========================================================================
    TransactionHistoryResult getTransactionHistory(const std::string& operatorUserId, 
                                                   const std::string& stk, 
                                                   int limit = 50, 
                                                   int offset = 0) {
        TransactionHistoryResult result;
        result.stk = stk;

        // 1. Xác thực người thao tác
        auto operatorUserOpt = m_userRepo.findUserById(operatorUserId);
        if (!operatorUserOpt.has_value()) {
            result.success = false;
            result.message = "Người dùng thao tác không tồn tại trên hệ thống!";
            return result;
        }

        const auto& operatorUser = operatorUserOpt.value();
        std::string role = operatorUser->getRole();

        // 2. Kiểm tra RBAC
        if (role == "CUSTOMER") {
            bool isOwner = m_accountRepo.isAccountOwnedByUser(stk, operatorUserId);
            if (!isOwner) {
                result.success = false;
                result.message = "Lỗi phân quyền: Khách hàng chỉ được phép xem sao kê tài khoản chính chủ!";
                return result;
            }
        }

        // 3. Lấy dữ liệu và đóng gói vào Struct
        result.transactions = m_accountRepo.getStatementByAccount(stk, limit, offset);
        result.total_records = result.transactions.size();
        result.success = true;
        result.message = "Lấy lịch sử giao dịch thành công!";

        return result; // Trả về Struct
    }
};

    // =========================================================================
    // TÍNH NĂNG 3: MỞ TÀI KHOẢN THANH TOÁN MỚI (eKYC / GDV MỞ TẠI QUẦY)
    // =========================================================================
    TransactionResult openNewAccount(const std::string& operatorUserId, const std::string& targetUserId, const std::string& accountType) {
        TransactionResult result;

        auto operatorUser = m_userRepo.findUserById(operatorUserId);
        if (!operatorUser) {
            result.success = false;
            result.message = "Không xác thực được người dùng!";
            return result;
        }

        std::string newStk = "10" + std::to_string(time(nullptr)); // Tự động sinh STK mới

        // Trường hợp 1: KH tự mở qua eKYC
        if ((*operatorUser)->getRole() == "CUSTOMER") {
            if (operatorUserId != targetUserId) {
                result.success = false;
                result.message = "Khách hàng chỉ được phép mở tài khoản eKYC cho chính mình!";
                return result;
            }
        } 
        // Trường hợp 2: GDV mở tại quầy cho Khách hàng
        else if ((*operatorUser)->getRole() == "TELLER") {
            // GDV có thể mở cho bất kỳ targetUserId nào sau khi tiếp nhận hồ sơ
        }

        bool created = m_accountRepo.createNewAccount(newStk, targetUserId, accountType, 0.0, "ACTIVE");
        if (!created) {
            result.success = false;
            result.message = "Lỗi CSDL khi tạo tài khoản mới!";
            return result;
        }

        result.success = true;
        result.message = "Mở tài khoản thanh toán mới thành công!";
        result.transaction_ref = newStk;
        return result;
    }

    // =========================================================================
    // TÍNH NĂNG 4: KHÓA / MỞ KHÓA TÀI KHOẢN KHẨN CẤP
    // =========================================================================
    TransactionResult toggleAccountLock(const std::string& operatorUserId, const std::string& stk, bool lockRequest, const std::string& pinOrVerifyCode) {
        TransactionResult result;

        auto operatorUser = m_userRepo.findUserById(operatorUserId);
        if (!operatorUser) {
            result.success = false;
            result.message = "Người dùng không tồn tại!";
            return result;
        }

        std::string role = (*operatorUser)->getRole();

        // 1. Khách hàng tự khóa khẩn cấp tài khoản chính chủ
        if (role == "CUSTOMER") {
            if (!lockRequest) {
                result.success = false;
                result.message = "Khách hàng chỉ được tự KHÓA khẩn cấp. Vui lòng ra quầy GDV để MỞ KHÓA!";
                return result;
            }

            if (!(*operatorUser)->verifyPin(pinOrVerifyCode)) {
                result.success = false;
                result.message = "Mã PIN xác thực khóa khẩn cấp không chính xác!";
                return result;
            }
        } 
        // 2. Giao dịch viên / Admin mở khóa khẩn cấp (sau khi xác minh CMND/CCCD)
        else if (role == "TELLER" || role == "ADMIN") {
            // GDV xác minh thông tin giấy tờ thành công mới tiến hành đổi status
        }

        std::string newStatus = lockRequest ? "LOCKED" : "ACTIVE";
        bool updated = m_accountRepo.updateAccountStatus(stk, newStatus);

        if (!updated) {
            result.success = false;
            result.message = "Lỗi CSDL khi cập nhật trạng thái tài khoản!";
            return result;
        }

        result.success = true;
        result.message = lockRequest ? "Đã khóa tài khoản thành công!" : "Mở khóa tài khoản thành công!";
        return result;
    }
};

} // namespace Banking

#endif // ACCOUNT_SERVICE_HPP