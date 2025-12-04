#include "garminconnect.h"
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QHttpPart>
#include <QNetworkCookieJar>
#include <QUrl>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUuid>
#include <QDateTime>

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

bool GarminConnect::performLogin(const QString &email, const QString &password)
{
    qDebug() << "GarminConnect: Performing login...";

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

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

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

    // Check for success title (like Python garth library)
    QRegularExpression titleRegex("<title>(.+?)</title>");
    QRegularExpressionMatch titleMatch = titleRegex.match(response);
    if (titleMatch.hasMatch()) {
        QString title = titleMatch.captured(1);
        qDebug() << "GarminConnect: Page title:" << title;
        if (title == "Success") {
            qDebug() << "GarminConnect: Login successful (Success page detected)";
        }
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

        qDebug() << "GarminConnect: Emitting mfaRequired signal";
        emit mfaRequired();
        return false;
    }

    // Check if MFA is required (legacy check for non-redirect MFA)
    if (response.contains("MFA", Qt::CaseInsensitive) ||
        response.contains("Enter MFA Code", Qt::CaseInsensitive)) {
        m_lastError = "MFA Required";
        qDebug() << "GarminConnect: MFA content detected in response";

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

        emit mfaRequired();
        reply->deleteLater();
        return false;
    }

    // Extract ticket from response URL (already declared above)
    if (responseUrl.isEmpty()) {
        responseUrl = reply->url();
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
            }
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

    // Check redirect URL for ticket (MFA response might also be a redirect)
    QUrl responseUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    qDebug() << "GarminConnect: MFA response redirect URL:" << responseUrl.toString();

    // Try to extract ticket from redirect URL first
    QString ticket;
    bool replyDeleted = false;
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
                replyDeleted = true;

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
                QEventLoop tokenLoop;
                connect(tokenReply, &QNetworkReply::finished, &tokenLoop, &QEventLoop::quit);
                tokenLoop.exec();

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

                tokenReply->deleteLater();
            }
        }
    }

    // If not found in redirect URL, try response body
    if (ticket.isEmpty() && !response.isEmpty()) {
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
            }
        }
    }

    // Only delete reply if not already deleted
    if (!replyDeleted) {
        reply->deleteLater();
    }

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
    query.addQueryItem("login-url", ssoUrl() + SSO_EMBED_PATH);  // Required by Python garth
    query.addQueryItem("accepts-mfa-tokens", "true");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    // Note: Content-Type not needed for GET requests

    // Generate OAuth1 signature for GET request
    QString authHeader = generateOAuth1AuthorizationHeader(
        "GET",
        url.toString(),
        consumerKey,
        consumerSecret,
        "",  // No token yet
        ""   // No token secret yet
    );
    request.setRawHeader("Authorization", authHeader.toUtf8());

    qDebug() << "GarminConnect: OAuth1 Authorization:" << authHeader.left(80) << "...";

    // CRITICAL: Add cookies from SSO login session
    // Garmin requires session continuity to validate the ticket
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
    }

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

    // Parse URL-encoded response (NOT JSON!)
    // Format: oauth_token=abc&oauth_token_secret=xyz&mfa_token=...
    QString responseText = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    qDebug() << "GarminConnect: OAuth1 response:" << responseText.left(100) << "...";

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

    // Generate OAuth1 signature for POST request with OAuth1 credentials
    QString authHeader = generateOAuth1AuthorizationHeader(
        "POST",
        url.toString(),
        consumerKey,
        consumerSecret,
        m_oauth1Token.oauth_token,
        m_oauth1Token.oauth_token_secret
    );
    request.setRawHeader("Authorization", authHeader.toUtf8());

    qDebug() << "GarminConnect: OAuth2 exchange with OAuth1 signature";

    // CRITICAL: Add cookies from SSO login session
    // Garmin requires session continuity for OAuth2 exchange
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
    }

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

// OAuth1 Signature Implementation
QString GarminConnect::generateOAuth1AuthorizationHeader(
    const QString &httpMethod,
    const QString &url,
    const QString &consumerKey,
    const QString &consumerSecret,
    const QString &oauth_token,
    const QString &oauth_token_secret)
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

    // Parse URL query parameters and add them to params map
    QUrl qurl(url);
    QUrlQuery urlQuery(qurl.query());
    QList<QPair<QString, QString>> queryItems = urlQuery.queryItems(QUrl::FullyDecoded);
    for (const auto &pair : queryItems) {
        params[pair.first] = pair.second;
    }

    // 3. Create parameter string (sorted by key)
    QString parameterString;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        if (it != params.constBegin()) parameterString += "&";
        parameterString += percentEncode(it.key()) + "=" + percentEncode(it.value());
    }

    // 4. Create base URL (without query string)
    QUrl baseUrl(url);
    baseUrl.setQuery(QString()); // Remove query from URL
    QString baseUrlStr = baseUrl.scheme() + "://" + baseUrl.host() + baseUrl.path();

    // 5. Generate signature
    QString signature = generateOAuth1Signature(
        httpMethod,
        baseUrlStr,
        parameterString,
        consumerSecret,
        oauth_token_secret
    );

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

    // Build base string: METHOD&baseUrl&params
    QString baseString = httpMethod.toUpper() + "&" + percentEncode(baseUrl) +
                         "&" + percentEncode(parameterString);

    // HMAC-SHA1
    QByteArray signature = QMessageAuthenticationCode::hash(
        baseString.toUtf8(),
        key.toUtf8(),
        QCryptographicHash::Sha1
    );

    return QString::fromLatin1(signature.toBase64());
}

QString GarminConnect::percentEncode(const QString &str)
{
    QString encoded;
    for (QChar c : str) {
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '.' || c == '_' || c == '~') {
            encoded += c;
        } else {
            QByteArray utf8 = QString(c).toUtf8();
            for (char byte : utf8) {
                encoded += QString("%%%1").arg((unsigned char)byte, 2, 16, QChar('0')).toUpper();
            }
        }
    }
    return encoded;
}

QString GarminConnect::generateNonce()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).replace("-", "");
}

QString GarminConnect::generateTimestamp()
{
    return QString::number(QDateTime::currentSecsSinceEpoch());
}
