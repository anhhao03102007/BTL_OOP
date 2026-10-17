#ifndef TURSO_DB_HPP
#define TURSO_DB_HPP

#include <string>
#include "json.hpp" 

using json = nlohmann::json;

class TursoDB {
private:
    std::string url;
    std::string token;

    // Callback hứng dữ liệu phản hồi từ HTTP
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    // Gửi HTTP POST request tới Turso API
    std::string sendHttpRequest(const std::string& json_payload);

public:
    // Constructor Khởi tạo
    TursoDB(const std::string& db_url, const std::string& auth_token);

    // Phương thức thực thi câu lệnh SQL (Execute Query)
    json execute(const std::string& sql_query);
};

#endif // TURSO_DB_HPP