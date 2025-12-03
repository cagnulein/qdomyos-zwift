# Side-by-Side Code Comparison: C++ vs Python

This document shows the exact differences in code between C++ implementation and Python garth library.

---

## ISSUE 1: OAuth1 Signature Implementation

### Python (requests-oauthlib)

```python
# sso.py lines 152-165
def exchange(oauth1: OAuth1Token, client: "http.Client") -> OAuth2Token:
    sess = GarminOAuth1Session(
        resource_owner_key=oauth1.oauth_token,
        resource_owner_secret=oauth1.oauth_token_secret,
        parent=client.sess,
    )
    data = dict(mfa_token=oauth1.mfa_token) if oauth1.mfa_token else {}
    base_url = f"https://connectapi.{client.domain}/oauth-service/oauth/"
    url = f"{base_url}exchange/user/2.0"
    headers = {
        **USER_AGENT,
        **{"Content-Type": "application/x-www-form-urlencoded"},
    }
    resp = sess.post(
        url,
        headers=headers,
        data=data,  # IMPORTANT: only mfa_token if present, NOT oauth credentials!
        timeout=client.timeout,
    )
    resp.raise_for_status()
    token = resp.json()
    return OAuth2Token(**set_expirations(token))
```

**Key Points:**
- `GarminOAuth1Session` extends `OAuth1Session` from `requests_oauthlib`
- Session is created with `resource_owner_key` and `resource_owner_secret`
- When `sess.post()` is called, `requests_oauthlib` automatically:
  - Generates random nonce
  - Gets current timestamp
  - Calculates HMAC-SHA1 signature using:
    - Consumer Key + Consumer Secret
    - Resource Owner Key + Resource Owner Secret
    - Request URL, method, and parameters
  - Constructs Authorization header with all OAuth1 parameters

**Resulting HTTP Request:**
```
POST /oauth-service/oauth/exchange/user/2.0 HTTP/1.1
Host: connectapi.garmin.com
User-Agent: com.garmin.android.apps.connectmobile
Content-Type: application/x-www-form-urlencoded
Authorization: OAuth oauth_consumer_key="cid_...", oauth_token="...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"

[empty body or: mfa_token=...]
```

### C++ (Current Implementation)

```cpp
// garminconnect.cpp lines 506-551
bool GarminConnect::exchangeForOAuth2Token()
{
    qDebug() << "GarminConnect: Exchanging OAuth1 for OAuth2 token...";

    QUrl url(connectApiUrl() + "/oauth-service/oauth/exchange/user/2.0");

    // Prepare POST data
    QUrlQuery postData;
    postData.addQueryItem("oauth_token", m_oauth1Token.oauth_token);          // ❌ WRONG - shouldn't be in body!
    postData.addQueryItem("oauth_token_secret", m_oauth1Token.oauth_token_secret);  // ❌ WRONG - shouldn't be in body!

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    // ❌ NO AUTHORIZATION HEADER AT ALL!

    QNetworkReply *reply = m_manager->post(request, data);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // ... rest of function
}
```

**Key Problems:**
- OAuth1 token and secret sent in POST body (WRONG - never send credentials in body!)
- No Authorization header (MISSING - this is required!)
- No OAuth1 signature calculation (MISSING)
- No nonce generation (MISSING)
- No timestamp generation (MISSING)

**Resulting HTTP Request:**
```
POST /oauth-service/oauth/exchange/user/2.0 HTTP/1.1
Host: connectapi.garmin.com
User-Agent: com.garmin.android.apps.connectmobile
Content-Type: application/x-www-form-urlencoded

oauth_token=abc123&oauth_token_secret=xyz789
```

**API Response:** ❌ **REJECTED** - Invalid request format

---

## ISSUE 2: OAuth1 Preauthorized Request

### Python

```python
# sso.py lines 143-161
def get_oauth1_token(ticket: str, client: "http.Client") -> OAuth1Token:
    sess = GarminOAuth1Session(parent=client.sess)  # OAuth1 session!
    base_url = f"https://connectapi.{client.domain}/oauth-service/oauth/"
    login_url = f"https://sso.{client.domain}/sso/embed"
    url = (
        f"{base_url}preauthorized?ticket={ticket}&login-url={login_url}"
        "&accepts-mfa-tokens=true"
    )
    resp = sess.get(
        url,
        headers=USER_AGENT,
        timeout=client.timeout,
    )
    resp.raise_for_status()
    parsed = parse_qs(resp.text)  # ⚠️ Parse as URL-encoded, NOT JSON!
    token = {k: v[0] for k, v in parsed.items()}
    return OAuth1Token(domain=client.domain, **token)
```

### C++ (Current Implementation)

```cpp
// garminconnect.cpp lines 429-504
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
    query.addQueryItem("accepts-mfa-tokens", "true");  // ❌ Missing login-url!
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");  // ❌ Not needed for GET

    // OAuth1 requires consumer key/secret in the request
    QString authHeader = QString("OAuth oauth_consumer_key=\"%1\"").arg(consumerKey);  // ❌ Incomplete!
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

    QJsonObject jsonResponse = extractJsonFromResponse(reply);  // ❌ Wrong format - should be URL-encoded!
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
```

