#ifndef GARMINCONNECT_H
#define GARMINCONNECT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkCookie>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QDateTime>
#include <QEventLoop>
#include <QHttpMultiPart>

/**
 * @brief GarminConnect class handles authentication and file upload to Garmin Connect
 *
 * This class implements the Garmin Connect SSO authentication flow and provides
 * methods to upload FIT files to Garmin Connect. It is based on the Python garth
 * library (https://github.com/matin/garth) ported to C++ Qt.
 *
 * Authentication Flow:
 * 1. User provides email/password
 * 2. SSO login to get ticket
 * 3. Exchange ticket for OAuth1 token
 * 4. Exchange OAuth1 for OAuth2 token
 * 5. Use OAuth2 token for API requests
 *
 * The OAuth2 tokens are stored in QSettings and automatically refreshed when expired.
 */
class GarminConnect : public QObject
{
    Q_OBJECT

public:
    explicit GarminConnect(QObject *parent = nullptr);
    ~GarminConnect();

    /**
     * @brief Authenticate with Garmin Connect using email and password
     * @param email User's email
     * @param password User's password
     * @param mfaCode Optional MFA code if two-factor authentication is enabled
     * @return true if authentication successful, false otherwise
     */
    bool login(const QString &email, const QString &password, const QString &mfaCode = QString());

    /**
     * @brief Upload a FIT file to Garmin Connect
     * @param fitData Binary FIT file data
     * @param fileName Name of the file
     * @return true if upload successful, false otherwise
     */
    bool uploadActivity(const QByteArray &fitData, const QString &fileName);

    /**
     * @brief Check if user is currently authenticated
     * @return true if valid OAuth2 token exists, false otherwise
     */
    bool isAuthenticated() const;

    /**
     * @brief Logout and clear all stored tokens
     */
    void logout();

    /**
     * @brief Get the last error message
     * @return Error message string
     */
    QString lastError() const { return m_lastError; }

    /**
     * @brief Set the Garmin domain (default: garmin.com, china: garmin.cn)
     * @param domain Domain name
     */
    void setDomain(const QString &domain) { m_domain = domain; }

signals:
    /**
     * @brief Emitted when authentication is successful
     */
    void authenticated();

    /**
     * @brief Emitted when authentication fails
     * @param error Error message
     */
    void authenticationFailed(const QString &error);

    /**
     * @brief Emitted when upload is successful
     */
    void uploadSucceeded();

    /**
     * @brief Emitted when upload fails
     * @param error Error message
     */
    void uploadFailed(const QString &error);

    /**
     * @brief Emitted when MFA code is required
     * This signal indicates that the user needs to provide a 2FA code
     */
    void mfaRequired();

private:
    // Network
    QNetworkAccessManager *m_manager;
    QString m_lastError;
    QString m_domain;
    QString m_csrfToken;
    QList<QNetworkCookie> m_cookies;

    // OAuth tokens
    struct OAuth1Token {
        QString oauth_token;
        QString oauth_token_secret;
        QString mfa_token;
        qint64 mfa_expiration_timestamp;
    };

    struct OAuth2Token {
        QString access_token;
        QString refresh_token;
        QString token_type;
        qint64 expires_at;
        qint64 refresh_token_expires_at;

        bool isExpired() const {
            return QDateTime::currentSecsSinceEpoch() >= expires_at;
        }

        bool isRefreshExpired() const {
            return QDateTime::currentSecsSinceEpoch() >= refresh_token_expires_at;
        }
    };

    OAuth1Token m_oauth1Token;
    OAuth2Token m_oauth2Token;

    // Constants
    static constexpr const char* USER_AGENT = "com.garmin.android.apps.connectmobile";
    static constexpr const char* SSO_URL_PATH = "/sso/signin";
    static constexpr const char* SSO_EMBED_PATH = "/sso/embed";
    static constexpr const char* OAUTH_CONSUMER_URL = "https://thegarth.s3.amazonaws.com/oauth_consumer.json";

    // Private methods
    QString ssoUrl() const { return QString("https://sso.%1").arg(m_domain); }
    QString connectApiUrl() const { return QString("https://connectapi.%1").arg(m_domain); }

    bool fetchCookies();
    bool fetchCsrfToken();
    bool performLogin(const QString &email, const QString &password);
    bool performMfaVerification(const QString &mfaCode);
    bool exchangeForOAuth1Token(const QString &ticket);
    bool exchangeForOAuth2Token();
    bool refreshOAuth2Token();

    void loadTokensFromSettings();
    void saveTokensToSettings();
    void clearTokens();

    QNetworkReply* makeRequest(const QString &url, const QByteArray &data,
                               const QString &contentType, const QList<QNetworkCookie> &cookies = QList<QNetworkCookie>());
    QJsonObject extractJsonFromResponse(QNetworkReply *reply);
};

#endif // GARMINCONNECT_H
