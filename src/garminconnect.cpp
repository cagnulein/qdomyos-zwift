#include "garminconnect.h"
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QHttpPart>
#include <QNetworkCookieJar>
#include <QUrl>

GarminConnect::GarminConnect(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_domain("garmin.com")
{
    loadTokensFromSettings();
}

GarminConnect::~GarminConnect()
{
    if (m_manager) {
        delete m_manager;
        m_manager = nullptr;
    }
}

bool GarminConnect::isAuthenticated() const
{
    return !m_oauth2Token.access_token.isEmpty() && !m_oauth2Token.isExpired();
}

void GarminConnect::logout()
{
    clearTokens();
    emit authenticated();
}

bool GarminConnect::login(const QString &email, const QString &password, const QString &mfaCode)
{
    qDebug() << "GarminConnect: Starting login process...";
    m_lastError.clear();

    // Step 1: Fetch cookies from embed endpoint
    if (!fetchCookies()) {
        m_lastError = "Failed to fetch cookies";
        emit authenticationFailed(m_lastError);
        return false;
    }

    // Step 2: Get CSRF token
    if (!fetchCsrfToken()) {
        m_lastError = "Failed to fetch CSRF token";
        emit authenticationFailed(m_lastError);
        return false;
    }

    // Step 3: Perform login
    QString ticket;
    if (!performLogin(email, password)) {
        // Check if MFA is required
        if (m_lastError.contains("MFA", Qt::CaseInsensitive)) {
            emit mfaRequired();
            if (mfaCode.isEmpty()) {
                m_lastError = "MFA code required";
                emit authenticationFailed(m_lastError);
                return false;
            }
            // Perform MFA verification
            if (!performMfaVerification(mfaCode)) {
                emit authenticationFailed(m_lastError);
                return false;
            }
        } else {
            emit authenticationFailed(m_lastError);
            return false;
        }
    }

    qDebug() << "GarminConnect: Login successful!";
    emit authenticated();
    return true;
}

bool GarminConnect::fetchCookies()
{
    qDebug() << "GarminConnect: Fetching cookies...";

    QUrl url(ssoUrl() + SSO_EMBED_PATH);
    QUrlQuery query;
    query.addQueryItem("id", "gauth-widget");
    query.addQueryItem("embedWidget", "true");
    query.addQueryItem("gauthHost", ssoUrl() + "/sso");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);

    QNetworkReply *reply = m_manager->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "GarminConnect: Error fetching cookies:" << reply->errorString();
        reply->deleteLater();
        return false;
    }

    // Store cookies
    m_cookies = m_manager->cookieJar()->cookiesForUrl(url);
    qDebug() << "GarminConnect: Cookies fetched:" << m_cookies.size();

    reply->deleteLater();
    return true;
}

bool GarminConnect::fetchCsrfToken()
{
    qDebug() << "GarminConnect: Fetching CSRF token...";

    QUrl url(ssoUrl() + SSO_URL_PATH);
    QUrlQuery query;
    query.addQueryItem("service", connectApiUrl());
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);

    // Add cookies
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
    }

    QNetworkReply *reply = m_manager->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "GarminConnect: Error fetching CSRF token:" << reply->errorString();
        reply->deleteLater();
        return false;
    }

    QString html = QString::fromUtf8(reply->readAll());

    // Debug: log part of the HTML to see structure
    qDebug() << "GarminConnect: HTML snippet:" << html.left(500);

    // Extract CSRF token from HTML - try multiple patterns
    QRegularExpression csrfRegex1("name=\"_csrf\"[^>]*value=\"([^\"]+)\"");
    QRegularExpression csrfRegex2("value=\"([^\"]+)\"[^>]*name=\"_csrf\"");
    QRegularExpression csrfRegex3("<input[^>]*name=\"csrf\"[^>]*value=\"([^\"]+)\"");

    QRegularExpressionMatch match = csrfRegex1.match(html);
    if (!match.hasMatch()) {
        match = csrfRegex2.match(html);
    }
    if (!match.hasMatch()) {
        match = csrfRegex3.match(html);
    }

    if (match.hasMatch()) {
        m_csrfToken = match.captured(1);
        qDebug() << "GarminConnect: CSRF token found:" << m_csrfToken.left(20) << "...";
    } else {
        qDebug() << "GarminConnect: CSRF token not found in HTML";
        reply->deleteLater();
        return false;
    }

    // Update cookies
    m_cookies = m_manager->cookieJar()->cookiesForUrl(url);

    reply->deleteLater();
    return true;
}