**Problems in C++:**
1. ❌ Missing `login-url` parameter in query string
2. ❌ Authorization header only includes `oauth_consumer_key`, missing:
   - `oauth_signature_method`
   - `oauth_signature` (the actual HMAC-SHA1 signature!)
   - `oauth_nonce`
   - `oauth_timestamp`
   - `oauth_version`
3. ❌ Response parsed as JSON instead of URL-encoded form data
4. ❌ Content-Type header on GET request (not harmful but unnecessary)

---

## ISSUE 3: Response Parsing Format

### Python (URL-Encoded Response)

```python
# sso.py line 156
from urllib.parse import parse_qs

parsed = parse_qs(resp.text)  # Parse URL-encoded: key1=value1&key2=value2
token = {k: v[0] for k, v in parsed.items()}
```

**Expected Response Format from `/oauth-service/oauth/preauthorized`:**
```
oauth_token=abc123def456&oauth_token_secret=xyz789&mfa_token=mfa123&mfa_expiration_timestamp=1234567890
```

### C++ (Incorrectly Parsing as JSON)

```cpp
// garminconnect.cpp lines 489-495
QJsonObject jsonResponse = extractJsonFromResponse(reply);

m_oauth1Token.oauth_token = jsonResponse["oauth_token"].toString();
m_oauth1Token.oauth_token_secret = jsonResponse["oauth_token_secret"].toString();
m_oauth1Token.mfa_token = jsonResponse["mfa_token"].toString();
m_oauth1Token.mfa_expiration_timestamp = jsonResponse["mfa_expiration_timestamp"].toVariant().toLongLong();
```

**Problem:** If the response is:
```
oauth_token=abc123&oauth_token_secret=xyz789
```

Trying to parse this as JSON will fail, and all values will be empty strings!

**Correct C++ Implementation:**
```cpp
// Parse as URL-encoded form data
QString responseText = QString::fromUtf8(reply->readAll());
QUrlQuery responseQuery(responseText);

m_oauth1Token.oauth_token = responseQuery.queryItemValue("oauth_token");
m_oauth1Token.oauth_token_secret = responseQuery.queryItemValue("oauth_token_secret");
m_oauth1Token.mfa_token = responseQuery.queryItemValue("mfa_token");

bool ok;
m_oauth1Token.mfa_expiration_timestamp = responseQuery.queryItemValue("mfa_expiration_timestamp").toLongLong(&ok);
```

---

## ISSUE 4: Service Parameter in Login POST

### Python

```python
# sso.py lines 71-81
SIGNIN_PARAMS = {
    **SSO_EMBED_PARAMS,
    **dict(
        gauthHost=SSO_EMBED,  # https://sso.{domain}/sso/embed
        service=SSO_EMBED,    # ✓ CORRECT - uses SSO embed URL
        source=SSO_EMBED,
        redirectAfterAccountLoginUrl=SSO_EMBED,
        redirectAfterAccountCreationUrl=SSO_EMBED,
    ),
}

# Used for both GET and POST
client.get("sso", "/sso/signin", params=SIGNIN_PARAMS, referrer=True)
client.post("sso", "/sso/signin", params=SIGNIN_PARAMS, referrer=True, data={...})
```

### C++ (GET vs POST - Inconsistent)

```cpp
// garminconnect.cpp lines 115-175 (GET for CSRF token)
bool GarminConnect::fetchCsrfToken()
{
    QString ssoEmbedUrl = ssoUrl() + SSO_EMBED_PATH;

    QUrl url(ssoUrl() + SSO_URL_PATH);
    QUrlQuery query;
    query.addQueryItem("id", "gauth-widget");
    query.addQueryItem("embedWidget", "true");
    query.addQueryItem("gauthHost", ssoEmbedUrl);
    query.addQueryItem("service", ssoEmbedUrl);  // ✓ CORRECT for GET
    query.addQueryItem("source", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);

    QNetworkReply *reply = m_manager->get(request);
    // ...
}

// garminconnect.cpp lines 185-222 (POST for login)
bool GarminConnect::performLogin(const QString &email, const QString &password)
{
    QString ssoEmbedUrl = ssoUrl() + SSO_EMBED_PATH;

    QUrl url(ssoUrl() + SSO_URL_PATH);
    QUrlQuery query;
    query.addQueryItem("id", "gauth-widget");
    query.addQueryItem("embedWidget", "true");
    query.addQueryItem("gauthHost", ssoEmbedUrl);
    query.addQueryItem("service", connectApiUrl());  // ❌ WRONG - uses connectAPI URL!
    query.addQueryItem("source", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
    query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
    url.setQuery(query);

    QNetworkRequest request(url);
    // ... POST data and headers

    QNetworkReply *reply = m_manager->post(request, data);
    // ...
}
```

**Issue:**
- **GET (fetchCsrfToken):** `service=https://sso.{domain}/sso/embed` ✓
- **POST (performLogin):** `service=https://connectapi.{domain}` ❌

