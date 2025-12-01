#ifndef ZWIFT_CLIENT_AUTH_H
#define ZWIFT_CLIENT_AUTH_H

#include <QCoreApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

class AuthToken : public QObject {
    Q_OBJECT
public:
    QString username;
    QString password;
    QString access_token;
    qint64 expires_in;
    QString id_token;
    qint64 not_before_policy;
    QString refresh_token;
    qint64 refresh_expires_in;
    QString session_state;
    QString token_type;
    qint64 access_token_expiration;
    qint64 refresh_token_expiration;

    AuthToken(const QString& username, const QString& password, QObject* parent = nullptr)
        : QObject(parent), username(username), password(password) {}

signals:
    void tokenReceived(bool success, const QString& message);

    bool haveValidAccessToken() const {
        return !access_token.isEmpty() && QDateTime::currentMSecsSinceEpoch() < access_token_expiration;
    }

    bool haveValidRefreshToken() const {
        return !refresh_token.isEmpty() && QDateTime::currentMSecsSinceEpoch() < refresh_token_expiration;
    }

    Q_INVOKABLE QString getAccessToken() {
        if (haveValidAccessToken()) {
            return access_token;
        } else {
            updateTokenData();
            return access_token;
        }
    }

public slots:
    void updateTokenData() {
        if (haveValidRefreshToken()) {
            QUrl url("https://secure.zwift.com/auth/realms/zwift/tokens/access/codes");
            QUrlQuery query;
            query.addQueryItem("refresh_token", refresh_token);
            query.addQueryItem("grant_type", "refresh_token");
            query.addQueryItem("client_id", "Zwift_Mobile_Link");
            url.setQuery(query);

            QNetworkRequest request(url);
            QNetworkReply* reply = networkManager.get(request);
            connect(reply, &QNetworkReply::finished, [=]() {
                handleTokenResponse(reply);
            });
        } else {
            QUrl url("https://secure.zwift.com/auth/realms/zwift/tokens/access/codes");
            QUrlQuery postData;
            postData.addQueryItem("username", QUrl::toPercentEncoding(username));
            postData.addQueryItem("password", password);
            postData.addQueryItem("grant_type", "password");
            postData.addQueryItem("client_id", "Zwift_Mobile_Link");
            
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            QNetworkReply* reply = networkManager.post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
            connect(reply, &QNetworkReply::finished, [=]() {
                handleTokenResponse(reply);
            });
        }
    }

private slots:
    void handleTokenResponse(QNetworkReply* reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);
            QJsonObject tokenData = jsonDocument.object();

            QDateTime now = QDateTime::currentDateTime();
            access_token = tokenData["access_token"].toString();
            expires_in = tokenData["expires_in"].toInt();
            id_token = tokenData["id_token"].toString();
            not_before_policy = tokenData["not-before-policy"].toInt();
            refresh_token = tokenData["refresh_token"].toString();
            refresh_expires_in = tokenData["refresh_expires_in"].toInt();
            session_state = tokenData["session_state"].toString();
            token_type = tokenData["token_type"].toString();

            access_token_expiration = now.toMSecsSinceEpoch() + (expires_in - 5) * 1000;
            refresh_token_expiration = now.toMSecsSinceEpoch() + (refresh_expires_in - 5) * 1000;

            qDebug() << "Access Token received successfully";
            emit tokenReceived(true, "Zwift Login OK!");
        } else {
            qDebug() << "Error fetching token: " << reply->errorString();
            emit tokenReceived(false, "Zwift Auth Failed!");
        }

        reply->deleteLater();
    }

private:
    QNetworkAccessManager networkManager;
};


#endif // ZWIFT_CLIENT_AUTH_H
