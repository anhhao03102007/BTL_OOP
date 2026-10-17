#ifndef LOAN_DTOS_HPP
#define LOAN_DTOS_HPP

#include <string>
#include <vector>
#include <Core ( Backend )/database/json.hpp>

namespace Banking {

// ============================================================================
// 1. DTO KỲ TRẢ NỢ (Sử dụng cho Bảng Lịch Trả Nợ / Installment Schedule)
// ============================================================================
struct LoanScheduleItem {
    int period_number = 0;              // Kỳ thứ (1, 2, 3...)
    std::string due_date;               // Ngày đến hạn (YYYY-MM-DD)
    double principal_due = 0.0;         // Tiền gốc phải trả kỳ này
    double interest_due = 0.0;          // Tiền lãi phải trả kỳ này
    double total_due = 0.0;             // Tổng tiền phải trả kỳ này (Gốc + Lãi)
    double remaining_principal = 0.0;   // Dư nợ gốc còn lại sau kỳ này
    std::string status = "UNPAID";      // Trạng thái: "UNPAID", "PAID", "OVERDUE"

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        LoanScheduleItem, 
        period_number, 
        due_date, 
        principal_due, 
        interest_due, 
        total_due, 
        remaining_principal, 
        status
    )
};

// ============================================================================
// 2. DTO ĐĂNG KÝ KHOẢN VAY MỚI (Request từ Frontend JS)
// ============================================================================
struct CreateLoanRequest {
    std::string customer_id;           // Mã khách hàng
    std::string account_id;            // Mã tài khoản thanh toán nhận giải ngân
    std::string loan_type;             // Loại vay: "CONSUMER", "MORTGAGE", "BUSINESS"
    double principal_amount = 0.0;     // Số tiền đề nghị vay
    int term_months = 12;              // Kỳ hạn vay (tháng)
    double annual_interest_rate = 0.0; // Lãi suất %/năm
    std::string interest_type;         // Phương thức tính lãi: "DECLINING_BALANCE", "FLAT"
    std::string purpose;               // Mục đích vay
    std::string pin_input;             // Mã PIN xác nhận

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        CreateLoanRequest, 
        customer_id, 
        account_id, 
        loan_type, 
        principal_amount, 
        term_months, 
        annual_interest_rate, 
        interest_type, 
        purpose, 
        pin_input
    )
};

// ============================================================================
// 3. DTO CHI TIẾT HỢP ĐỒNG KHOẢN VAY (Response / Database Entity Model)
// ============================================================================
struct LoanDetailDTO {
    std::string loan_id;               // Mã hợp đồng vay (VD: "LN20261014001")
    std::string customer_id;           // Mã khách hàng sở hữu
    std::string account_id;            // Tài khoản liên kết
    std::string loan_type;             // Loại khoản vay
    double original_principal = 0.0;   // Số tiền vay ban đầu
    double remaining_principal = 0.0;  // Dư nợ gốc còn lại
    int term_months = 0;               // Tổng kỳ hạn (tháng)
    int remaining_months = 0;          // Số tháng còn lại
    double annual_interest_rate = 0.0; // Lãi suất năm
    std::string interest_type;         // Phương thức tính lãi
    std::string status;                // Trạng thái: "PENDING", "ACTIVE", "COMPLETED", "REJECTED", "OVERDUE"
    std::string created_at;            // Ngày khởi tạo
    std::string next_payment_date;     // Ngày trả nợ tiếp theo
    double next_payment_amount = 0.0;  // Số tiền cần trả kỳ tiếp theo

    // Danh sách lịch trả nợ kỳ chi tiết
    std::vector<LoanScheduleItem> schedule;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        LoanDetailDTO, 
        loan_id, 
        customer_id, 
        account_id, 
        loan_type, 
        original_principal, 
        remaining_principal, 
        term_months, 
        remaining_months, 
        annual_interest_rate, 
        interest_type, 
        status, 
        created_at, 
        next_payment_date, 
        next_payment_amount, 
        schedule
    )
};

