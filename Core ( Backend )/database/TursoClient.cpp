#include "TursoClient.hpp" 
#include <iostream>

// Hàm phụ trợ xử lý và giải phóng lỗi an toàn
void TursoClient::handleError(libsql_error_t* err, const std::string& message) {
    if (err) {
        std::cerr << "[Turso Error] " << message << ": " << libsql_error_message(err) << "\n";
        libsql_error_deinit(err);
    }
}

TursoClient::TursoClient(const std::string& dbUrl, const std::string& token) 
    : url(dbUrl), authToken(token), isConnected(false) {
    // Khởi tạo cấu hình toàn cục libSQL
    libsql_setup((libsql_config_t){0});
}

TursoClient::~TursoClient() {
    disconnect();
}

bool TursoClient::connect() {
    libsql_database_desc_t desc = {0};
    desc.url = url.c_str();
    desc.auth_token = authToken.c_str();

    // 1. Khởi tạo database
    db = libsql_database_init(desc);
    if (db.err) {
        handleError(db.err, "Lỗi khởi tạo database");
        return false;
    }

    // 2. Tạo kết nối
    conn = libsql_database_connect(db);
    if (conn.err) {
        handleError(conn.err, "Lỗi kết nối database");
        libsql_database_deinit(db);
        return false;
    }

    isConnected = true;
    return true;
}

void TursoClient::disconnect() {
    if (isConnected) {
        libsql_connection_deinit(conn);
        libsql_database_deinit(db);
        isConnected = false;
    }
}

bool TursoClient::isConnecting() const {
    return isConnected;
}

// Phương thức truy vấn tổng quát trả về dữ liệu thô hoặc xử lý tùy biến
void TursoClient::executeQuery(const std::string& sql) {
    if (!isConnected) {
        std::cerr << "[Turso Error] Chưa kết nối đến database!\n";
        return;
    }

    // 1. Chuẩn bị câu lệnh (Prepare statement)
    libsql_statement_t stmt = libsql_connection_prepare(conn, sql.c_str());
    if (stmt.err) {
        handleError(stmt.err, "Lỗi chuẩn bị câu lệnh SQL");
        return;
    }

    // 2. Thực thi truy vấn lấy tập kết quả (Rows)
    libsql_rows_t rows = libsql_statement_query(stmt);
    if (rows.err) {
        handleError(rows.err, "Lỗi thực thi truy vấn");
        libsql_statement_deinit(stmt);
        return;
    }

    // 3. Duyệt qua từng dòng kết quả
    while (true) {
        libsql_row_t row = libsql_rows_next(rows);
        if (row.err) {
            handleError(row.err, "Lỗi đọc dòng dữ liệu");
            break;
        }

        if (libsql_row_empty(row)) {
            libsql_row_deinit(row);
            break;
        }

        int32_t columnCount = libsql_row_length(row);
        for (int32_t i = 0; i < columnCount; ++i) {
            libsql_result_value_t res_val = libsql_row_value(row, i);
            if (res_val.err) {
                handleError(res_val.err, "Lỗi lấy giá trị cột");
                continue;
            }
            if (res_val.ok.type == LIBSQL_TYPE_TEXT) {
                std::string textVal((const char*)res_val.ok.value.text.ptr, res_val.ok.value.text.len);
                std::cout << textVal << "\t";
            } 
            else if (res_val.ok.type == LIBSQL_TYPE_INTEGER) {
                std::cout << res_val.ok.value.integer << "\t";
            } 
            else if (res_val.ok.type == LIBSQL_TYPE_REAL) {
                std::cout << res_val.ok.value.real << "\t";
            }
            else {
                std::cout << "NULL\t";
            }
        }
        std::cout << "\n";

        libsql_row_deinit(row);
    }

    // 4. Dọn dẹp tài nguyên
    libsql_rows_deinit(rows);
    libsql_statement_deinit(stmt);
}