bool GarminConnect::performLogin(const QString &email, const QString &password)
{
    qDebug() << "GarminConnect: Performing login...";

    QUrl url(ssoUrl() + SSO_URL_PATH);
    QUrlQuery query;
    query.addQueryItem("service", connectApiUrl());
    url.setQuery(query);

    // Prepare POST data
    QUrlQuery postData;
    postData.addQueryItem("username", email);
    postData.addQueryItem("password", password);
    postData.addQueryItem("embed", "true");
    postData.addQueryItem("_csrf", m_csrfToken);

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // Add cookies
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
    }

    QNetworkReply *reply = m_manager->post(request, data);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "Login failed: " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        return false;
    }

    QString response = QString::fromUtf8(reply->readAll());

    // Debug: log response details
    qDebug() << "GarminConnect: Login response length:" << response.length();
    qDebug() << "GarminConnect: Response snippet:" << response.left(300);

    // Check redirect URL
    QUrl responseUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    qDebug() << "GarminConnect: Redirect URL:" << responseUrl.toString();

    // Check all headers for ticket
    QList<QNetworkReply::RawHeaderPair> headers = reply->rawHeaderPairs();
    for (const auto &header : headers) {
        if (QString(header.first).contains("location", Qt::CaseInsensitive)) {
            qDebug() << "GarminConnect: Location header:" << QString(header.second);
        }
    }

    // Check if MFA is required
    if (response.contains("MFA", Qt::CaseInsensitive) ||
        response.contains("Enter MFA Code", Qt::CaseInsensitive)) {
        m_lastError = "MFA Required";

        // Extract new CSRF token from MFA page - try multiple patterns
        QRegularExpression csrfRegex1("name=\"_csrf\"[^>]*value=\"([^\"]+)\"");
        QRegularExpression csrfRegex2("value=\"([^\"]+)\"[^>]*name=\"_csrf\"");

        QRegularExpressionMatch match = csrfRegex1.match(response);
        if (!match.hasMatch()) {
            match = csrfRegex2.match(response);
        }
        if (match.hasMatch()) {
            m_csrfToken = match.captured(1);
        }

        // Update cookies
        m_cookies = m_manager->cookieJar()->cookiesForUrl(url);

        reply->deleteLater();
        return false;
    }

    // Extract ticket from response URL
    QUrl responseUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (responseUrl.isEmpty()) {
        responseUrl = reply->url();
    }

    QUrlQuery responseQuery(responseUrl);
    QString ticket = responseQuery.queryItemValue("ticket");

    if (ticket.isEmpty()) {
        // Try to extract from response body
        QRegularExpression ticketRegex("ticket=([^&\"']+)");
        QRegularExpressionMatch match = ticketRegex.match(response);
        if (match.hasMatch()) {
            ticket = match.captured(1);
        }
    }

    reply->deleteLater();

    if (ticket.isEmpty()) {
        m_lastError = "Failed to extract ticket from login response";
        qDebug() << "GarminConnect:" << m_lastError;
        return false;
    }

    qDebug() << "GarminConnect: Login successful, got ticket";

    // Exchange ticket for OAuth1 token
    if (!exchangeForOAuth1Token(ticket)) {
        return false;
    }

    // Exchange OAuth1 for OAuth2 token
    if (!exchangeForOAuth2Token()) {
        return false;
    }

    saveTokensToSettings();
    return true;
}

