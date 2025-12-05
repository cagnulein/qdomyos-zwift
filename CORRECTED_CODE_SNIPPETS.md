# Corrected Code Snippets

This document shows the corrected versions of the problematic functions in garminconnect.cpp.

---

## Fix 1: Correct Response Parsing for OAuth1 Token

**Location:** `garminconnect.cpp` - `exchangeForOAuth1Token()` function

**Problem:** Currently parses response as JSON, but it's URL-encoded form data

**Current Code (WRONG):**
```cpp
QJsonObject jsonResponse = extractJsonFromResponse(reply);  // ❌ JSON parsing
reply->deleteLater();

m_oauth1Token.oauth_token = jsonResponse["oauth_token"].toString();
m_oauth1Token.oauth_token_secret = jsonResponse["oauth_token_secret"].toString();
m_oauth1Token.mfa_token = jsonResponse["mfa_token"].toString();
m_oauth1Token.mfa_expiration_timestamp = jsonResponse["mfa_expiration_timestamp"].toVariant().toLongLong();
```

**Corrected Code:**
```cpp
QString responseText = QString::fromUtf8(reply->readAll());

// Parse as URL-encoded form data (key1=value1&key2=value2)
QUrlQuery responseQuery(responseText);
reply->deleteLater();

m_oauth1Token.oauth_token = responseQuery.queryItemValue("oauth_token");
m_oauth1Token.oauth_token_secret = responseQuery.queryItemValue("oauth_token_secret");
m_oauth1Token.mfa_token = responseQuery.queryItemValue("mfa_token");

bool ok = false;
QString expTimestampStr = responseQuery.queryItemValue("mfa_expiration_timestamp");
if (!expTimestampStr.isEmpty()) {
    m_oauth1Token.mfa_expiration_timestamp = expTimestampStr.toLongLong(&ok);
} else {
    m_oauth1Token.mfa_expiration_timestamp = 0;
}

qDebug() << "GarminConnect: Parsed OAuth1 response:"
         << "token=" << m_oauth1Token.oauth_token.left(10) << "..."
         << "has_secret=" << !m_oauth1Token.oauth_token_secret.isEmpty()
         << "has_mfa=" << !m_oauth1Token.mfa_token.isEmpty();
```

---

## Fix 2: Add Missing login-url Parameter

**Location:** `garminconnect.cpp` - `exchangeForOAuth1Token()` function

**Problem:** Missing `login-url` parameter in preauthorized request

**Current Code (WRONG):**
```cpp
QUrl url(connectApiUrl() + "/oauth-service/oauth/preauthorized");
QUrlQuery query;
query.addQueryItem("ticket", ticket);
query.addQueryItem("accepts-mfa-tokens", "true");
url.setQuery(query);
```

**Corrected Code:**
```cpp
QUrl url(connectApiUrl() + "/oauth-service/oauth/preauthorized");
QUrlQuery query;
query.addQueryItem("ticket", ticket);
query.addQueryItem("login-url", ssoUrl() + SSO_EMBED_PATH);  // ✅ ADD THIS
query.addQueryItem("accepts-mfa-tokens", "true");
url.setQuery(query);
```

---

## Fix 3: Fix OAuth2 Exchange POST Parameters

**Location:** `garminconnect.cpp` - `exchangeForOAuth2Token()` function

**Problem:** OAuth1 credentials sent in POST body instead of in OAuth1 signature

**Current Code (WRONG):**
```cpp
QUrl url(connectApiUrl() + "/oauth-service/oauth/exchange/user/2.0");

// Prepare POST data
QUrlQuery postData;
postData.addQueryItem("oauth_token", m_oauth1Token.oauth_token);          // ❌ REMOVE
postData.addQueryItem("oauth_token_secret", m_oauth1Token.oauth_token_secret);  // ❌ REMOVE

QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

QNetworkRequest request(url);
request.setRawHeader("User-Agent", USER_AGENT);
request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
// ❌ NO AUTHORIZATION HEADER

QNetworkReply *reply = m_manager->post(request, data);
```

**Corrected Code:**
```cpp
QUrl url(connectApiUrl() + "/oauth-service/oauth/exchange/user/2.0");

// Prepare POST data - ONLY include mfa_token if it exists
QUrlQuery postData;
if (!m_oauth1Token.mfa_token.isEmpty()) {
    postData.addQueryItem("mfa_token", m_oauth1Token.mfa_token);
}

QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

QNetworkRequest request(url);
request.setRawHeader("User-Agent", USER_AGENT);
request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

// ✅ ADD PROPER OAUTH1 SIGNATURE
// This requires an OAuth1 library or manual HMAC-SHA1 implementation
// See "Fix 4" section for OAuth1 signing implementation
QString authHeader = generateOAuth1AuthorizationHeader(
    "POST",
    url.toString(),
    m_oauth2ConsumerKey,
    m_oauth2ConsumerSecret,
    m_oauth1Token.oauth_token,
    m_oauth1Token.oauth_token_secret,
    data
);
request.setRawHeader("Authorization", authHeader.toUtf8());

QNetworkReply *reply = m_manager->post(request, data);
```

