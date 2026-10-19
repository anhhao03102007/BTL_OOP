#include "core/TursoDB.hpp"
#include <iostream>

namespace Banking {

size_t TursoDB::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

TursoDB::TursoDB(std::string dbUrl, std::string authToken)
    : m_dbUrl(std::move(dbUrl)), m_authToken(std::move(authToken)) {}

json TursoDB::execute(const std::string& sql, const json& args) {
    return executeBatch({{sql, args}});
}

json TursoDB::executeBatch(const std::vector<std::pair<std::string, json>>& statements) {
    CURL* curl = curl_easy_init();
    json responseJson = json::object();

    if (!curl) {
        std::cerr << "[TursoDB Error] Khởi tạo Curl thất bại!\n";
        return responseJson;
    }

    json requests = json::array();

    // 1. Mở Transaction nếu có nhiều câu lệnh
    if (statements.size() > 1) {
        requests.push_back({{"type", "execute"}, {"stmt", {{"sql", "BEGIN TRANSACTION;"}}}});
    }

    // 2. Đưa các câu lệnh tham số hóa vào Pipeline API
    for (const auto& [sql, args] : statements) {
        json stmtObj = {{"sql", sql}};
        if (!args.empty()) {
            stmtObj["args"] = args;
        }
        requests.push_back({{"type", "execute"}, {"stmt", stmtObj}});
    }

    // 3. Đóng Transaction
    if (statements.size() > 1) {
        requests.push_back({{"type", "execute"}, {"stmt", {{"sql", "COMMIT;"}}}});
    }

    requests.push_back({{"type", "close"}});

    json body = {{"requests", requests}};
    std::string jsonPayload = body.dump();
    std::string responseString;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer " + m_authToken;
    headers = curl_slist_append(headers, authHeader.c_str());

    std::string pipelineUrl = m_dbUrl + "/v2/pipeline";

    curl_easy_setopt(curl, CURLOPT_URL, pipelineUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    
    // Cấu hình Timeout an toàn
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L); // 5 giây kết nối
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);        // 10 giây phản hồi tổng

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    if (res == CURLE_OK && httpCode == 200) {
        try {
            responseJson = json::parse(responseString);
        } catch (const std::exception& e) {
            std::cerr << "[TursoDB Parse Error] " << e.what() << "\n";
        }
    } else {
        std::cerr << "[TursoDB HTTP Error] Code: " << httpCode << " | CurlMsg: " << curl_easy_strerror(res) << "\n";
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return responseJson;
}

} // namespace Banking