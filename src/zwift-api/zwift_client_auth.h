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
// Temporary instrumentation for Dirk's Zwift authentication case. Do not merge into master.
#define QZ_ZWIFT_AUTH_DIAGNOSTICS 1
#if QZ_ZWIFT_AUTH_DIAGNOSTICS
#include <QCryptographicHash>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSslError>
#include <QStringList>
#endif

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

signals:
    void tokenReceived(bool success, const QString& message);

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
#if QZ_ZWIFT_AUTH_DIAGNOSTICS
            logAuthRequest(reply, "refresh_token", QByteArray());
#endif
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
            const QByteArray wireBody = postData.toString(QUrl::FullyEncoded).toUtf8();
            QNetworkReply* reply = networkManager.post(request, wireBody);
#if QZ_ZWIFT_AUTH_DIAGNOSTICS
            logAuthRequest(reply, "password", wireBody);
#endif
            connect(reply, &QNetworkReply::finished, [=]() {
                handleTokenResponse(reply);
            });
        }
    }

private slots:
    void handleTokenResponse(QNetworkReply* reply) {
        // Consume the body once: even HTTP 400 responses often include useful JSON
        // such as invalid_grant or a more specific server-side error code.
        const QByteArray responseData = reply->readAll();
#if QZ_ZWIFT_AUTH_DIAGNOSTICS
        logAuthResponse(reply, responseData);
#endif
        if (reply->error() == QNetworkReply::NoError) {
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
            emit tokenReceived(true, QObject::tr("Zwift Login OK!"));
        } else {
#if QZ_ZWIFT_AUTH_DIAGNOSTICS
            qDebug() << "Error fetching token:" << safeAuthDiagnostic(reply->errorString());
#else
            qDebug() << "Error fetching token: " << reply->errorString();
#endif
            emit tokenReceived(false, QObject::tr("Zwift Auth Failed!"));
        }

        reply->deleteLater();
    }