// ============================================================================
// 4. DTO THANH TOÁN KHOẢN VAY / TẤT TOÁN KỲ (Request từ Frontend JS)
// ============================================================================
struct PayLoanRequest {
    std::string loan_id;               // Mã khoản vay cần thanh toán
    std::string source_account_id;     // Tài khoản nguồn trích tiền
    double amount = 0.0;               // Số tiền nộp (trả kỳ hoặc tất toán)
    bool is_full_settlement = false;   // true nếu là tất toán toàn bộ khoản vay
    std::string pin_input;             // Mã PIN giao dịch

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        PayLoanRequest, 
        loan_id, 
        source_account_id, 
        amount, 
        is_full_settlement, 
        pin_input
    )
};

// ============================================================================
// 5. DTO KẾT QUẢ THỰC THI GIAO DỊCH KHOẢN VAY (Response về JS)
// ============================================================================
struct LoanOperationResult {
    bool success = false;              // Trạng thái thành công / thất bại
    std::string message;               // Thông báo cho người dùng
    std::string transaction_ref;       // Mã tham chiếu giao dịch
    std::string loan_id;               // Mã hợp đồng vay liên quan
    double paid_principal = 0.0;       // Tiền gốc đã thanh toán
    double paid_interest = 0.0;        // Tiền lãi đã thanh toán
    double remaining_loan_balance = 0.0;// Dư nợ còn lại sau giao dịch
    double source_account_balance = 0.0;// Số dư tài khoản sau khi trích tiền

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        LoanOperationResult, 
        success, 
        message, 
        transaction_ref, 
        loan_id, 
        paid_principal, 
        paid_interest, 
        remaining_loan_balance, 
        source_account_balance
    )
};

// ============================================================================
// 6. DTO YÊU CẦU TRA CỨU DANH SÁCH KHOẢN VAY (Request từ Frontend JS)
// ============================================================================
struct GetLoansRequest {
    std::string customer_id;           // Mã khách hàng cần tra cứu
    std::string status_filter = "ALL"; // Lọc trạng thái: "ALL", "PENDING", "ACTIVE", "COMPLETED", "OVERDUE"

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        GetLoansRequest,
        customer_id,
        status_filter
    )
};

// ============================================================================
// 7. DTO KẾT QUẢ TRA CỨU DANH SÁCH KHOẢN VAY (Response về JS)
// ============================================================================
struct LoanListResult {
    bool success = false;               // Trạng thái truy vấn
    std::string message;                // Thông báo kết quả
    size_t total_count = 0;             // Tổng số khoản vay tìm thấy
    std::vector<LoanDetailDTO> loans;   // Danh sách chi tiết các khoản vay

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        LoanListResult,
        success,
        message,
        total_count,
        loans
    )
};
// ============================================================================
// DTO YÊU CẦU DUYỆT / TỪ CHỐI KHOẢN VAY (Admin/Teller Request)
// ============================================================================
struct ApproveLoanRequest {
    std::string admin_id;          // ID của Admin hoặc Teller thực hiện duyệt
    std::string loan_id;           // Mã khoản vay cần xử lý
    std::string action;            // "APPROVE" (Phê duyệt) hoặc "REJECT" (Từ chối)
    std::string note;              // Ghi chú lý do duyệt/từ chối
    std::string admin_pin;         // Mã PIN/Xác thực thao tác của Admin

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        ApproveLoanRequest,
        admin_id,
        loan_id,
        action,
        note,
        admin_pin
    )
};

// ============================================================================
// DTO KẾT QUẢ XỬ LÝ DUYỆT KHOẢN VAY (Response về Bridge/UI)
// ============================================================================
struct LoanApprovalResult {
    bool success = false;
    std::string message;
    std::string loan_id;
    std::string new_status;              // "ACTIVE", "REJECTED"
    double disbursed_amount = 0.0;       // Số tiền đã giải ngân (nếu APPROVE)
    double customer_new_balance = 0.0;   // Số dư mới của tài khoản KH sau giải ngân
    std::string timestamp;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        LoanApprovalResult,
        success,
        message,
        loan_id,
        new_status,
        disbursed_amount,
        customer_new_balance,
        timestamp
    )
};


} // namespace Banking

#endif // LOAN_DTOS_HPP