#include "garminconnect.h"
#include "qzsettings.h"
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QFileInfo>
#include <QNetworkCookieJar>
#include <QUrl>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUuid>
#include <QDateTime>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

bool GarminConnect::uploadFitFile(const QString &fitFilePath)
{
    qDebug() << "GarminConnect: Uploading FIT file:" << fitFilePath;
    m_lastError.clear();

    // Check if authenticated
    if (!isAuthenticated()) {
        // Try to refresh token if we have a valid refresh_token
        if (!m_oauth2Token.refresh_token.isEmpty() && !m_oauth2Token.isRefreshExpired()) {
            qDebug() << "GarminConnect: Access token expired, attempting refresh...";
            if (!refreshOAuth2Token()) {
                m_lastError = "Failed to refresh token. Please login again.";
                qDebug() << "GarminConnect:" << m_lastError;
                emit uploadFailed(m_lastError);
                return false;
            }
            qDebug() << "GarminConnect: Token refreshed successfully";
        } else {
            m_lastError = "Not authenticated. Please login first.";
            qDebug() << "GarminConnect:" << m_lastError;
            emit uploadFailed(m_lastError);
            return false;
        }
    }

    // Check if file exists
    QFile fitFile(fitFilePath);
    if (!fitFile.exists()) {
        m_lastError = "FIT file does not exist: " + fitFilePath;
        qDebug() << "GarminConnect:" << m_lastError;
        emit uploadFailed(m_lastError);
        return false;
    }

    if (!fitFile.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to open FIT file: " + fitFile.errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        emit uploadFailed(m_lastError);
        return false;
    }

    QByteArray fitData = fitFile.readAll();
    fitFile.close();

    if (fitData.isEmpty()) {
        m_lastError = "FIT file is empty";
        qDebug() << "GarminConnect:" << m_lastError;
        emit uploadFailed(m_lastError);
        return false;
    }

    qDebug() << "GarminConnect: FIT file size:" << fitData.size() << "bytes";

    // Prepare multipart form data
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(fitFilePath).fileName() + "\""));
    filePart.setBody(fitData);
    multiPart->append(filePart);

    // Prepare request
    QUrl url(connectApiUrl() + "/upload-service/upload/.fit");
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);

    // CRITICAL: Force connection close to prevent SSL connection reuse issues
    // Without this, consecutive uploads fail with "SSL routines:ssl3_read_bytes:sslv3 alert bad record mac"
    // because QNetworkAccessManager tries to reuse a stale SSL connection
    request.setRawHeader("Connection", "close");

    // Use OAuth2 Bearer token for authorization
    QString authHeader = "Bearer " + m_oauth2Token.access_token;
    request.setRawHeader("Authorization", authHeader.toUtf8());

    qDebug() << "GarminConnect: Uploading to:" << url.toString();
    qDebug() << "GarminConnect: Using OAuth2 access token (length:" << m_oauth2Token.access_token.length() << ")";

    // Send POST request
    QNetworkReply *reply = m_manager->post(request, multiPart);
    multiPart->setParent(reply); // Delete multiPart with reply

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray responseBody = reply->readAll();

    qDebug() << "GarminConnect: Upload HTTP status code:" << statusCode;
    qDebug() << "GarminConnect: Upload response:" << QString::fromUtf8(responseBody);

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "Upload failed (HTTP " + QString::number(statusCode) + "): " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        emit uploadFailed(m_lastError);
        return false;
    }

    // Parse JSON response
    QJsonDocument doc = QJsonDocument::fromJson(responseBody);
    if (!doc.isObject()) {
        m_lastError = "Invalid JSON response from upload";
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        emit uploadFailed(m_lastError);
        return false;
    }

    QJsonObject obj = doc.object();

    // Check for upload ID or success indicators
    if (obj.contains("detailedImportResult")) {
        QJsonObject importResult = obj["detailedImportResult"].toObject();
        qDebug() << "GarminConnect: Upload successful!";
        qDebug() << "GarminConnect: Upload ID:" << importResult["uploadId"].toVariant();

        if (importResult.contains("failures") && !importResult["failures"].toArray().isEmpty()) {
            QJsonArray failures = importResult["failures"].toArray();
            qDebug() << "GarminConnect: Upload had failures:" << failures;
            m_lastError = "Upload completed with failures: " + QString::fromUtf8(QJsonDocument(failures).toJson());
            reply->deleteLater();
            emit uploadFailed(m_lastError);
            return false;
        }

        reply->deleteLater();
        emit uploadSucceeded();
        return true;
    } else {
        qDebug() << "GarminConnect: Upload successful (basic response)";
        reply->deleteLater();
        emit uploadSucceeded();
        return true;
    }
}

bool GarminConnect::login(const QString &email, const QString &password, const QString &mfaCode)
{
    qDebug() << "GarminConnect: Starting login process...";
    m_lastError.clear();

    // Store credentials for MFA flow continuation
    m_pendingEmail = email;
    m_pendingPassword = password;

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
    // If we have an MFA code, suppress the mfaRequired signal since we're retrying with the code
    QString ticket;
    if (!performLogin(email, password, !mfaCode.isEmpty())) {
        // Check if MFA is required
        if (m_lastError.contains("MFA", Qt::CaseInsensitive)) {
            // Only emit mfaRequired if we don't already have an MFA code
            // This prevents showing the MFA dialog multiple times when retrying with a code
            if (mfaCode.isEmpty()) {
                emit mfaRequired();
                m_lastError = "MFA code required";
                emit authenticationFailed(m_lastError);
                return false;
            }
            // Perform MFA verification with provided code (async - signals will be emitted)
            performMfaVerification(mfaCode);
            return true;  // Return true to indicate the async flow has started
        } else {
            emit authenticationFailed(m_lastError);
            return false;
        }
    }

    qDebug() << "GarminConnect: Login successful!";
    emit authenticated();
    return true;
}

void GarminConnect::submitMfaCode(const QString &mfaCode)
{
    qDebug() << "GarminConnect: Submitting MFA code (continuing authentication flow)...";
    m_lastError.clear();

    if (mfaCode.isEmpty()) {
        m_lastError = "MFA code cannot be empty";
        emit authenticationFailed(m_lastError);
        return;
    }

    // Perform MFA verification asynchronously - signals will be emitted when complete
    performMfaVerification(mfaCode);
}