#if QZ_ZWIFT_AUTH_DIAGNOSTICS
    static QByteArray formField(const QByteArray &body, const QByteArray &key) {
        for (const QByteArray &part : body.split('&')) {
            if (part.startsWith(key + '=')) return part.mid(key.size() + 1);
        }
        return QByteArray();
    }

    // Intentionally NEVER log an actual username, password, bearer/refresh/id
    // token, Authorization header, Set-Cookie header, or full refresh-token URL.
    QString safeAuthDiagnostic(QString value) const {
        if (!username.isEmpty()) value.replace(username, "[username]", Qt::CaseInsensitive);
        if (!password.isEmpty()) value.replace(password, "[password]");
        value.replace(QRegularExpression(QStringLiteral("(?i)\\bBearer\\s+[^\\s,;]+")), QStringLiteral("Bearer [redacted]"));
        value.replace(QRegularExpression(QStringLiteral("(?i)(access_token|refresh_token|id_token|password|client_secret)\\s*[:=]\\s*[^\\s,;]+")), QStringLiteral("[redacted-secret]"));
        value.replace(QRegularExpression(QStringLiteral("https?://[^\\s\\\"'<>]+")), QStringLiteral("[url-redacted]"));
        return value.left(350);
    }

    static QString shortTokenFingerprint(const QString &token) {
        if (token.isEmpty()) return QStringLiteral("absent");
        // Correlate multiple issued tokens without exposing any usable token.
        return QString::fromLatin1(QCryptographicHash::hash(token.toUtf8(), QCryptographicHash::Sha256)
                                       .toHex().left(12));
    }

    void logAuthRequest(QNetworkReply *reply, const char *grant, const QByteArray &wireBody) {
        reply->setProperty("_qz_auth_start_ms", QDateTime::currentMSecsSinceEpoch());
        reply->setProperty("_qz_auth_grant", QString::fromLatin1(grant));
        const QByteArray encodedUser = formField(wireBody, "username");
        const QByteArray encodedPass = formField(wireBody, "password");
        const bool passwordGrant = !wireBody.isEmpty();
        const QUrl urlWithoutQuery = reply->url().adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment);
        qDebug() << "[DIRK-ZWIFT-AUTH] START"
                 << "grant=" << grant
                 << "method=" << (passwordGrant ? "POST" : "GET")
                 << "endpoint=" << urlWithoutQuery.toString()
                 << "usernameLength=" << username.size()
                 << "usernameLeadingTrailingWhitespace=" << (username != username.trimmed())
                 << "usernameNonAscii=" << (username.toUtf8().size() != username.size())
                 << "usernameAtCount=" << username.count(QLatin1Char('@'))
                 << "passwordLength=" << password.size()
                 << "passwordNonAscii=" << (password.toUtf8().size() != password.size())
                 << "passwordHasWhitespace=" << password.contains(QRegularExpression(QStringLiteral("\\s")))
                 << "passwordHasFormSpecials=" << password.contains(QRegularExpression(QStringLiteral("[&=+%?]")))
                 << "wireBodyBytes=" << wireBody.size()
                 << "wireUsernameBytes=" << encodedUser.size()
                 << "wirePasswordBytes=" << encodedPass.size()
                 << "wireUsernameDecodesOnceCorrectly=" << (!passwordGrant || QUrl::fromPercentEncoding(encodedUser) == username)
                 << "wirePasswordDecodesOnceCorrectly=" << (!passwordGrant || QUrl::fromPercentEncoding(encodedPass) == password)
                 << "wireUsernameContainsDoubleEncodedAt=" << encodedUser.contains("%2540")
                 << "refreshTokenPresent=" << !refresh_token.isEmpty()
                 << "refreshTokenLength=" << refresh_token.size()
                 << "refreshTokenFingerprint=" << shortTokenFingerprint(refresh_token)
                 << "accessTokenPresent=" << !access_token.isEmpty()
                 << "accessTokenExpiryMs=" << access_token_expiration;
        connect(reply, &QNetworkReply::sslErrors, this, [this](const QList<QSslError> &errors) {
            for (const QSslError &error : errors)
                qDebug() << "[DIRK-ZWIFT-AUTH] TLS ERROR" << safeAuthDiagnostic(error.errorString());
        });
    }

    void logAuthResponse(QNetworkReply *reply, const QByteArray &body) const {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const qint64 started = reply->property("_qz_auth_start_ms").toLongLong();
        const QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "[DIRK-ZWIFT-AUTH] FINISH"
                 << "grant=" << reply->property("_qz_auth_grant").toString()
                 << "httpStatus=" << httpStatus
                 << "httpReason=" << safeAuthDiagnostic(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString())
                 << "networkErrorCode=" << int(reply->error())
                 << "networkErrorText=" << safeAuthDiagnostic(reply->errorString())
                 << "elapsedMs=" << (started > 0 ? QDateTime::currentMSecsSinceEpoch() - started : -1)
                 << "bodyBytes=" << body.size()
                 << "responseContentType=" << QString::fromLatin1(reply->rawHeader("Content-Type")).left(100)
                 << "requestId=" << QString::fromLatin1(reply->rawHeader("X-Request-Id")).left(100)
                 << "correlationId=" << QString::fromLatin1(reply->rawHeader("X-Correlation-Id")).left(100)
                 << "cfRay=" << QString::fromLatin1(reply->rawHeader("CF-Ray")).left(100)
                 << "retryAfter=" << QString::fromLatin1(reply->rawHeader("Retry-After")).left(50)
                 << "redirectPathOnly=" << redirect.adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment).toString()
                 << "tlsCipher=" << reply->sslConfiguration().sessionCipher().name()
                 << "tlsProtocol=" << int(reply->sslConfiguration().sessionProtocol());

        QJsonParseError jsonError;
        const QJsonDocument json = QJsonDocument::fromJson(body, &jsonError);
        if (json.isObject()) {
            const QJsonObject object = json.object();
            for (const QString &field : {QStringLiteral("error"), QStringLiteral("error_description"),
                                         QStringLiteral("message"), QStringLiteral("status"), QStringLiteral("code")}) {
                if (object.contains(field))
                    qDebug() << "[DIRK-ZWIFT-AUTH] SERVER FIELD" << field
                             << safeAuthDiagnostic(object.value(field).toVariant().toString());
            }
            const QString access = object.value(QStringLiteral("access_token")).toString();
            const QString refresh = object.value(QStringLiteral("refresh_token")).toString();
            const QString id = object.value(QStringLiteral("id_token")).toString();
            qDebug() << "[DIRK-ZWIFT-AUTH] JSON TOKEN METADATA"
                     << "accessPresent=" << !access.isEmpty()
                     << "accessLength=" << access.size()
                     << "accessFingerprint=" << shortTokenFingerprint(access)
                     << "refreshPresent=" << !refresh.isEmpty()
                     << "refreshLength=" << refresh.size()
                     << "refreshFingerprint=" << shortTokenFingerprint(refresh)
                     << "idPresent=" << !id.isEmpty()
                     << "idLength=" << id.size()
                     << "expiresInSeconds=" << object.value(QStringLiteral("expires_in")).toInt(-1)
                     << "refreshExpiresInSeconds=" << object.value(QStringLiteral("refresh_expires_in")).toInt(-1)
                     << "tokenType=" << safeAuthDiagnostic(object.value(QStringLiteral("token_type")).toString())
                     << "serverErrorFieldPresent=" << object.contains(QStringLiteral("error"));
        } else {
            // Non-JSON response: do NOT dump HTML, gateway messages, or opaque blobs.
            qDebug() << "[DIRK-ZWIFT-AUTH] NON-JSON RESPONSE"
                     << "jsonParseError=" << jsonError.errorString()
                     << "bodySha256Prefix=" << QCryptographicHash::hash(body, QCryptographicHash::Sha256).toHex().left(12);
        }
    }
#endif

private:
    QNetworkAccessManager networkManager;
};


#endif // ZWIFT_CLIENT_AUTH_H
