#ifndef BILL_DTOS_HPP
#define BILL_DTOS_HPP

#include <string>
#include <vector>
#include <Core ( Backend )/database/json.hpp>

namespace Banking {

// ============================================================================
// 1. DTO BẢN GHI HÓA ĐƠN (BILL ITEM DTO)
// ============================================================================
struct BillDTO {
    std::string bill_id;           // Mã hóa đơn (VD: "BILL_EVN_102026")
    std::string service_type;      // Loại dịch vụ: "ELECTRICITY", "WATER", "INTERNET", "TUITION"
    std::string provider_code;     // Mã nhà cung cấp (VD: "EVN_HN", "VIETTEL", "HAWACO")
    std::string provider_name;     // Tên nhà cung cấp (VD: "Điện lực Hà Nội")
    std::string customer_code;     // Mã khách hàng/Mã danh bộ tại nhà cung cấp (VD: "PA123456")
    std::string customer_name;     // Tên chủ hộ / Khách hàng ghi trên hóa đơn
    double amount = 0.0;           // Số tiền hóa đơn cần thanh toán
    std::string period;            // Kỳ hóa đơn (VD: "10/2026")
    std::string due_date;          // Hạn thanh toán (YYYY-MM-DD)
    std::string status = "UNPAID"; // Trạng thái: "UNPAID", "PAID"

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        BillDTO,
        bill_id,
        service_type,
        provider_code,
        provider_name,
        customer_code,
        customer_name,
        amount,
        period,
        due_date,
        status
    )
};

// ============================================================================
// 2. DTO TRA CỨU HÓA ĐƠN (QUERY BILL REQUEST & RESULT)
// ============================================================================

// Yêu cầu tra cứu hóa đơn từ UI
struct QueryBillRequest {
    std::string service_type;      // "ELECTRICITY", "WATER", "INTERNET",...
    std::string provider_code;     // Mã nhà cung cấp
    std::string customer_code;     // Mã khách hàng/Mã danh bộ nhập trên UI

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        QueryBillRequest,
        service_type,
        provider_code,
        customer_code
    )
};

// Kết quả tra cứu hóa đơn trả về UI
struct QueryBillResult {
    bool success = false;
    std::string message;
    bool has_unpaid_bill = false;  // Có hóa đơn nợ cước không?
    BillDTO bill;                  // Chi tiết hóa đơn nợ cước

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        QueryBillResult,
        success,
        message,
        has_unpaid_bill,
        bill
    )
};

// ============================================================================
// 3. DTO THỰC HIỆN THANH TOÁN HÓA ĐƠN (PAY BILL REQUEST & RESULT)
// ============================================================================

// Yêu cầu thanh toán gửi từ UI Form
struct PayBillRequest {
    std::string user_id;           // ID người dùng thực hiện
    std::string source_stk;        // Số tài khoản trích tiền
    std::string bill_id;           // Mã hóa đơn cần thanh toán
    std::string service_type;      // Loại dịch vụ
    std::string provider_code;     // Mã nhà cung cấp
    std::string customer_code;     // Mã khách hàng
    double amount = 0.0;           // Số tiền thanh toán
    std::string pin_input;         // Mã PIN 6 số xác thực

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        PayBillRequest,
        user_id,
        source_stk,
        bill_id,
        service_type,
        provider_code,
        customer_code,
        amount,
        pin_input
    )
};

// Biên lai kết quả thanh toán hóa đơn
struct PayBillResult {
    bool success = false;
    std::string message;
    std::string transaction_ref;    // Mã tham chiếu giao dịch (VD: "BILL20261016...")
    std::string bill_id;
    double amount = 0.0;
    double fee_applied = 0.0;       // Phí thanh toán (nếu có, mặc định 0đ)
    double source_new_balance = 0.0;// Số dư mới của tài khoản trích tiền
    std::string timestamp;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        PayBillResult,
        success,
        message,
        transaction_ref,
        bill_id,
        amount,
        fee_applied,
        source_new_balance,
        timestamp
    )
};

} // namespace Banking

#endif // BILL_DTOS_HPP