bool GarminConnect::tryRefreshToken()
{
    // Already authenticated and not expired - no need to refresh
    if (isAuthenticated()) {
        qDebug() << "GarminConnect: Token is still valid, no refresh needed";
        return true;
    }

    // Check if we have a valid refresh token (actually OAuth1 token)
    if (m_oauth1Token.oauth_token.isEmpty() || m_oauth1Token.oauth_token_secret.isEmpty()) {
        qDebug() << "GarminConnect: No valid OAuth1 token available for refresh";
        return false;
    }

    // Token expired - refresh using OAuth1 token (garth method)
    qDebug() << "GarminConnect: ===== TOKEN REFRESH ATTEMPT =====";
    qDebug() << "GarminConnect: Current time:" << QDateTime::currentDateTime().toString(Qt::ISODate);
    qDebug() << "GarminConnect: access_token expired at:"
             << QDateTime::fromSecsSinceEpoch(m_oauth2Token.expires_at).toString(Qt::ISODate);
    qDebug() << "GarminConnect: Token expired, refreshing...";

    bool success = refreshOAuth2Token();

    if (success) {
        qDebug() << "GarminConnect: Token refresh successful!";
        qDebug() << "GarminConnect: New access_token expires at:"
                 << QDateTime::fromSecsSinceEpoch(m_oauth2Token.expires_at).toString(Qt::ISODate);
        qDebug() << "GarminConnect: ============================";
    } else {
        qDebug() << "GarminConnect: Token refresh failed:" << m_lastError;
        qDebug() << "GarminConnect: ============================";
    }

    return success;
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

    QString ssoEmbedUrl = ssoUrl() + SSO_EMBED_PATH;

    QUrl url(ssoUrl() + SSO_URL_PATH);
    QUrlQuery query;
    query.addQueryItem("id", "gauth-widget");
    query.addQueryItem("embedWidget", "true");
    query.addQueryItem("gauthHost", ssoEmbedUrl);  // Should be ssoEmbedUrl for GET signin
    query.addQueryItem("service", ssoEmbedUrl);
    query.addQueryItem("source", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
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

bool GarminConnect::performLogin(const QString &email, const QString &password, bool suppressMfaSignal)
{
    qDebug() << "GarminConnect: Performing login...";
    qDebug() << "GarminConnect: Using domain:" << m_domain;
    qDebug() << "GarminConnect: SSO URL:" << ssoUrl();
    qDebug() << "GarminConnect: Connect API URL:" << connectApiUrl();

    QString ssoEmbedUrl = ssoUrl() + SSO_EMBED_PATH;

    QUrl url(ssoUrl() + SSO_URL_PATH);
    QUrlQuery query;
    query.addQueryItem("id", "gauth-widget");
    query.addQueryItem("embedWidget", "true");
    query.addQueryItem("gauthHost", ssoEmbedUrl);
    query.addQueryItem("service", ssoEmbedUrl);  // Python uses SSO_EMBED for both GET and POST!
    query.addQueryItem("source", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
    url.setQuery(query);

    // Prepare POST data
    QUrlQuery postData;
    postData.addQueryItem("username", email);
    postData.addQueryItem("password", password);
    postData.addQueryItem("embed", "true");
    postData.addQueryItem("_csrf", m_csrfToken);

    QString queryString = postData.query(QUrl::FullyEncoded);

    // CRITICAL: Fix '+' character encoding for usernames like "user+tag@example.com"
    // QUrlQuery doesn't percent-encode '+' in form data (treats it as space character)
    // but Garmin authentication requires literal '+' to be encoded as '%2B'
    // This is safe because in URL-encoded form data, '+' always means space,
    // so any literal '+' must be encoded as '%2B'
    queryString.replace("+", "%2B");

    QByteArray data = queryString.toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", url.toString().toUtf8());  // Add Referer header for security check
    request.setRawHeader("Origin", ssoUrl().toUtf8());  // Add Origin header

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

    // Check page title (like Python garth library)
    // garth checks ONLY the title for MFA detection, not the body
    // This is important because some servers (like garmin.cn) may have "MFA" text
    // in their Success page HTML body, which would cause false positives
    QString pageTitle;
    QRegularExpression titleRegex("<title>(.+?)</title>");
    QRegularExpressionMatch titleMatch = titleRegex.match(response);
    if (titleMatch.hasMatch()) {
        pageTitle = titleMatch.captured(1);
        qDebug() << "GarminConnect: Page title:" << pageTitle;
    }

    // Check if MFA is required by looking at the TITLE (garth approach)
    // This is more reliable than checking the body which may contain "MFA" in scripts/URLs
    if (pageTitle.contains("MFA", Qt::CaseInsensitive)) {
        m_lastError = "MFA Required";
        qDebug() << "GarminConnect: MFA detected in page title";

        // Extract new CSRF token from MFA page - try multiple patterns
        QRegularExpression csrfRegex1("name=\"_csrf\"[^>]*value=\"([^\"]+)\"");
        QRegularExpression csrfRegex2("value=\"([^\"]+)\"[^>]*name=\"_csrf\"");

        QRegularExpressionMatch match = csrfRegex1.match(response);
        if (!match.hasMatch()) {
            match = csrfRegex2.match(response);
        }
        if (match.hasMatch()) {
            m_csrfToken = match.captured(1);
            qDebug() << "GarminConnect: CSRF token from MFA page:" << m_csrfToken.left(20) << "...";
        }

        // Update cookies
        m_cookies = m_manager->cookieJar()->cookiesForUrl(url);

        if (!suppressMfaSignal) {
            qDebug() << "GarminConnect: Emitting mfaRequired signal";
            emit mfaRequired();
        } else {
            qDebug() << "GarminConnect: MFA required but signal suppressed (retrying with MFA code)";
        }
        reply->deleteLater();
        return false;
    }

    // Check if login was successful (title is "Success")
    if (pageTitle == "Success") {
        qDebug() << "GarminConnect: Login successful (Success page detected)";
        // Continue to extract ticket below
    }

    // Check for error messages in response
    if (response.contains("error", Qt::CaseInsensitive)) {
        QRegularExpression errorRegex("error[^>]*>([^<]+)<");
        QRegularExpressionMatch errorMatch = errorRegex.match(response);
        if (errorMatch.hasMatch()) {
            qDebug() << "GarminConnect: Error in response:" << errorMatch.captured(1);
        }
    }

    // Check if we're still on login page (failed login)
    if (response.contains("GARMIN Authentication Application")) {
        qDebug() << "GarminConnect: Still on login page - credentials may be incorrect";
        // Look for validation error
        QRegularExpression validationRegex("validation[^>]*>([^<]+)<");
        QRegularExpressionMatch validationMatch = validationRegex.match(response);
        if (validationMatch.hasMatch()) {
            qDebug() << "GarminConnect: Validation error:" << validationMatch.captured(1);
        }
    }

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

    // Check if redirect URL indicates MFA is required
    if (!responseUrl.isEmpty() && responseUrl.toString().contains("verifyMFA", Qt::CaseInsensitive)) {
        m_lastError = "MFA Required";
        qDebug() << "GarminConnect: MFA redirect detected - fetching MFA page for CSRF token";

        reply->deleteLater();

        // Follow redirect to get MFA page and extract new CSRF token
        QNetworkRequest mfaRequest(responseUrl);
        mfaRequest.setRawHeader("User-Agent", USER_AGENT);
        mfaRequest.setRawHeader("Referer", url.toString().toUtf8());

        // Update cookies from login response
        m_cookies = m_manager->cookieJar()->cookiesForUrl(url);
        for (const QNetworkCookie &cookie : m_cookies) {
            m_manager->cookieJar()->insertCookie(cookie);
        }

        QNetworkReply *mfaReply = m_manager->get(mfaRequest);
        QEventLoop mfaLoop;
        connect(mfaReply, &QNetworkReply::finished, &mfaLoop, &QEventLoop::quit);
        mfaLoop.exec();

        if (mfaReply->error() == QNetworkReply::NoError) {
            QString mfaHtml = QString::fromUtf8(mfaReply->readAll());

            // Extract new CSRF token from MFA page
            QRegularExpression csrfRegex1("name=\"_csrf\"[^>]*value=\"([^\"]+)\"");
            QRegularExpression csrfRegex2("value=\"([^\"]+)\"[^>]*name=\"_csrf\"");

            QRegularExpressionMatch match = csrfRegex1.match(mfaHtml);
            if (!match.hasMatch()) {
                match = csrfRegex2.match(mfaHtml);
            }
            if (match.hasMatch()) {
                m_csrfToken = match.captured(1);
                qDebug() << "GarminConnect: New CSRF token from MFA page:" << m_csrfToken.left(20) << "...";
            }

            // Update cookies from MFA page
            m_cookies = m_manager->cookieJar()->cookiesForUrl(responseUrl);
        }

        mfaReply->deleteLater();

        if (!suppressMfaSignal) {
            qDebug() << "GarminConnect: Emitting mfaRequired signal";
            emit mfaRequired();
        } else {
            qDebug() << "GarminConnect: MFA required but signal suppressed (retrying with MFA code)";
        }
        return false;
    }

    // Extract ticket from response URL (already declared above)
    if (responseUrl.isEmpty()) {
        responseUrl = reply->url();
    }

    if (DEBUG_GARMIN_VERBOSE) {
        qDebug() << "GarminConnect: Response URL:" << responseUrl.toString();
        qDebug() << "GarminConnect: Response length:" << response.length();
        qDebug() << "GarminConnect: Full response body:" << response;
    }

    QUrlQuery responseQuery(responseUrl);
    QString ticket = responseQuery.queryItemValue("ticket");

    if (ticket.isEmpty()) {
        // Try to extract from response body using Python garth pattern
        QRegularExpression ticketRegex("embed\\?ticket=([^\"]+)\"");
        QRegularExpressionMatch match = ticketRegex.match(response);
        if (match.hasMatch()) {
            ticket = match.captured(1);
            qDebug() << "GarminConnect: Found ticket in response body:" << ticket.left(20) << "...";
        } else {
            // Fallback to generic ticket extraction
            QRegularExpression fallbackRegex("ticket=([^&\"']+)");
            match = fallbackRegex.match(response);
            if (match.hasMatch()) {
                ticket = match.captured(1);
                qDebug() << "GarminConnect: Found ticket with fallback pattern:" << ticket.left(20) << "...";
            } else if (DEBUG_GARMIN_VERBOSE) {
                qDebug() << "GarminConnect: No ticket patterns matched in response body";
            }
        }
    }

    reply->deleteLater();

    if (ticket.isEmpty()) {
        m_lastError = "Failed to extract ticket from login response";
        qDebug() << "GarminConnect:" << m_lastError;
        if (DEBUG_GARMIN_VERBOSE) {
            qDebug() << "GarminConnect: Response snippet:" << response.left(1000);
        }
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

void GarminConnect::performMfaVerification(const QString &mfaCode)
{
    qDebug() << "GarminConnect: Performing MFA verification...";

    m_pendingMfaCode = mfaCode;

    QString ssoEmbedUrl = ssoUrl() + SSO_EMBED_PATH;

    QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");
    // MFA endpoint requires same query parameters as signin!
    QUrlQuery query;
    query.addQueryItem("id", "gauth-widget");
    query.addQueryItem("embedWidget", "true");
    query.addQueryItem("gauthHost", ssoEmbedUrl);
    query.addQueryItem("service", ssoEmbedUrl);
    query.addQueryItem("source", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
    url.setQuery(query);

    // Prepare POST data
    QUrlQuery postData;
    postData.addQueryItem("mfa-code", mfaCode);
    postData.addQueryItem("embed", "true");
    postData.addQueryItem("fromPage", "setupEnterMfaCode");
    postData.addQueryItem("_csrf", m_csrfToken);

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", url.toString().toUtf8());  // Add Referer header
    request.setRawHeader("Origin", ssoUrl().toUtf8());  // Add Origin header

    // Add cookies
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
    }

    QNetworkReply *reply = m_manager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &GarminConnect::handleMfaReplyFinished);
}

void GarminConnect::handleMfaReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        m_lastError = "Invalid reply object";
        emit authenticationFailed(m_lastError);
        return;
    }

    QString ssoEmbedUrl = ssoUrl() + SSO_EMBED_PATH;
    QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");
    QUrlQuery query;
    query.addQueryItem("id", "gauth-widget");
    query.addQueryItem("embedWidget", "true");
    query.addQueryItem("gauthHost", ssoEmbedUrl);
    query.addQueryItem("service", ssoEmbedUrl);
    query.addQueryItem("source", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
    url.setQuery(query);

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "MFA verification failed: " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        emit authenticationFailed(m_lastError);
        return;
    }

    QString response = QString::fromUtf8(reply->readAll());

    // Check redirect URL for ticket (MFA response might also be a redirect)
    QUrl responseUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "GarminConnect: MFA response status code:" << statusCode;
    qDebug() << "GarminConnect: MFA response redirect URL:" << responseUrl.toString();

    // Log detailed response information
    if (DEBUG_GARMIN_VERBOSE) {
        qDebug() << "GarminConnect: MFA response length:" << response.length();
        qDebug() << "GarminConnect: Full MFA response body:" << response;
    } else if (responseUrl.isEmpty()) {
        // If no redirect, log response body to understand what happened (non-verbose)
        qDebug() << "GarminConnect: MFA response body (first 500 chars):" << response.left(500);
    }

    // Try to extract ticket from redirect URL first
    QString ticket;
    if (!responseUrl.isEmpty()) {
        QUrlQuery responseQuery(responseUrl);
        ticket = responseQuery.queryItemValue("ticket");
        if (!ticket.isEmpty()) {
            qDebug() << "GarminConnect: Found ticket in redirect URL:" << ticket.left(20) << "...";
        } else {
            // Check for logintoken - need to follow redirect to get actual ticket
            QString loginToken = responseQuery.queryItemValue("logintoken");
            if (!loginToken.isEmpty()) {
                qDebug() << "GarminConnect: Found logintoken in redirect, following to get ticket:" << loginToken;

                reply->deleteLater();

                // Follow the logintoken redirect to get the actual ticket
                QNetworkRequest tokenRequest(responseUrl);
                tokenRequest.setRawHeader("User-Agent", USER_AGENT);
                tokenRequest.setRawHeader("Referer", url.toString().toUtf8());

                // Update cookies
                m_cookies = m_manager->cookieJar()->cookiesForUrl(url);
                for (const QNetworkCookie &cookie : m_cookies) {
                    m_manager->cookieJar()->insertCookie(cookie);
                }

                QNetworkReply *tokenReply = m_manager->get(tokenRequest);
                connect(tokenReply, &QNetworkReply::finished, this, &GarminConnect::handleMfaLoginTokenReplyFinished);
                return;
            }
        }
    }

    // If not found in redirect URL, try response body
    if (ticket.isEmpty() && !response.isEmpty()) {
        if (DEBUG_GARMIN_VERBOSE) {
            qDebug() << "GarminConnect: Attempting to extract ticket from MFA response body";
        }
        // Try multiple patterns for ticket extraction
        QRegularExpression ticketRegex1("embed\\?ticket=([^\"]+)\"");
        QRegularExpression ticketRegex2("ticket=([^&\"']+)");

        QRegularExpressionMatch match = ticketRegex1.match(response);
        if (match.hasMatch()) {
            ticket = match.captured(1);
            qDebug() << "GarminConnect: Found ticket in response body (pattern 1):" << ticket.left(20) << "...";
        } else {
            match = ticketRegex2.match(response);
            if (match.hasMatch()) {
                ticket = match.captured(1);
                qDebug() << "GarminConnect: Found ticket in response body (pattern 2):" << ticket.left(20) << "...";
            } else if (DEBUG_GARMIN_VERBOSE) {
                qDebug() << "GarminConnect: No MFA ticket patterns matched. Checking for other patterns...";
                // Check for JSON format
                if (response.contains("ticket")) {
                    qDebug() << "GarminConnect: Response contains 'ticket' keyword, may be JSON or different format";
                }
                // Check for common response patterns
                if (response.contains("\"")) {
                    qDebug() << "GarminConnect: Response contains quoted strings (may be JSON)";
                }
            }
        }
    }

    reply->deleteLater();

    if (ticket.isEmpty()) {
        m_lastError = "Failed to extract ticket after MFA";
        qDebug() << "GarminConnect:" << m_lastError;
        if (DEBUG_GARMIN_VERBOSE) {
            qDebug() << "GarminConnect: Response snippet:" << response.left(1000);
        }
        emit authenticationFailed(m_lastError);
        return;
    }

    qDebug() << "GarminConnect: MFA verification successful";

    // Exchange ticket for OAuth1 token
    if (!exchangeForOAuth1Token(ticket)) {
        emit authenticationFailed(m_lastError);
        return;
    }

    // Exchange OAuth1 for OAuth2 token
    if (!exchangeForOAuth2Token()) {
        emit authenticationFailed(m_lastError);
        return;
    }

    saveTokensToSettings();
    qDebug() << "GarminConnect: Login successful!";
    emit authenticated();
}

void GarminConnect::handleMfaLoginTokenReplyFinished()
{
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply*>(sender());
    if (!tokenReply) {
        m_lastError = "Invalid token reply object";
        emit authenticationFailed(m_lastError);
        return;
    }

    QString ticket;

    if (tokenReply->error() == QNetworkReply::NoError) {
        // Check for another redirect with the ticket
        QUrl ticketUrl = tokenReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "GarminConnect: Logintoken redirect result:" << ticketUrl.toString();

        if (!ticketUrl.isEmpty()) {
            QUrlQuery ticketQuery(ticketUrl);
            ticket = ticketQuery.queryItemValue("ticket");
            if (!ticket.isEmpty()) {
                qDebug() << "GarminConnect: Found ticket after logintoken redirect:" << ticket.left(20) << "...";
            }
        }

        // If still no ticket, try response body
        if (ticket.isEmpty()) {
            QString tokenResponse = QString::fromUtf8(tokenReply->readAll());
            QRegularExpression ticketRegex1("embed\\?ticket=([^\"]+)\"");
            QRegularExpression ticketRegex2("ticket=([^&\"']+)");

            QRegularExpressionMatch match = ticketRegex1.match(tokenResponse);
            if (match.hasMatch()) {
                ticket = match.captured(1);
                qDebug() << "GarminConnect: Found ticket in logintoken response body (pattern 1)";
            } else {
                match = ticketRegex2.match(tokenResponse);
                if (match.hasMatch()) {
                    ticket = match.captured(1);
                    qDebug() << "GarminConnect: Found ticket in logintoken response body (pattern 2)";
                }
            }
        }
    }

    // CRITICAL: Update cookies after logintoken redirect
    // Garmin may set new session cookies during this redirect
    QUrl loginTokenUrl = tokenReply->url();
    m_cookies = m_manager->cookieJar()->cookiesForUrl(loginTokenUrl);
    qDebug() << "GarminConnect: Updated cookies after logintoken redirect, count:" << m_cookies.size();

    tokenReply->deleteLater();

    if (ticket.isEmpty()) {
        m_lastError = "Failed to extract ticket after logintoken redirect";
        qDebug() << "GarminConnect:" << m_lastError;
        emit authenticationFailed(m_lastError);
        return;
    }

    qDebug() << "GarminConnect: MFA verification successful";

    // Exchange ticket for OAuth1 token
    if (!exchangeForOAuth1Token(ticket)) {
        emit authenticationFailed(m_lastError);
        return;
    }

    // Exchange OAuth1 for OAuth2 token
    if (!exchangeForOAuth2Token()) {
        emit authenticationFailed(m_lastError);
        return;
    }

    saveTokensToSettings();
    qDebug() << "GarminConnect: Login successful!";
    emit authenticated();
}

bool GarminConnect::exchangeForOAuth1Token(const QString &ticket)
{
    qDebug() << "GarminConnect: Exchanging ticket for OAuth1 token...";

    // Fetch OAuth consumer credentials from S3
    QUrl consumerUrl(OAUTH_CONSUMER_URL);
    qDebug() << "GarminConnect: Fetching OAuth consumer from:" << OAUTH_CONSUMER_URL;
    QNetworkRequest consumerRequest(consumerUrl);
    consumerRequest.setRawHeader("User-Agent", USER_AGENT);
    consumerRequest.setRawHeader("Accept", "application/json, */*");
    consumerRequest.setRawHeader("Accept-Encoding", "gzip, deflate");

    QNetworkReply *consumerReply = m_manager->get(consumerRequest);
    QEventLoop consumerLoop;
    connect(consumerReply, &QNetworkReply::finished, &consumerLoop, &QEventLoop::quit);
    consumerLoop.exec();

    // Check HTTP status code
    int statusCode = consumerReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "GarminConnect: OAuth consumer HTTP status code:" << statusCode;

    if (consumerReply->error() != QNetworkReply::NoError || statusCode >= 400) {
        QByteArray errorBody = consumerReply->readAll();
        m_lastError = QString("Failed to fetch OAuth consumer (HTTP %1): %2")
                          .arg(statusCode)
                          .arg(consumerReply->errorString());
        qDebug() << "GarminConnect:" << m_lastError;
        qDebug() << "GarminConnect: Error response body:" << QString::fromUtf8(errorBody);
        consumerReply->deleteLater();
        return false;
    }

    QByteArray consumerData = consumerReply->readAll();
    qDebug() << "GarminConnect: OAuth consumer response received, length:" << consumerData.length();
    // NOTE: Consumer response contains sensitive data (consumer_secret) - not logging

    QJsonDocument consumerDoc = QJsonDocument::fromJson(consumerData);
    consumerReply->deleteLater();

    if (!consumerDoc.isObject()) {
        m_lastError = "Invalid OAuth consumer response - not a JSON object";
        qDebug() << "GarminConnect:" << m_lastError;
        qDebug() << "GarminConnect: JSON isNull:" << consumerDoc.isNull()
                 << "isArray:" << consumerDoc.isArray();
        qDebug() << "GarminConnect: Full response body:" << QString::fromUtf8(consumerData);
        return false;
    }

    QJsonObject consumerObj = consumerDoc.object();
    QString consumerKey = consumerObj["consumer_key"].toString();
    QString consumerSecret = consumerObj["consumer_secret"].toString();

    // Exchange ticket for OAuth1 token
    //
    // ===== URL ENCODING - HISTORY OF FAILED ATTEMPTS =====
    //
    // ATTEMPT 1: url.toString(FullyEncoded) + QUrl::fromEncoded()
    //   Code: encodedStr = url.toString(QUrl::FullyEncoded);
    //         request(QUrl::fromEncoded(encodedStr.toUtf8()));
    // RESULT: FAILED - Qt didn't encode ':', '/' in parameter values
    // ERROR: "Invalid URL encoding: not a valid digit (radix 16): 37"
    // REASON: url.toString(FullyEncoded) returned "login-url=https://..." (no encoding)
    //         Then fromEncoded() interpreted it but Qt still didn't encode on HTTP send
    //
    // ATTEMPT 2: query.toString(FullyEncoded) + manual concatenation + fromEncoded()
    //   Code: encodedStr = baseUrl + "?" + query.toString(QUrl::FullyEncoded);
    //         request.setUrl(QUrl::fromEncoded(encodedStr.toUtf8()));
    // RESULT: FAILED - Still no encoding of ':', '/' in values
    // ERROR: Same 401 error, URL still had "https://..." unencoded
    // REASON: query.toString(FullyEncoded) doesn't encode these characters either
    //
    // ATTEMPT 3: Manual percentEncode() for everything + fromEncoded()
    //   Code: queryStr = "ticket=" + percentEncode(ticket) + "&login-url=" + percentEncode(url);
    //         encodedStr = baseUrl + "?" + queryStr;
    //         request.setUrl(QUrl::fromEncoded(encodedStr.toUtf8()));
    // RESULT: FAILED - Multiple encoding (double/triple)
    // ERROR: Same 401, but now "https%%3A%%2F%%2F..." or "https%25%253A..."
    // REASON: percentEncode() correctly made "https%3A%2F%2F..."
    //         But QUrl::fromEncoded() decoded it back to "https://..."
    //         Then Qt re-encoded on HTTP send: "https%25%3A..." (double encoded)
    // COMMIT: b3c1b1b, 5a0ccf7
    //
    // ATTEMPT 4: Revert to Qt natural encoding (QUrlQuery.addQueryItem)
    //   Code: QUrlQuery query;
    //         query.addQueryItem("login-url", url);
    //         url.setQuery(query);
    // RESULT: FAILED - Back to no encoding of ':' and '/'
    // ERROR: "Invalid URL encoding..." - URL had "https://..." unencoded
    // REASON: Qt considers ':' and '/' as "unreserved" and won't encode them
    // COMMIT: 40ee928
    //
    // ATTEMPT 5: QUrl::toPercentEncoding() with DecodedMode
    //   Code: queryString = "ticket=" + QUrl::toPercentEncoding(ticket) + ...;
    //         url.setQuery(queryString, QUrl::DecodedMode);
    // RESULT: FAILED - Triple encoding!
    // ERROR: "Invalid URL encoding..." - URL had "https%253A%252F%252F..."
    // REASON: DecodedMode tells Qt "this string is NOT encoded, please encode it"
    //         So Qt encoded the '%' characters: %3A → %253A (triple encoding!)
    // COMMIT: (attempted but not committed - same session)
    //
    // ATTEMPT 6 (CURRENT): toPercentEncoding() + fromEncoded(StrictMode) + manual query parsing
    //   Code: queryString = "ticket=" + QUrl::toPercentEncoding(ticket) + ...;
    //         fullUrl = baseUrl + "?" + queryString;  // Complete URL as string
    //         url = QUrl::fromEncoded(fullUrl.toUtf8(), StrictMode);  // Parse as-is
    //         Use fullUrl STRING for signature (NOT url.toString())
    //         Fixed signature to manually parse query params with fromPercentEncoding()
    // REASON: - Build URL with correct encoding as string: "login-url=https%3A%2F%2F..."
    //         - fromEncoded(StrictMode) should preserve encoding
    //         - Using fullUrl for signature avoids Qt toString() re-encoding
    //         - Signature now properly: decode URL params → re-encode (OAuth1 spec)
    // STATUS: Testing now...
    //
    QUrl url(connectApiUrl() + "/oauth-service/oauth/preauthorized");

    // CRITICAL: Qt NEVER encodes ':' and '/' in parameter values (considers them "unreserved")
    // But OAuth1 and Garmin REQUIRE them to be encoded
    // Solution: Build complete URL string manually, use fromEncoded() to create QUrl
    QString queryString = "ticket=" + QString::fromUtf8(QUrl::toPercentEncoding(ticket)) +
                          "&login-url=" + QString::fromUtf8(QUrl::toPercentEncoding(ssoUrl() + SSO_EMBED_PATH)) +
                          "&accepts-mfa-tokens=true";

    // Build complete URL as string (this is what we'll use for signature and request)
    QString baseUrl = connectApiUrl() + "/oauth-service/oauth/preauthorized";
    QString fullUrl = baseUrl + "?" + queryString;

    // Create QUrl from the encoded string WITHOUT any further processing
    // StrictMode ensures Qt doesn't try to "fix" or re-encode anything
    url = QUrl::fromEncoded(fullUrl.toUtf8(), QUrl::StrictMode);

    QString prettyUrl = QUrl::fromPercentEncoding(fullUrl.toUtf8());

    qDebug() << "GarminConnect: ===== URL ENCODING DEBUG =====";
    qDebug() << "GarminConnect: URL (PrettyDecoded):" << prettyUrl;
    qDebug() << "GarminConnect: URL (FullyEncoded):" << fullUrl;
    qDebug() << "GarminConnect: Ticket value:" << ticket.left(30) << "...";

    // Check for double encoding markers
    if (fullUrl.contains("%%") || fullUrl.contains("%25%25")) {
        qDebug() << "GarminConnect: WARNING - Double encoding detected in URL!";
    }
    if (fullUrl.contains("%253A") || fullUrl.contains("%252F")) {
        qDebug() << "GarminConnect: WARNING - Triple encoding detected in URL!";
    }

    // Extract login-url parameter directly from our query string to verify encoding
    int loginUrlStart = queryString.indexOf("login-url=") + 10;  // Skip "login-url="
    int loginUrlEnd = queryString.indexOf("&", loginUrlStart);
    if (loginUrlEnd == -1) loginUrlEnd = queryString.length();
    QString loginUrl = queryString.mid(loginUrlStart, loginUrlEnd - loginUrlStart);
    qDebug() << "GarminConnect: login-url parameter (encoded):" << loginUrl;
    qDebug() << "GarminConnect: Expected: https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed";

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    // Note: Content-Type not needed for GET requests

    // Generate OAuth1 signature for GET request
    // CRITICAL: Must use the EXACT URL that Qt will send in the HTTP request
    // Use url.toEncoded() to get the actual bytes Qt will send
    QString urlForSignature = QString::fromUtf8(url.toEncoded(QUrl::FullyEncoded));

    qDebug() << "GarminConnect: URL for signature:" << urlForSignature;
    qDebug() << "GarminConnect: Original fullUrl:  " << fullUrl;
    qDebug() << "GarminConnect: URLs match:" << (urlForSignature == fullUrl);

    QString authHeader = generateOAuth1AuthorizationHeader(
        "GET",
        urlForSignature,  // Use the URL Qt will actually send
        consumerKey,
        consumerSecret,
        "",  // No token yet
        ""   // No token secret yet
    );
    request.setRawHeader("Authorization", authHeader.toUtf8());

    qDebug() << "GarminConnect: OAuth1 Authorization:" << authHeader.left(80) << "...";

    // CRITICAL: Add cookies from SSO login session
    // Garmin requires session continuity to validate the ticket
    qDebug() << "GarminConnect: Adding" << m_cookies.size() << "cookies to OAuth1 request";

    // Build Cookie header manually to ensure cookies are sent
    QStringList cookieValues;
    for (const QNetworkCookie &cookie : m_cookies) {
        qDebug() << "  Cookie:" << cookie.name() << "for domain:" << cookie.domain();
        m_manager->cookieJar()->insertCookie(cookie);
        // Also add to manual Cookie header
        cookieValues.append(QString::fromLatin1(cookie.name()) + "=" + QString::fromLatin1(cookie.value()));
    }

    // Set Cookie header explicitly
    if (!cookieValues.isEmpty()) {
        QString cookieHeader = cookieValues.join("; ");
        request.setRawHeader("Cookie", cookieHeader.toUtf8());
        qDebug() << "GarminConnect: Cookie header length:" << cookieHeader.length();
    }

    QNetworkReply *reply = m_manager->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Check HTTP status code first
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "GarminConnect: OAuth1 HTTP status code:" << statusCode;

    // Check for redirects
    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirectUrl.isEmpty()) {
        qDebug() << "GarminConnect: OAuth1 response is a redirect to:" << redirectUrl.toString();
    }

    // Read response body
    QString responseText = QString::fromUtf8(reply->readAll());
    qDebug() << "GarminConnect: OAuth1 response length:" << responseText.length();
    // NOTE: OAuth1 response contains sensitive tokens (oauth_token, oauth_token_secret, mfa_token) - not logging

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
        m_lastError = QString("OAuth1 exchange failed (HTTP %1): %2").arg(statusCode).arg(reply->errorString());
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        return false;
    }

    reply->deleteLater();

    // Check if response is empty
    if (responseText.isEmpty()) {
        m_lastError = "OAuth1 response is empty - possible redirect or authentication failure";
        qDebug() << "GarminConnect:" << m_lastError;
        qDebug() << "GarminConnect: This usually means cookies or signature are incorrect";
        return false;
    }

    qDebug() << "GarminConnect: Parsing OAuth1 response...";

    QUrlQuery responseQuery(responseText);
    m_oauth1Token.oauth_token = responseQuery.queryItemValue("oauth_token");
    m_oauth1Token.oauth_token_secret = responseQuery.queryItemValue("oauth_token_secret");
    m_oauth1Token.mfa_token = responseQuery.queryItemValue("mfa_token");

    bool ok;
    m_oauth1Token.mfa_expiration_timestamp = responseQuery.queryItemValue("mfa_expiration_timestamp").toLongLong(&ok);

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

    // Prepare POST data - only include mfa_token if present
    // oauth_token and oauth_token_secret go in OAuth1 signature, NOT in POST body!
    QUrlQuery postData;
    if (!m_oauth1Token.mfa_token.isEmpty()) {
        postData.addQueryItem("mfa_token", m_oauth1Token.mfa_token);
    }

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

    // Need consumer key/secret from settings or fetch again
    // For simplicity, fetch consumer credentials again (cached by Garmin)
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

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // Extract POST body parameters for OAuth1 signature
    // Per OAuth1 spec: POST body params with application/x-www-form-urlencoded
    // MUST be included in signature calculation
    QMap<QString, QString> postBodyParamsMap;
    if (!m_oauth1Token.mfa_token.isEmpty()) {
        postBodyParamsMap["mfa_token"] = m_oauth1Token.mfa_token;
    }

    // Generate OAuth1 signature for POST request with OAuth1 credentials
    // CRITICAL: Use FullyEncoded URL to match HTTP request encoding
    // CRITICAL: Include POST body params in signature (OAuth1 RFC 5849, Section 3.4.1.3.1)
    QString authHeader = generateOAuth1AuthorizationHeader(
        "POST",
        url.toString(QUrl::FullyEncoded),
        consumerKey,
        consumerSecret,
        m_oauth1Token.oauth_token,
        m_oauth1Token.oauth_token_secret,
        postBodyParamsMap
    );
    request.setRawHeader("Authorization", authHeader.toUtf8());

    qDebug() << "GarminConnect: OAuth2 exchange with OAuth1 signature";

    // CRITICAL: Add cookies from SSO login session
    // Garmin requires session continuity for OAuth2 exchange
    QStringList cookieValues;
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
        // Also add to manual Cookie header
        cookieValues.append(QString::fromLatin1(cookie.name()) + "=" + QString::fromLatin1(cookie.value()));
    }

    // Set Cookie header explicitly
    if (!cookieValues.isEmpty()) {
        QString cookieHeader = cookieValues.join("; ");
        request.setRawHeader("Cookie", cookieHeader.toUtf8());
        qDebug() << "GarminConnect: Cookie header set for OAuth2, length:" << cookieHeader.length();
    }

    QNetworkReply *reply = m_manager->post(request, data);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Debug: Check HTTP status and response
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray responseBody = reply->readAll();

    qDebug() << "GarminConnect: OAuth2 HTTP status code:" << statusCode;
    qDebug() << "GarminConnect: OAuth2 response length:" << responseBody.length();
    // NOTE: OAuth2 response contains sensitive tokens (access_token, refresh_token) - not logging

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = "OAuth2 exchange failed (HTTP " + QString::number(statusCode) + "): " + reply->errorString();
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        return false;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseBody);
    if (!jsonDoc.isObject()) {
        m_lastError = "Invalid OAuth2 JSON response";
        qDebug() << "GarminConnect:" << m_lastError;
        reply->deleteLater();
        return false;
    }

    QJsonObject jsonResponse = jsonDoc.object();
    reply->deleteLater();

    m_oauth2Token.access_token = jsonResponse["access_token"].toString();
    m_oauth2Token.refresh_token = jsonResponse["refresh_token"].toString();
    m_oauth2Token.token_type = jsonResponse["token_type"].toString();
    m_oauth2Token.expires_at = QDateTime::currentSecsSinceEpoch() + jsonResponse["expires_in"].toInt();
    m_oauth2Token.refresh_token_expires_at = QDateTime::currentSecsSinceEpoch() +
                                              jsonResponse["refresh_token_expires_in"].toInt();

    // Log token validity periods for debugging
    int expiresIn = jsonResponse["expires_in"].toInt();
    int refreshExpiresIn = jsonResponse["refresh_token_expires_in"].toInt();
    qDebug() << "GarminConnect: ===== TOKEN VALIDITY =====";
    qDebug() << "GarminConnect: access_token expires_in:" << expiresIn << "seconds ("
             << (expiresIn / 3600) << "hours," << (expiresIn / 86400) << "days)";
    qDebug() << "GarminConnect: refresh_token expires_in:" << refreshExpiresIn << "seconds ("
             << (refreshExpiresIn / 3600) << "hours," << (refreshExpiresIn / 86400) << "days)";
    qDebug() << "GarminConnect: access_token expires at:" << QDateTime::fromSecsSinceEpoch(m_oauth2Token.expires_at).toString(Qt::ISODate);
    qDebug() << "GarminConnect: refresh_token expires at:" << QDateTime::fromSecsSinceEpoch(m_oauth2Token.refresh_token_expires_at).toString(Qt::ISODate);
    qDebug() << "GarminConnect: ============================";

    if (m_oauth2Token.access_token.isEmpty()) {
        m_lastError = "Failed to get OAuth2 token";
        return false;
    }

    qDebug() << "GarminConnect: OAuth2 token obtained";
    return true;
}

