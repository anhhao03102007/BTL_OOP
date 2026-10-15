#ifndef TRANSACTION_REPOSITORY_HPP
#define TRANSACTION_REPOSITORY_HPP

#include "Transaction.hpp"
#include "database/TursoDB.hpp"
#include <iomanip>
#include <sstream>

class TransactionRepository {
private:
    TursoDB& db;

    // Helper 1: Chuyển double thành chuỗi số nguyên/thập phân chuẩn SQL (tránh lỗi 1e+06)
    std::string formatAmount(double amount) const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << amount;
        return ss.str();
    }

    // Helper 2: Làm sạch chuỗi chống gãy cú pháp SQL / SQL Injection khi ghép chuỗi
    std::string sanitize(std::string str) const {
        size_t pos = 0;
        while ((pos = str.find("'", pos)) != std::string::npos) {
            str.replace(pos, 1, "''");
            pos += 2;
        }
        return str;
    }

public:
    explicit TransactionRepository(TursoDB& database) : db(database) {}

    // 1. Truy vấn trạng thái tài khoản bằng cách ghép chuỗi SQL
    std::pair<double, std::string> getAccountState(const std::string& accountId) {
        std::string sql = "SELECT so_du, trang_thai FROM tai_khoan WHERE ma_tk = '" + sanitize(accountId) + "';";
        
        auto query_res = db.execute(sql);

        if (!query_res.contains("results") || query_res["results"].empty() || 
            !query_res["results"][0].contains("response")) {
            return {-1.0, "NOT_FOUND"};
        }

        auto& rows = query_res["results"][0]["response"]["result"]["rows"];
        if (rows.empty()) return {-1.0, "NOT_FOUND"};

        double balance = std::stod(rows[0][0]["value"].get<std::string>());
        std::string status = rows[0][1]["value"].get<std::string>();
        return {balance, status};
    }

    // 2. Thực thi cập nhật CSDL bằng ghép chuỗi SQL (ACID Transaction)
    bool persistTransaction(Transaction& txn, double totalDeduct) {
        try {
            db.execute("BEGIN TRANSACTION;");

            // Ghép chuỗi SQL trừ tiền tài khoản nguồn
            std::string deduct_sql = "UPDATE tai_khoan SET so_du = so_du - " + formatAmount(totalDeduct) + 
                                     " WHERE ma_tk = '" + sanitize(txn.getSenderAccountId()) + "';";
            db.execute(deduct_sql);

            // Nếu chuyển tiền nội bộ -> Ghép chuỗi SQL cộng tiền tài khoản đích
            if (txn.getTransactionType() == "INTERNAL_TRANSFER") {
                std::string add_sql = "UPDATE tai_khoan SET so_du = so_du + " + formatAmount(txn.getAmount()) + 
                                      " WHERE ma_tk = '" + sanitize(txn.getReceiverAccountId()) + "';";
                db.execute(add_sql);
            }

            // Ghép chuỗi SQL ghi Nhật ký Giao dịch (Audit Log)
            std::string log_sql = 
                "INSERT INTO lich_su_giao_dich (ma_gd_ref, stk_nguon, stk_dich, so_tien, loai_gd, noi_dung, trang_thai) "
                "VALUES ('" + sanitize(txn.getTransactionId()) + "', '" 
                            + sanitize(txn.getSenderAccountId()) + "', '" 
                            + sanitize(txn.getReceiverAccountId()) + "', " 
                            + formatAmount(txn.getAmount()) + ", '" 
                            + sanitize(txn.getTransactionType()) + "', '" 
                            + sanitize(txn.getDescription()) + "', 'SUCCESS');";
            
            db.execute(log_sql);

            db.execute("COMMIT;");
            return true;
        } catch (...) {
            db.execute("ROLLBACK;");
            return false;
        }
    }
};

#endif // TRANSACTION_REPOSITORY_HPP