bool GarminConnect::performMfaVerification(const QString &mfaCode)
{
    qDebug() << "GarminConnect: Performing MFA verification...";

    QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");

    // Prepare POST data
    QUrlQuery postData;
    postData.addQueryItem("mfa-code", mfaCode);
    postData.addQueryItem("fromPage", "setupEnterMfaCode");
    postData.addQueryItem("_csrf", m_csrfToken);

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // Add cookies
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
    }

    QNetworkReply *reply = m_manager->post(request, data);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "MFA verification failed: " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        return false;
    }

    QString response = QString::fromUtf8(reply->readAll());

    // Extract ticket
    QRegularExpression ticketRegex("ticket=([^&\"']+)");
    QRegularExpressionMatch match = ticketRegex.match(response);
    QString ticket;
    if (match.hasMatch()) {
        ticket = match.captured(1);
    }

    reply->deleteLater();

    if (ticket.isEmpty()) {
        m_lastError = "Failed to extract ticket after MFA";
        qDebug() << "GarminConnect:" << m_lastError;
        return false;
    }

    qDebug() << "GarminConnect: MFA verification successful";

    // Exchange ticket for OAuth1 token
    if (!exchangeForOAuth1Token(ticket)) {
        return false;
    }

    // Exchange OAuth1 for OAuth2 token
    if (!exchangeForOAuth2Token()) {
        return false;
    }

    saveTokensToSettings();
    return true;
}

bool GarminConnect::exchangeForOAuth1Token(const QString &ticket)
{
    qDebug() << "GarminConnect: Exchanging ticket for OAuth1 token...";

    // First, fetch OAuth consumer credentials
    QUrl consumerUrl(OAUTH_CONSUMER_URL);
    QNetworkRequest consumerRequest(consumerUrl);
    consumerRequest.setRawHeader("User-Agent", USER_AGENT);

    QNetworkReply *consumerReply = m_manager->get(consumerRequest);
    QEventLoop consumerLoop;
    connect(consumerReply, &QNetworkReply::finished, &consumerLoop, &QEventLoop::quit);
    consumerLoop.exec();

    if (consumerReply->error() != QNetworkReply::NoError) {
        m_lastError = "Failed to fetch OAuth consumer: " + consumerReply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        consumerReply->deleteLater();
        return false;
    }

    QJsonDocument consumerDoc = QJsonDocument::fromJson(consumerReply->readAll());
    consumerReply->deleteLater();

    if (!consumerDoc.isObject()) {
        m_lastError = "Invalid OAuth consumer response";
        return false;
    }

    QJsonObject consumerObj = consumerDoc.object();
    QString consumerKey = consumerObj["consumer_key"].toString();
    QString consumerSecret = consumerObj["consumer_secret"].toString();

    // Exchange ticket for OAuth1 token
    QUrl url(connectApiUrl() + "/oauth-service/oauth/preauthorized");
    QUrlQuery query;
    query.addQueryItem("ticket", ticket);
    query.addQueryItem("login-url", ssoUrl() + SSO_EMBED_PATH);
    query.addQueryItem("accepts-mfa-tokens", "true");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // OAuth1 requires consumer key/secret in the request
    QString authHeader = QString("OAuth oauth_consumer_key=\"%1\"").arg(consumerKey);
    request.setRawHeader("Authorization", authHeader.toUtf8());

    QNetworkReply *reply = m_manager->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "OAuth1 exchange failed: " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        return false;
    }

    QJsonObject jsonResponse = extractJsonFromResponse(reply);
    reply->deleteLater();

    m_oauth1Token.oauth_token = jsonResponse["oauth_token"].toString();
    m_oauth1Token.oauth_token_secret = jsonResponse["oauth_token_secret"].toString();
    m_oauth1Token.mfa_token = jsonResponse["mfa_token"].toString();
    m_oauth1Token.mfa_expiration_timestamp = jsonResponse["mfa_expiration_timestamp"].toVariant().toLongLong();

    if (m_oauth1Token.oauth_token.isEmpty()) {
        m_lastError = "Failed to get OAuth1 token";
        return false;
    }

    qDebug() << "GarminConnect: OAuth1 token obtained";
    return true;
}