bool GarminConnect::refreshOAuth2Token()
{
    qDebug() << "GarminConnect: Refreshing OAuth2 token using OAuth1 (garth method)...";

    // Check if we have OAuth1 token (lasts 1 year!)
    if (m_oauth1Token.oauth_token.isEmpty() || m_oauth1Token.oauth_token_secret.isEmpty()) {
        qDebug() << "GarminConnect: ⚠️  NO OAUTH1 TOKEN AVAILABLE ⚠️";
        qDebug() << "GarminConnect: Cannot refresh without OAuth1 token";
        qDebug() << "GarminConnect: You will need to login again with email/password/MFA";
        m_lastError = "OAuth1 token is empty, full login required";
        return false;
    }

    qDebug() << "GarminConnect: Using saved OAuth1 token to obtain fresh OAuth2 token";
    qDebug() << "GarminConnect: This is how garth refreshes - reuses OAuth1 token";

    // Use the existing exchangeForOAuth2Token() method which works correctly!
    // This is exactly what garth does: reuse OAuth1 token to get new OAuth2 token
    bool success = exchangeForOAuth2Token();

    if (success) {
        qDebug() << "GarminConnect: OAuth2 token refreshed successfully using OAuth1!";
    } else {
        qDebug() << "GarminConnect: OAuth2 refresh failed:" << m_lastError;
        qDebug() << "GarminConnect: OAuth1 token may have expired (lasts ~1 year)";
    }

    return success;
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

    // CRITICAL: Force connection close to prevent SSL connection reuse issues
    // Without this, consecutive uploads fail with "SSL routines:ssl3_read_bytes:sslv3 alert bad record mac"
    // because QNetworkAccessManager tries to reuse a stale SSL connection
    request.setRawHeader("Connection", "close");

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
    m_oauth2Token.access_token = settings.value(QZSettings::garmin_access_token, QZSettings::default_garmin_access_token).toString();
    m_oauth2Token.refresh_token = settings.value(QZSettings::garmin_refresh_token, QZSettings::default_garmin_refresh_token).toString();
    m_oauth2Token.token_type = settings.value(QZSettings::garmin_token_type, QZSettings::default_garmin_token_type).toString();
    m_oauth2Token.expires_at = settings.value(QZSettings::garmin_expires_at, QZSettings::default_garmin_expires_at).toLongLong();
    m_oauth2Token.refresh_token_expires_at = settings.value(QZSettings::garmin_refresh_token_expires_at, QZSettings::default_garmin_refresh_token_expires_at).toLongLong();
    m_oauth1Token.oauth_token = settings.value(QZSettings::garmin_oauth1_token, QZSettings::default_garmin_oauth1_token).toString();
    m_oauth1Token.oauth_token_secret = settings.value(QZSettings::garmin_oauth1_token_secret, QZSettings::default_garmin_oauth1_token_secret).toString();
    m_domain = settings.value(QZSettings::garmin_domain, QZSettings::default_garmin_domain).toString();
    qDebug() << "GarminConnect: Loaded Garmin domain from settings:" << m_domain;

    if (!m_oauth2Token.access_token.isEmpty()) {
        qDebug() << "GarminConnect: Loaded tokens from settings (OAuth1 + OAuth2)";
        qint64 now = QDateTime::currentSecsSinceEpoch();
        qDebug() << "GarminConnect: ===== LOADED TOKEN STATUS =====";
        qDebug() << "GarminConnect: Current time:" << QDateTime::currentDateTime().toString(Qt::ISODate);
        qDebug() << "GarminConnect: access_token expires at:" << QDateTime::fromSecsSinceEpoch(m_oauth2Token.expires_at).toString(Qt::ISODate)
                 << (m_oauth2Token.expires_at > now ? "(valid)" : "(EXPIRED)");
        qDebug() << "GarminConnect: refresh_token expires at:" << QDateTime::fromSecsSinceEpoch(m_oauth2Token.refresh_token_expires_at).toString(Qt::ISODate)
                 << (m_oauth2Token.refresh_token_expires_at > now ? "(valid)" : "(EXPIRED)");
        if (m_oauth2Token.refresh_token_expires_at > now) {
            qint64 days = (m_oauth2Token.refresh_token_expires_at - now) / 86400;
            qint64 hours = ((m_oauth2Token.refresh_token_expires_at - now) % 86400) / 3600;
            qDebug() << "GarminConnect: refresh_token valid for:" << days << "days" << hours << "hours";
        }
        qDebug() << "GarminConnect: ============================";
    }
}