---

## Fix 4: Fix Service Parameter in Login POST

**Location:** `garminconnect.cpp` - `performLogin()` function

**Problem:** Uses connectApiUrl() instead of ssoEmbedUrl for service parameter

**Current Code (WRONG):**
```cpp
QUrl url(ssoUrl() + SSO_URL_PATH);
QUrlQuery query;
query.addQueryItem("id", "gauth-widget");
query.addQueryItem("embedWidget", "true");
query.addQueryItem("gauthHost", ssoEmbedUrl);
query.addQueryItem("service", connectApiUrl());  // ❌ WRONG - should be ssoEmbedUrl
query.addQueryItem("source", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
url.setQuery(query);
```

**Corrected Code:**
```cpp
QUrl url(ssoUrl() + SSO_URL_PATH);
QUrlQuery query;
query.addQueryItem("id", "gauth-widget");
query.addQueryItem("embedWidget", "true");
query.addQueryItem("gauthHost", ssoEmbedUrl);
query.addQueryItem("service", ssoEmbedUrl);  // ✅ FIXED - same as GET request
query.addQueryItem("source", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
url.setQuery(query);
```

---

## Fix 5: Add Query Parameters to MFA Endpoint

**Location:** `garminconnect.cpp` - `performMfaVerification()` function

**Problem:** MFA endpoint missing SIGNIN_PARAMS query string

**Current Code (WRONG):**
```cpp
bool GarminConnect::performMfaVerification(const QString &mfaCode)
{
    qDebug() << "GarminConnect: Performing MFA verification...";

    QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");
    // ❌ NO QUERY PARAMETERS

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
    request.setRawHeader("Referer", url.toString().toUtf8());
    request.setRawHeader("Origin", ssoUrl().toUtf8());

    // Add cookies
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
    }

    QNetworkReply *reply = m_manager->post(request, data);
    // ...
}
```

**Corrected Code:**
```cpp
bool GarminConnect::performMfaVerification(const QString &mfaCode)
{
    qDebug() << "GarminConnect: Performing MFA verification...";

    QString ssoEmbedUrl = ssoUrl() + SSO_EMBED_PATH;

    // ✅ ADD QUERY PARAMETERS (same as signin endpoint)
    QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");
    QUrlQuery query;
    query.addQueryItem("id", "gauth-widget");
    query.addQueryItem("embedWidget", "true");
    query.addQueryItem("gauthHost", ssoEmbedUrl);
    query.addQueryItem("service", ssoEmbedUrl);  // ✅ Use ssoEmbedUrl, not connectApi
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
    request.setRawHeader("Referer", url.toString().toUtf8());
    request.setRawHeader("Origin", ssoUrl().toUtf8());

    // Add cookies
    for (const QNetworkCookie &cookie : m_cookies) {
        m_manager->cookieJar()->insertCookie(cookie);
    }

    QNetworkReply *reply = m_manager->post(request, data);
    // ...
}
```

---

## Fix 6: Remove Content-Type Header from OAuth1 GET Request

**Location:** `garminconnect.cpp` - `exchangeForOAuth1Token()` function

**Problem:** Content-Type header on GET request (GET requests shouldn't have body)

**Current Code:**
```cpp
QNetworkRequest request(url);
request.setRawHeader("User-Agent", USER_AGENT);
request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");  // ❌ Remove this

QString authHeader = QString("OAuth oauth_consumer_key=\"%1\"").arg(consumerKey);
request.setRawHeader("Authorization", authHeader.toUtf8());

QNetworkReply *reply = m_manager->get(request);
```

**Corrected Code:**
```cpp
QNetworkRequest request(url);
request.setRawHeader("User-Agent", USER_AGENT);
// ✅ REMOVED Content-Type header for GET request

// TODO: Add proper OAuth1 signature (see Fix 4 for implementation details)
QString authHeader = generateOAuth1AuthorizationHeader(
    "GET",
    url.toString(),
    m_oauth2ConsumerKey,
    m_oauth2ConsumerSecret,
    "",  // No token yet
    "",  // No token secret yet
    ""   // No body for GET
);
request.setRawHeader("Authorization", authHeader.toUtf8());

QNetworkReply *reply = m_manager->get(request);
```

---

## Fix 7: Add Referer Header to CSRF Token Fetch

**Location:** `garminconnect.cpp` - `fetchCsrfToken()` function

**Problem:** No Referer header on CSRF token fetch

**Current Code:**
```cpp
QUrl url(ssoUrl() + SSO_URL_PATH);
QUrlQuery query;
// ... query items

QNetworkRequest request(url);
request.setRawHeader("User-Agent", USER_AGENT);
// ❌ NO REFERER HEADER

QNetworkReply *reply = m_manager->get(request);
```

**Corrected Code:**
```cpp
QUrl url(ssoUrl() + SSO_URL_PATH);
QUrlQuery query;
// ... query items

QNetworkRequest request(url);
request.setRawHeader("User-Agent", USER_AGENT);

// ✅ ADD REFERER FROM PREVIOUS RESPONSE
// Store the previous URL from /sso/embed response
QString previousUrl = ssoUrl() + SSO_EMBED_PATH;
QUrl prevUrl(previousUrl);
request.setRawHeader("Referer", previousUrl.toUtf8());

QNetworkReply *reply = m_manager->get(request);
```

**Alternative (Better):** Track the previous response URL automatically:
```cpp
// Add to header of class
private:
    QString m_lastRequestUrl;

// In fetchCookies():
    m_lastRequestUrl = url.toString();

// In fetchCsrfToken():
    request.setRawHeader("Referer", m_lastRequestUrl.toUtf8());
    m_lastRequestUrl = url.toString();
```

---

## CRITICAL IMPLEMENTATION: OAuth1 Signature

**This is the most complex fix and requires either:**
- A) Using a C++ OAuth1 library
- B) Implementing HMAC-SHA1 signature manually