**Fix:** Line 196 should use `ssoEmbedUrl` instead of `connectApiUrl()`:
```cpp
query.addQueryItem("service", ssoEmbedUrl);  // Same as GET
```

---

## ISSUE 5: Missing Query Parameters on MFA Endpoint

### Python

```python
# sso.py lines 168-181
def handle_mfa(
    client: "http.Client", signin_params: dict, prompt_mfa: Callable
) -> None:
    csrf_token = get_csrf_token(client.last_resp.text)
    if asyncio.iscoroutinefunction(prompt_mfa):
        mfa_code = asyncio.run(prompt_mfa())
    else:
        mfa_code = prompt_mfa()
    client.post(
        "sso",
        "/sso/verifyMFA/loginEnterMfaCode",
        params=signin_params,  # ✓ Query parameters included!
        referrer=True,
        data={
            "mfa-code": mfa_code,
            "embed": "true",
            "_csrf": csrf_token,
            "fromPage": "setupEnterMfaCode",
        },
    )
```

### C++ (Missing Query Parameters)

```cpp
// garminconnect.cpp lines 357-427
bool GarminConnect::performMfaVerification(const QString &mfaCode)
{
    qDebug() << "GarminConnect: Performing MFA verification...";

    QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");
    // ❌ NO QUERY PARAMETERS ADDED!

    // Prepare POST data
    QUrlQuery postData;
    postData.addQueryItem("mfa-code", mfaCode);
    postData.addQueryItem("embed", "true");
    postData.addQueryItem("fromPage", "setupEnterMfaCode");
    postData.addQueryItem("_csrf", m_csrfToken);

    QByteArray data = postData.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(url);
    // ... headers

    QNetworkReply *reply = m_manager->post(request, data);
    // ...
}
```

**Issue:** The URL should include SIGNIN_PARAMS query parameters (same as the login POST):
```
/sso/verifyMFA/loginEnterMfaCode?id=gauth-widget&embedWidget=true&gauthHost=...&service=...&source=...&redirectAfterAccountLoginUrl=...&redirectAfterAccountCreationUrl=...
```

**Fix:** Add query parameters before the POST:
```cpp
QString ssoEmbedUrl = ssoUrl() + SSO_EMBED_PATH;

QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");
QUrlQuery query;
query.addQueryItem("id", "gauth-widget");
query.addQueryItem("embedWidget", "true");
query.addQueryItem("gauthHost", ssoEmbedUrl);
query.addQueryItem("service", ssoEmbedUrl);  // Use ssoEmbedUrl
query.addQueryItem("source", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
url.setQuery(query);
```

---

## ISSUE 6: Login-URL Parameter Missing

### Python (Correct)

```python
# sso.py lines 143-150
def get_oauth1_token(ticket: str, client: "http.Client") -> OAuth1Token:
    sess = GarminOAuth1Session(parent=client.sess)
    base_url = f"https://connectapi.{client.domain}/oauth-service/oauth/"
    login_url = f"https://sso.{client.domain}/sso/embed"
    url = (
        f"{base_url}preauthorized?ticket={ticket}&login-url={login_url}"
        "&accepts-mfa-tokens=true"
    )
```

**Complete URL:**
```
https://connectapi.garmin.com/oauth-service/oauth/preauthorized?ticket=abc123&login-url=https://sso.garmin.com/sso/embed&accepts-mfa-tokens=true
```

### C++ (Missing login-url)

```cpp
// garminconnect.cpp lines 463-467
QUrl url(connectApiUrl() + "/oauth-service/oauth/preauthorized");
QUrlQuery query;
query.addQueryItem("ticket", ticket);
query.addQueryItem("accepts-mfa-tokens", "true");
url.setQuery(query);
```

**Current URL (WRONG):**
```
https://connectapi.garmin.com/oauth-service/oauth/preauthorized?ticket=abc123&accepts-mfa-tokens=true
```

**Fix:** Add this line after accepts-mfa-tokens:
```cpp
query.addQueryItem("login-url", ssoUrl() + SSO_EMBED_PATH);
```

---

## SUMMARY OF FIXES

| Issue | File | Line | Fix Type | Severity |
|-------|------|------|----------|----------|
| OAuth1 signature | garminconnect.cpp | 469-475, 514-521 | Implement HMAC-SHA1 | CRITICAL |
| Wrong POST body | garminconnect.cpp | 514-515 | Remove 2 lines | CRITICAL |
| Response parsing | garminconnect.cpp | 489-495 | Change JSON to URL-encoded | CRITICAL |
| Missing login-url | garminconnect.cpp | 465-466 | Add 1 line | CRITICAL |
| Wrong service param | garminconnect.cpp | 196 | Change 1 parameter value | CRITICAL |
| MFA missing params | garminconnect.cpp | 361-368 | Add query parameters | HIGH |
| Content-Type GET | garminconnect.cpp | 471 | Remove 1 line | MEDIUM |
| Missing Referer | garminconnect.cpp | 133 | Add 1 header | MEDIUM |

---

**Document Generated:** 2025-12-03
**Purpose:** Exact code-level comparison of differences