void GarminConnect::saveTokensToSettings()
{
    QSettings settings;
    settings.setValue(QZSettings::garmin_access_token, m_oauth2Token.access_token);
    settings.setValue(QZSettings::garmin_refresh_token, m_oauth2Token.refresh_token);
    settings.setValue(QZSettings::garmin_token_type, m_oauth2Token.token_type);
    settings.setValue(QZSettings::garmin_expires_at, m_oauth2Token.expires_at);
    settings.setValue(QZSettings::garmin_refresh_token_expires_at, m_oauth2Token.refresh_token_expires_at);
    settings.setValue(QZSettings::garmin_oauth1_token, m_oauth1Token.oauth_token);
    settings.setValue(QZSettings::garmin_oauth1_token_secret, m_oauth1Token.oauth_token_secret);
    settings.setValue(QZSettings::garmin_domain, m_domain);
    settings.setValue(QZSettings::garmin_last_refresh, QDateTime::currentDateTime());

    qDebug() << "GarminConnect: Tokens saved to settings (OAuth1 + OAuth2)";
}

void GarminConnect::clearTokens()
{
    QSettings settings;
    settings.remove(QZSettings::garmin_access_token);
    settings.remove(QZSettings::garmin_refresh_token);
    settings.remove(QZSettings::garmin_token_type);
    settings.remove(QZSettings::garmin_expires_at);
    settings.remove(QZSettings::garmin_refresh_token_expires_at);
    settings.remove(QZSettings::garmin_last_refresh);

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

// OAuth1 Signature Implementation
QString GarminConnect::generateOAuth1AuthorizationHeader(
    const QString &httpMethod,
    const QString &url,
    const QString &consumerKey,
    const QString &consumerSecret,
    const QString &oauth_token,
    const QString &oauth_token_secret,
    const QMap<QString, QString> &postBodyParams)
{
    // 1. Generate OAuth parameters
    QString nonce = generateNonce();
    QString timestamp = generateTimestamp();

    // 2. Build parameter map
    QMap<QString, QString> params;
    params["oauth_consumer_key"] = consumerKey;
    params["oauth_nonce"] = nonce;
    params["oauth_signature_method"] = "HMAC-SHA1";
    params["oauth_timestamp"] = timestamp;
    params["oauth_version"] = "1.0";

    if (!oauth_token.isEmpty()) {
        params["oauth_token"] = oauth_token;
    }

    // ==============================================================================
    // QUERY PARAMETER PARSING FOR OAUTH1 SIGNATURE
    // ==============================================================================
    // HISTORY OF ATTEMPTS TO FIX OAUTH1 ENCODING:
    //
    // SIGNATURE PARSING ATTEMPT 1 (original): QUrlQuery with PrettyDecoded
    //   Code: QUrlQuery urlQuery(qurl.query());
    //         queryItems = urlQuery.queryItems(QUrl::PrettyDecoded);
    //         params[key] = value;  // Then percentEncode(value) in signature
    // RESULT: Created double/triple encoding because:
    //   - If URL had "login-url=https%3A%2F%2Fsso.com" (single encoded)
    //   - PrettyDecoded gave "https://sso.com" (decoded)
    //   - percentEncode() created "https%253A%252F%252Fsso.com" (double encoded)
    //
    // SIGNATURE PARSING ATTEMPT 2 (current): Manual parsing with fromPercentEncoding
    //   Code: Extract query string manually, split by '&'
    //         For each param: decode with QUrl::fromPercentEncoding()
    //         Store decoded value, then percentEncode() in signature
    // REASON: OAuth1 spec requires: decode URL params → re-encode for signature
    //   This creates intentional "double encoding" in signature base string,
    //   but should match what OAuth servers expect
    // ==============================================================================

    // Parse URL query parameters and add them to params map
    // CRITICAL: Extract query params WITHOUT Qt decoding/re-encoding them
    // OAuth1 spec says: decode URL params, then re-encode for signature
    int queryStart = url.indexOf('?');
    QString queryString = (queryStart >= 0) ? url.mid(queryStart + 1) : QString();

    qDebug() << "GarminConnect: ========== OAuth1 SIGNATURE DEBUG ==========";
    qDebug() << "GarminConnect: URL being parsed:" << url;
    qDebug() << "GarminConnect: Query string extracted:" << queryString;

    // Parse query string manually to get DECODED values
    QStringList queryPairs = queryString.split('&', Qt::SkipEmptyParts);
    qDebug() << "GarminConnect: Number of query items extracted:" << queryPairs.size();
    qDebug() << "GarminConnect: Query parameters (DECODED VALUES for signature):";

    for (const QString &pair : queryPairs) {
        int eqPos = pair.indexOf('=');
        if (eqPos > 0) {
            QString key = pair.left(eqPos);
            QString encodedValue = pair.mid(eqPos + 1);
            // Decode the value from URL encoding
            QString decodedValue = QUrl::fromPercentEncoding(encodedValue.toUtf8());
            qDebug() << "  Query parameter:" << key << "(value length:" << decodedValue.length() << ")";
            params[key] = decodedValue;
        }
    }

    // Add POST body parameters to signature
    // Per OAuth1 spec (RFC 5849, Section 3.4.1.3.1): When Content-Type is
    // application/x-www-form-urlencoded, POST body params MUST be included
    if (!postBodyParams.isEmpty()) {
        qDebug() << "GarminConnect: Adding" << postBodyParams.size() << "POST body parameters to signature";
        for (auto it = postBodyParams.constBegin(); it != postBodyParams.constEnd(); ++it) {
            qDebug() << "  POST parameter:" << it.key() << "(value length:" << it.value().length() << ")";
            params[it.key()] = it.value();
        }
    }

    // 3. Create parameter string (sorted by key)
    qDebug() << "GarminConnect: Building signature with" << params.size() << "parameters";
    QString parameterString;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        // Only log keys, not values (values may contain sensitive tokens)
        qDebug() << "  Parameter:" << it.key();
        if (it != params.constBegin()) parameterString += "&";
        parameterString += percentEncode(it.key()) + "=" + percentEncode(it.value());
    }
    qDebug() << "GarminConnect: Parameter string length:" << parameterString.length();
    // NOTE: Parameter string may contain sensitive data (mfa_token, tickets) - not logging full string

    // 4. Create base URL (without query string)
    QUrl baseUrl(url);
    baseUrl.setQuery(QString()); // Remove query from URL
    QString baseUrlStr = baseUrl.scheme() + "://" + baseUrl.host() + baseUrl.path();
    qDebug() << "GarminConnect: Base URL (no query):" << baseUrlStr;
    qDebug() << "GarminConnect: Base URL (encoded):" << percentEncode(baseUrlStr);

    // 5. Generate signature
    qDebug() << "GarminConnect: Generating signature with:";
    qDebug() << "  HTTP Method:" << httpMethod;
    qDebug() << "  Consumer Secret length:" << consumerSecret.length();
    qDebug() << "  Token Secret length:" << oauth_token_secret.length();
    QString signature = generateOAuth1Signature(
        httpMethod,
        baseUrlStr,
        parameterString,
        consumerSecret,
        oauth_token_secret
    );
    qDebug() << "GarminConnect: Final signature:" << signature;

    // 6. Build authorization header
    QString authHeader = "OAuth ";
    authHeader += "oauth_consumer_key=\"" + percentEncode(consumerKey) + "\", ";
    authHeader += "oauth_nonce=\"" + percentEncode(nonce) + "\", ";
    authHeader += "oauth_signature=\"" + percentEncode(signature) + "\", ";
    authHeader += "oauth_signature_method=\"HMAC-SHA1\", ";
    authHeader += "oauth_timestamp=\"" + percentEncode(timestamp) + "\", ";
    authHeader += "oauth_version=\"1.0\"";

    if (!oauth_token.isEmpty()) {
        authHeader += ", oauth_token=\"" + percentEncode(oauth_token) + "\"";
    }

    return authHeader;
}

