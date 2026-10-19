#ifndef TURSO_DB_HPP
#define TURSO_DB_HPP

#include <string>
#include <vector>
#include <utility>
#include <core/json.hpp>
#include <curl/curl.h>

namespace Banking {

using json = nlohmann::json;

class TursoDB {
private:
    std::string m_dbUrl;
    std::string m_authToken;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    TursoDB(std::string dbUrl, std::string authToken);

    // Thực thi 1 câu lệnh đơn có tham số (Parameterized Query)
    json execute(const std::string& sql, const json& args = json::array());

    // Thực thi danh sách câu lệnh trong 1 Transaction (Atomic Batch Execution)
    json executeBatch(const std::vector<std::pair<std::string, json>>& statements);
};

} // namespace Banking

#endif // TURSO_DB_HPP