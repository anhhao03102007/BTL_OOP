#include "core/TursoDB.hpp"

#include <QByteArray>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

#include <iostream>
#include <utility>

namespace Banking {

TursoDB::TursoDB(std::string dbUrl, std::string authToken)
    : m_dbUrl(std::move(dbUrl)), m_authToken(std::move(authToken)) {}

json TursoDB::execute(const std::string& sql, const json& args) {
    return executeBatch({{sql, args}});
}

json TursoDB::executeBatch(const std::vector<std::pair<std::string, json>>& statements) {
    json responseJson = json::object();
    json requests = json::array();

    // foreign_keys is scoped to each HTTP stream, so enable it for every pipeline.
    requests.push_back({{"type", "execute"}, {"stmt", {{"sql", "PRAGMA foreign_keys = ON;"}}}});

    if (statements.size() > 1) {
        requests.push_back({{"type", "execute"}, {"stmt", {{"sql", "BEGIN TRANSACTION;"}}}});
    }

    for (const auto& [sql, args] : statements) {
        json stmtObj = {{"sql", sql}};
        if (!args.empty()) {
            stmtObj["args"] = args;
        }
        requests.push_back({{"type", "execute"}, {"stmt", stmtObj}});
    }

    if (statements.size() > 1) {
        requests.push_back({{"type", "execute"}, {"stmt", {{"sql", "COMMIT;"}}}});
    }
    requests.push_back({{"type", "close"}});

    const json body = {{"requests", requests}};
    const QByteArray payload = QByteArray::fromStdString(body.dump());
    QString endpoint = QString::fromStdString(m_dbUrl).trimmed();
    if (endpoint.startsWith(QStringLiteral("libsql://"), Qt::CaseInsensitive)) {
        endpoint.replace(0, 9, QStringLiteral("https://"));
    }
    while (endpoint.endsWith(QLatin1Char('/'))) {
        endpoint.chop(1);
    }
    if (!endpoint.endsWith(QStringLiteral("/v2/pipeline"), Qt::CaseInsensitive)) {
        endpoint += QStringLiteral("/v2/pipeline");
    }
    const QUrl url(endpoint);

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QByteArray("Bearer ") + QByteArray::fromStdString(m_authToken));

    QNetworkReply* reply = manager.post(request, payload);
    QEventLoop eventLoop;
    QTimer timeout;
    bool timedOut = false;
    timeout.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &eventLoop, [&]() {
        timedOut = true;
        reply->abort();
    });
    if (!reply->isFinished()) {
        timeout.start(10000);
        eventLoop.exec();
        timeout.stop();
    }

    const int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QNetworkReply::NetworkError networkError = reply->error();
    const QString errorMessage = reply->errorString();
    const QByteArray responseBody = reply->readAll();
    delete reply;

    if (networkError == QNetworkReply::NoError && httpCode == 200) {
        try {
            responseJson = json::parse(responseBody.toStdString());
            if (!responseJson.contains("results") || !responseJson["results"].is_array() ||
                responseJson["results"].empty() || !responseJson["results"][0].is_object() ||
                responseJson["results"][0].value("type", "") != "ok") {
                std::cerr << "[TursoDB] Could not enable foreign key enforcement\n";
                return json::object();
            }
            // Hide the setup result so repository result indices remain unchanged.
            responseJson["results"].erase(responseJson["results"].begin());
        } catch (const std::exception& e) {
            std::cerr << "[TursoDB Parse Error] " << e.what() << "\n";
        }
    } else {
        std::cerr << "[TursoDB HTTP Error] Code: " << httpCode
                  << " | " << (timedOut ? "Request timed out" : errorMessage.toStdString()) << "\n";
    }

    return responseJson;
}

} // namespace Banking