QString GarminConnect::generateOAuth1Signature(
    const QString &httpMethod,
    const QString &baseUrl,
    const QString &parameterString,
    const QString &consumerSecret,
    const QString &oauth_token_secret)
{
    // Build signing key: consumer_secret&token_secret
    QString key = percentEncode(consumerSecret) + "&";
    if (!oauth_token_secret.isEmpty()) {
        key += percentEncode(oauth_token_secret);
    }

    qDebug() << "GarminConnect: ========== SIGNATURE GENERATION ==========";
    qDebug() << "GarminConnect: Signing key (consumer_secret&token_secret):";
    qDebug() << "  Consumer secret (encoded):" << percentEncode(consumerSecret);
    qDebug() << "  Token secret (encoded):" << (oauth_token_secret.isEmpty() ? "(empty)" : percentEncode(oauth_token_secret));
    qDebug() << "  Full key length:" << key.length();

    // Build base string: METHOD&baseUrl&params
    QString baseString = httpMethod.toUpper() + "&" + percentEncode(baseUrl) +
                         "&" + percentEncode(parameterString);

    qDebug() << "GarminConnect: Base string length:" << baseString.length();
    // NOTE: Base string contains sensitive data (mfa_token, tickets, parameters) - not logging

    // HMAC-SHA1
    QByteArray signature = QMessageAuthenticationCode::hash(
        baseString.toUtf8(),
        key.toUtf8(),
        QCryptographicHash::Sha1
    );

    QString signatureBase64 = QString::fromLatin1(signature.toBase64());
    qDebug() << "GarminConnect: HMAC-SHA1 signature (base64):" << signatureBase64;
    qDebug() << "GarminConnect: Signature length:" << signatureBase64.length();

    return signatureBase64;
}