### Option A: Using a Library (Recommended)

You could integrate a C++ OAuth library. However, I'm not aware of a simple Qt-compatible OAuth1 library readily available.

### Option B: Manual Implementation

Here's a skeleton for manual HMAC-SHA1 OAuth1 signature implementation:

**Header file additions needed:**
```cpp
private:
    QString m_oauth2ConsumerKey;
    QString m_oauth2ConsumerSecret;

    QString generateOAuth1AuthorizationHeader(
        const QString &httpMethod,
        const QString &url,
        const QString &consumerKey,
        const QString &consumerSecret,
        const QString &oauth_token = "",
        const QString &oauth_token_secret = "",
        const QString &requestBody = ""
    );

    QString generateOAuth1Signature(
        const QString &httpMethod,
        const QString &baseUrl,
        const QString &parameterString,
        const QString &consumerSecret,
        const QString &oauth_token_secret = ""
    );

    QString percentEncode(const QString &str);
    QString generateNonce();
    QString generateTimestamp();
```

**Implementation skeleton (garminconnect.cpp):**
```cpp
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUuid>
#include <QDateTime>

QString GarminConnect::generateOAuth1AuthorizationHeader(
    const QString &httpMethod,
    const QString &url,
    const QString &consumerKey,
    const QString &consumerSecret,
    const QString &oauth_token,
    const QString &oauth_token_secret,
    const QString &requestBody)
{
    // 1. Generate OAuth parameters
    QString nonce = generateNonce();
    QString timestamp = generateTimestamp();

    // 2. Build parameter string
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
    for (auto &&pair : urlQuery.queryItems()) {
        params[pair.first] = pair.second;
    }

    // 3. Create parameter string (sorted by key)
    QString parameterString;
    for (auto it = params.begin(); it != params.end(); ++it) {
        if (it != params.begin()) parameterString += "&";
        parameterString += percentEncode(it.key()) + "=" + percentEncode(it.value());
    }

    // 4. Create base string
    QUrl baseUrl(url);
    baseUrl.setQuery(QUrl::Query); // Remove query from URL
    QString baseUrlStr = baseUrl.scheme() + "://" + baseUrl.host() + baseUrl.path();

    QString baseString = httpMethod + "&" + percentEncode(baseUrlStr) +
                         "&" + percentEncode(parameterString);

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
    authHeader += "oauth_consumer_key=\"" + consumerKey + "\", ";
    authHeader += "oauth_nonce=\"" + nonce + "\", ";
    authHeader += "oauth_signature_method=\"HMAC-SHA1\", ";
    authHeader += "oauth_signature=\"" + percentEncode(signature) + "\", ";
    authHeader += "oauth_timestamp=\"" + timestamp + "\", ";
    authHeader += "oauth_version=\"1.0\"";

    if (!oauth_token.isEmpty()) {
        authHeader += ", oauth_token=\"" + oauth_token + "\"";
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
    QString baseString = httpMethod + "&" + percentEncode(baseUrl) +
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
            encoded += "%" + QString("%1").arg(c.unicode(), 2, 16, QChar('0')).toUpper();
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
    return QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
}
```

**⚠️ IMPORTANT NOTES:**
- The above implementation is a skeleton and may need refinement
- OAuth1 signature generation is complex and error-prone
- I recommend testing with Python's garth library side-by-side to verify correctness
- Consider using a proper OAuth1 library if available

---

## TESTING CHECKLIST AFTER APPLYING FIXES

- [ ] Fix 1: OAuth1 response parsing as URL-encoded
- [ ] Fix 2: Add login-url parameter
- [ ] Fix 3: Fix OAuth2 exchange POST body (only mfa_token)
- [ ] Fix 4: Fix service parameter in login POST
- [ ] Fix 5: Add query parameters to MFA endpoint
- [ ] Fix 6: Remove Content-Type from OAuth1 GET
- [ ] Fix 7: Add Referer header to CSRF fetch
- [ ] **CRITICAL:** Implement OAuth1 signature (Fix 4 in this section)
- [ ] Test login without MFA
- [ ] Test login with MFA
- [ ] Test token refresh
- [ ] Test activity upload
- [ ] Test with garmin.cn domain
- [ ] Compare with Python garth library behavior

---

**Document Generated:** 2025-12-03
**Status:** Ready for implementation
