#ifndef REQUEST_H
#define REQUEST_H

#include "request.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>

class Request {
public:
    Request(const std::function<QString()>& get_access_token) : get_access_token(get_access_token) {}

    QByteArray getHeaders(const QString& accept_type) {
        QByteArray headers;
        headers.append("Accept: " + accept_type + "\r\n");
        headers.append("Authorization: Bearer " + get_access_token() + "\r\n");
        headers.append("User-Agent: Zwift/115 CFNetwork/758.0.2 Darwin/15.0.0\r\n");
        return headers;
    }

    QVariantMap json(const QString& url) {
        QNetworkAccessManager manager;
        QNetworkRequest request(QUrl(BASE_URL + url));
        request.setRawHeader("Content-Type", "application/json");
        request.setRawHeader("Authorization", ("Bearer " + get_access_token()).toUtf8());
        request.setRawHeader("User-Agent", "Zwift/115 CFNetwork/758.0.2 Darwin/15.0.0");

        QNetworkReply* reply = manager.get(request);
        if (!reply->waitForFinished()) {
            qWarning() << "Request failed:" << reply->errorString();
        }

        QByteArray data = reply->readAll();
        QVariantMap result = QtJson::Json::parse(data).toMap();

        delete reply;
        return result;
    }

    QByteArray protobuf(const QString& url) {
        QNetworkAccessManager manager;
        QNetworkRequest request(QUrl(BASE_URL + url));
        request.setRawHeader("Content-Type", "application/x-protobuf-lite");
        request.setRawHeader("Authorization", ("Bearer " + get_access_token()).toUtf8());
        request.setRawHeader("User-Agent", "Zwift/115 CFNetwork/758.0.2 Darwin/15.0.0");

        QNetworkReply* reply = manager.get(request);
        if (!reply->waitForFinished()) {
            qWarning() << "Request failed:" << reply->errorString();
        }

        QByteArray data = reply->readAll();

        delete reply;
        return data;
    }

private:
    const QString BASE_URL = "https://us-or-rly101.zwift.com";
    std::function<QString()> get_access_token;
};


#endif // REQUEST_H