QString GarminConnect::percentEncode(const QString &str)
{
    // Use Qt's built-in percent encoding for consistency
    // This ensures we use the same encoding method as QUrl::toPercentEncoding()
    // which we use to build the HTTP request URL
    return QString::fromUtf8(QUrl::toPercentEncoding(str));
}

QString GarminConnect::generateNonce()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).replace("-", "");
}

QString GarminConnect::generateTimestamp()
{
    return QString::number(QDateTime::currentSecsSinceEpoch());
}

// ========== Daily Workout Download ==========

static QString garminSecondsToTime(int seconds) {
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    return QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

static void appendGarminStep(QString &xml, const QJsonObject &step, int indent) {
    QString pad(indent * 4, QChar(' '));
    QString condTypeKey = step["endCondition"].toObject()["conditionTypeKey"].toString();
    double endConditionValue = step["endConditionValue"].toDouble();

    QString targetTypeKey;
    if (step["targetType"].isObject() && !step["targetType"].isNull()) {
        targetTypeKey = step["targetType"].toObject()["workoutTargetTypeKey"].toString();
    }

    QString attrs;
    if (condTypeKey == "time" && endConditionValue > 0) {
        attrs += QString(" duration=\"%1\"").arg(garminSecondsToTime(static_cast<int>(endConditionValue)));
    }
    if (targetTypeKey == "heart.rate.zone") {
        int hrMin = static_cast<int>(step["targetValueOne"].toDouble());
        int hrMax = static_cast<int>(step["targetValueTwo"].toDouble());
        if (hrMin > 0) attrs += QString(" hrmin=\"%1\"").arg(hrMin);
        if (hrMax > 0) attrs += QString(" hrmax=\"%1\"").arg(hrMax);
        attrs += " looptimehr=\"1\"";
    }
    xml += pad + "<row" + attrs + "/>\n";
}

static QString generateGarminWorkoutXml(const QJsonObject &workoutJson) {
    QString xml;
    xml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml += "<rows>\n";

    const QJsonArray segments = workoutJson["workoutSegments"].toArray();
    for (const QJsonValue &segVal : segments) {
        const QJsonArray steps = segVal.toObject()["workoutSteps"].toArray();
        for (const QJsonValue &stepVal : steps) {
            QJsonObject step = stepVal.toObject();
            if (step["type"].toString() == "RepeatGroupDTO") {
                int iterations = static_cast<int>(step["numberOfIterations"].toDouble());
                xml += QString("    <repeat times=\"%1\">\n").arg(iterations);
                const QJsonArray innerSteps = step["workoutSteps"].toArray();
                for (const QJsonValue &inner : innerSteps)
                    appendGarminStep(xml, inner.toObject(), 2);
                xml += "    </repeat>\n";
            } else {
                appendGarminStep(xml, step, 1);
            }
        }
    }

    xml += "</rows>\n";
    return xml;
}

void GarminConnect::downloadTodaysWorkout(const QString &saveDir) {
    if (m_oauth2Token.access_token.isEmpty()) {
        qDebug() << "GarminConnect: No access token for workout download";
        emit workoutDownloadFailed("Not authenticated");
        return;
    }

    QDate today = QDate::currentDate();
    int year = today.year();
    int month = today.month() - 1; // Garmin calendar API uses 0-indexed months
    int day = today.day();

    QString urlString =
        QString("%1/calendar-service/year/%2/month/%3/day/%4/start/1")
            .arg(connectApiUrl()).arg(year).arg(month).arg(day);

    QUrl url(urlString);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_oauth2Token.access_token).toUtf8());
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("NK", "NT");
    request.setRawHeader("Accept", "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);

    const QString todayStr = today.toString(Qt::ISODate);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager, todayStr, saveDir]() {
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray response = reply->readAll();
        reply->deleteLater();
        manager->deleteLater();

        qDebug() << "GarminConnect: Calendar API status:" << statusCode;

        if (statusCode != 200) {
            emit workoutDownloadFailed(QString("Calendar API failed (HTTP %1)").arg(statusCode));
            return;
        }

        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        if (!jsonDoc.isObject()) {
            emit workoutDownloadFailed("Invalid calendar response");
            return;
        }

        const QJsonArray items = jsonDoc.object()["calendarItems"].toArray();
        bool foundWorkout = false;

        for (const QJsonValue &itemVal : items) {
            QJsonObject item = itemVal.toObject();
            if (item["date"].toString() != todayStr) continue;

            const QString workoutUuid = item["workoutUuid"].toString();
            if (workoutUuid.isEmpty()) continue;

            const QString itemType = item["itemType"].toString();
            if (itemType != "fbtAdaptiveWorkout" && itemType != "workout") continue;

            QString workoutName = item["title"].toString();
            if (workoutName.isEmpty()) workoutName = "Workout";
            const QString sportTypeKey = item["sportTypeKey"].toString();

            qDebug() << "GarminConnect: Found workout for today:" << workoutName << "UUID:" << workoutUuid;
            downloadWorkoutDetails(workoutUuid, todayStr, workoutName, itemType, saveDir);
            foundWorkout = true;
        }

        if (!foundWorkout) {
            qDebug() << "GarminConnect: No workouts scheduled for today";
            emit noWorkoutFoundToday();
        }
    });

    qDebug() << "GarminConnect: Fetching calendar for" << todayStr;
}