bool GarminConnect::exchangeForOAuth2Token()
{
    qDebug() << "GarminConnect: Exchanging OAuth1 for OAuth2 token...";

    QUrl url(connectApiUrl() + "/oauth-service/oauth/exchange/user/2.0");

    // Prepare POST data
    QUrlQuery postData;
    postData.addQueryItem("oauth_token", m_oauth1Token.oauth_token);
    postData.addQueryItem("oauth_token_secret", m_oauth1Token.oauth_token_secret);

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_manager->post(request, data);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "OAuth2 exchange failed: " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        return false;
    }

    QJsonObject jsonResponse = extractJsonFromResponse(reply);
    reply->deleteLater();

    m_oauth2Token.access_token = jsonResponse["access_token"].toString();
    m_oauth2Token.refresh_token = jsonResponse["refresh_token"].toString();
    m_oauth2Token.token_type = jsonResponse["token_type"].toString();
    m_oauth2Token.expires_at = QDateTime::currentSecsSinceEpoch() + jsonResponse["expires_in"].toInt();
    m_oauth2Token.refresh_token_expires_at = QDateTime::currentSecsSinceEpoch() +
                                              jsonResponse["refresh_token_expires_in"].toInt();

    if (m_oauth2Token.access_token.isEmpty()) {
        m_lastError = "Failed to get OAuth2 token";
        return false;
    }

    qDebug() << "GarminConnect: OAuth2 token obtained";
    return true;
}

bool GarminConnect::refreshOAuth2Token()
{
    qDebug() << "GarminConnect: Refreshing OAuth2 token...";

    if (m_oauth2Token.refresh_token.isEmpty() || m_oauth2Token.isRefreshExpired()) {
        m_lastError = "Refresh token is empty or expired, full login required";
        return false;
    }

    QUrl url(connectApiUrl() + "/oauth-service/oauth/exchange/user/2.0");

    // Prepare POST data
    QUrlQuery postData;
    postData.addQueryItem("refresh_token", m_oauth2Token.refresh_token);
    postData.addQueryItem("grant_type", "refresh_token");

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_manager->post(request, data);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "Token refresh failed: " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        return false;
    }

    QJsonObject jsonResponse = extractJsonFromResponse(reply);
    reply->deleteLater();

    m_oauth2Token.access_token = jsonResponse["access_token"].toString();
    m_oauth2Token.refresh_token = jsonResponse["refresh_token"].toString();
    m_oauth2Token.token_type = jsonResponse["token_type"].toString();
    m_oauth2Token.expires_at = QDateTime::currentSecsSinceEpoch() + jsonResponse["expires_in"].toInt();
    m_oauth2Token.refresh_token_expires_at = QDateTime::currentSecsSinceEpoch() +
                                              jsonResponse["refresh_token_expires_in"].toInt();

    saveTokensToSettings();
    qDebug() << "GarminConnect: Token refreshed successfully";
    return true;
}

