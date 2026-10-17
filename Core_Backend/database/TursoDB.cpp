#include <iostream>
#include "TursoDB.hpp"
#include <curl/curl.h>

// Constructor
TursoDB::TursoDB(const std::string& db_url, const std::string& auth_token) {
    this->url = db_url;
    // Tự động đổi libsql:// thành https:// nếu có
    if (this->url.find("libsql://") == 0) {
        this->url.replace(0, 9, "https://");
    }
    this->token = auth_token;
}

// Callback xử lý dữ liệu nhận về từ libcurl
size_t TursoDB::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Hàm gửi request POST
std::string TursoDB::sendHttpRequest(const std::string& json_payload) {
    CURL* curl = curl_easy_init();
    std::string response_data;

    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        std::string auth_header = "Authorization: Bearer " + this->token;
        headers = curl_slist_append(headers, auth_header.c_str());

        std::string endpoint = this->url + "/v2/pipeline";

        curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, TursoDB::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Loi CURL: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return response_data;
}

// Thực thi câu lệnh SQL
json TursoDB::execute(const std::string& sql_query) {
    json payload = {
        {"requests", json::array({
            {
                {"type", "execute"},
                {"stmt", {{"sql", sql_query}}}
            },
            {
                {"type", "close"}
            }
        })}
    };

    std::string raw_response = sendHttpRequest(payload.dump());
    
    try {
        return json::parse(raw_response);
    } catch (const json::parse_error& e) {
        std::cerr << "Loi Parse JSON Response: " << e.what() << std::endl;
        return json();
    }
}