void GarminConnect::downloadWorkoutDetails(const QString &uuid, const QString &date,
                                           const QString &workoutName, const QString &itemType,
                                           const QString &saveDir) {
    const QString apiPath = (itemType == "fbtAdaptiveWorkout") ? "fbt-adaptive" : "workout";
    const QString urlString =
        QString("%1/workout-service/%2/%3").arg(connectApiUrl()).arg(apiPath).arg(uuid);

    QUrl url2(urlString);
    QNetworkRequest request2(url2);
    request2.setRawHeader("Authorization", QString("Bearer %1").arg(m_oauth2Token.access_token).toUtf8());
    request2.setRawHeader("User-Agent", USER_AGENT);
    request2.setRawHeader("NK", "NT");
    request2.setRawHeader("Accept", "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request2);

    connect(reply, &QNetworkReply::finished, this, [this, reply, manager, date, workoutName, saveDir]() {
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray response = reply->readAll();
        reply->deleteLater();
        manager->deleteLater();

        qDebug() << "GarminConnect: Workout details status:" << statusCode;

        if (statusCode != 200) {
            emit workoutDownloadFailed(QString("Workout details failed (HTTP %1)").arg(statusCode));
            return;
        }

        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        if (!jsonDoc.isObject()) {
            emit workoutDownloadFailed("Invalid workout details response");
            return;
        }

        const QString xmlContent = generateGarminWorkoutXml(jsonDoc.object());

        // Sanitize filename
        QString safeName = workoutName;
        safeName.replace(QRegularExpression("[^a-zA-Z0-9_\\-]"), "_");

        // Create training/garmin/{date}/ directory
        const QString garminDir = saveDir + "/garmin/" + date;
        QDir dir;
        if (!dir.exists(garminDir))
            dir.mkpath(garminDir);

        const QString filename = garminDir + "/" + safeName + ".xml";
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(xmlContent.toUtf8());
            file.close();
            qDebug() << "GarminConnect: Workout saved to" << filename;
            emit workoutDownloaded(filename, workoutName);
        } else {
            qDebug() << "GarminConnect: Failed to write" << filename;
            emit workoutDownloadFailed(QString("Cannot write file: %1").arg(filename));
        }
    });

    qDebug() << "GarminConnect: Fetching workout details UUID:" << uuid;
}

// ========== End Daily Workout Download ==========
