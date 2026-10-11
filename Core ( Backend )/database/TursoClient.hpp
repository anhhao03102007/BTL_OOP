#ifndef TURSOCLIENT_HPP
#define TURSOCLIENT_HPP

#include <string>
#include "libsql.h"

class TursoClient {
private:
    std::string url ;
    std::string authToken;
    libsql_database_t db;
    libsql_connection_t conn;
    bool isConnected;

    void handleError(libsql_error_t* err, const std::string& message);


public:
    TursoClient(const std::string& dbUrl, const std::string& token);
    ~TursoClient();

    bool connect();
    void disconnect();
    bool isConnecting() const;
    
    // Hàm thực thi câu lệnh SQL tổng quát
    void executeQuery(const std::string& sql);
};

#endif