bool GarminConnect::uploadActivity(const QByteArray &fitData, const QString &fileName)
{
    qDebug() << "GarminConnect: Uploading activity to Garmin Connect...";
    qDebug() << "File size:" << fitData.size() << "bytes";
    qDebug() << "Filename:" << fileName;

    // Check authentication
    if (m_oauth2Token.access_token.isEmpty()) {
        m_lastError = "Not authenticated";
        emit uploadFailed(m_lastError);
        return false;
    }

    // Refresh token if expired
    if (m_oauth2Token.isExpired()) {
        if (!refreshOAuth2Token()) {
            m_lastError = "Failed to refresh token";
            emit uploadFailed(m_lastError);
            return false;
        }
    }

    // Prepare multipart form data
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // Add FIT file part
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName)));
    filePart.setBody(fitData);
    multiPart->append(filePart);

    // Prepare upload request
    QUrl url(connectApiUrl() + "/upload-service/upload");
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "GCM-iOS-5.7.2.1");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_oauth2Token.access_token).toUtf8());

    // Perform upload
    QNetworkReply *reply = m_manager->post(request, multiPart);
    multiPart->setParent(reply); // Delete multiPart with reply

    // Monitor upload progress
    connect(reply, &QNetworkReply::uploadProgress,
            [](qint64 bytesSent, qint64 bytesTotal) {
                qDebug() << "GarminConnect upload progress:" << bytesSent << "/" << bytesTotal;
            });

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "Upload failed: " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        qDebug() << "HTTP status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "Response:" << reply->readAll();
        emit uploadFailed(m_lastError);
        reply->deleteLater();
        return false;
    }

    QByteArray response = reply->readAll();
    qDebug() << "GarminConnect: Upload response:" << response;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
    if (jsonDoc.isObject()) {
        QJsonObject jsonObj = jsonDoc.object();
        if (jsonObj.contains("detailedImportResult")) {
            QJsonObject importResult = jsonObj["detailedImportResult"].toObject();
            qDebug() << "Upload successful! Activity ID:" << importResult["activityId"].toVariant();
        }
    }

    reply->deleteLater();

    qDebug() << "GarminConnect: Upload completed successfully!";
    emit uploadSucceeded();
    return true;
}

void GarminConnect::loadTokensFromSettings()
{
    QSettings settings;
    m_oauth2Token.access_token = settings.value("garmin_access_token").toString();
    m_oauth2Token.refresh_token = settings.value("garmin_refresh_token").toString();
    m_oauth2Token.token_type = settings.value("garmin_token_type").toString();
    m_oauth2Token.expires_at = settings.value("garmin_expires_at").toLongLong();
    m_oauth2Token.refresh_token_expires_at = settings.value("garmin_refresh_token_expires_at").toLongLong();
    m_domain = settings.value("garmin_domain", "garmin.com").toString();

    if (!m_oauth2Token.access_token.isEmpty()) {
        qDebug() << "GarminConnect: Loaded tokens from settings";
    }
}

void GarminConnect::saveTokensToSettings()
{
    QSettings settings;
    settings.setValue("garmin_access_token", m_oauth2Token.access_token);
    settings.setValue("garmin_refresh_token", m_oauth2Token.refresh_token);
    settings.setValue("garmin_token_type", m_oauth2Token.token_type);
    settings.setValue("garmin_expires_at", m_oauth2Token.expires_at);
    settings.setValue("garmin_refresh_token_expires_at", m_oauth2Token.refresh_token_expires_at);
    settings.setValue("garmin_domain", m_domain);
    settings.setValue("garmin_last_refresh", QDateTime::currentDateTime());

    qDebug() << "GarminConnect: Tokens saved to settings";
}

void GarminConnect::clearTokens()
{
    QSettings settings;
    settings.remove("garmin_access_token");
    settings.remove("garmin_refresh_token");
    settings.remove("garmin_token_type");
    settings.remove("garmin_expires_at");
    settings.remove("garmin_refresh_token_expires_at");
    settings.remove("garmin_last_refresh");

    m_oauth1Token = OAuth1Token();
    m_oauth2Token = OAuth2Token();
    m_cookies.clear();
    m_csrfToken.clear();

    qDebug() << "GarminConnect: Tokens cleared";
}

QJsonObject GarminConnect::extractJsonFromResponse(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "GarminConnect: Failed to parse JSON response:" << data;
        return QJsonObject();
    }

    return doc.object();
}
