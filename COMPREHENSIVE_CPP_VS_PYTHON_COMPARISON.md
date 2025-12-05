# Comprehensive C++ vs Python Garth Library Comparison

**Status:** Detailed line-by-line analysis of garminconnect.cpp vs garth Python library
**Date:** 2025-12-03
**Files Compared:**
- C++ Implementation: `/home/user/qdomyos-zwift/src/garminconnect.cpp`
- Python Library: `https://github.com/matin/garth/blob/main/src/garth/sso.py`

---

## 1. HTTP HEADERS ANALYSIS

### 1.1 User-Agent Header

| Request Type | C++ Code | Python Code | Status |
|--------------|----------|-------------|--------|
| All SSO requests (embed, signin, MFA) | `com.garmin.android.apps.connectmobile` | `com.garmin.android.apps.connectmobile` | ✅ CORRECT |
| OAuth1 preauthorized GET | `com.garmin.android.apps.connectmobile` | `com.garmin.android.apps.connectmobile` | ✅ CORRECT |
| OAuth2 exchange POST | `com.garmin.android.apps.connectmobile` | `com.garmin.android.apps.connectmobile` | ✅ CORRECT |
| Token refresh POST | `com.garmin.android.apps.connectmobile` | (implied same) | ✅ CORRECT |
| Activity upload POST | `GCM-iOS-5.7.2.1` | (not in garth) | ⚠️ DIFFERENT - Uses iOS UA instead of standard |

**Issue Found:** Line 639 uses iOS user-agent for upload instead of standard Android user-agent. This may be intentional (upload endpoint may require iOS UA) or may be incorrect.

---

### 1.2 Content-Type Header

| Endpoint | C++ Code | Python Code | Status | Location |
|----------|----------|-------------|--------|----------|
| /sso/embed GET | Not set | Not set | ✅ CORRECT | Line 94 |
| /sso/signin GET (CSRF fetch) | Not set | Not set | ✅ CORRECT | Line 133 |
| /sso/signin POST (login) | `application/x-www-form-urlencoded` | `application/x-www-form-urlencoded` | ✅ CORRECT | Line 213 |
| /sso/verifyMFA POST | `application/x-www-form-urlencoded` | `application/x-www-form-urlencoded` | ✅ CORRECT | Line 374 |
| /oauth-service/oauth/preauthorized GET | `application/x-www-form-urlencoded` | Not set | ❌ INCORRECT | Line 471 |
| /oauth-service/oauth/exchange/user/2.0 POST | `application/x-www-form-urlencoded` | `application/x-www-form-urlencoded` | ✅ CORRECT | Line 521 |
| /oauth-service/oauth/exchange/user/2.0 POST (refresh) | `application/x-www-form-urlencoded` | (implied same) | ✅ CORRECT | Line 574 |

**Issue Found:** Line 471 sets Content-Type on a GET request (OAuth1 preauthorized). GET requests should not have Content-Type headers. This is harmless but unnecessary.

---

### 1.3 Authorization Header

#### OAuth1 Preauthorized (Line 474-475)
**C++ Code:**
```cpp
QString authHeader = QString("OAuth oauth_consumer_key=\"%1\"").arg(consumerKey);
request.setRawHeader("Authorization", authHeader.toUtf8());
```

**Expected (Python via requests-oauthlib):**
```
Authorization: OAuth oauth_consumer_key="...", oauth_token="...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"
```

**Status:** ❌ INCOMPLETE AND INCORRECT
- C++ only includes `oauth_consumer_key`
- Missing: `oauth_token`, `oauth_signature`, `oauth_nonce`, `oauth_timestamp`, `oauth_signature_method`, `oauth_version`
- This is for a GET request, but OAuth1 signature IS needed for this endpoint

**Severity:** CRITICAL - The OAuth1 preauthorized endpoint may fail if it validates the signature.

---

#### OAuth2 Exchange (Line 520-521)
**C++ Code:** Only sets User-Agent and Content-Type, no Authorization header

**Expected:** OAuth1 signed request (handled by requests_oauthlib in Python)

**Status:** ❌ CRITICAL - Missing OAuth1 signature

---

#### Token Refresh (Line 573-574)
**C++ Code:** Only sets User-Agent and Content-Type, no Authorization header

**Expected:** No authorization header (POST with oauth_token and oauth_token_secret in body for refresh_token flow)

**Status:** ✅ CORRECT for this endpoint

---

#### Activity Upload (Line 640)
**C++ Code:**
```cpp
request.setRawHeader("Authorization", QString("Bearer %1").arg(m_oauth2Token.access_token).toUtf8());
```

**Expected:** Bearer token format for OAuth2

**Status:** ✅ CORRECT

---

### 1.4 Referer Header

| Endpoint | C++ Code | Python Code | Status | Line |
|----------|----------|-------------|--------|------|
| /sso/embed GET | Not set | Not set (first request) | ✅ CORRECT | 94 |
| /sso/signin GET (CSRF fetch) | Not set | Uses referrer=True | ⚠️ DIFFERS | 133 |
| /sso/signin POST (login) | `url.toString()` | Uses referrer=True (from previous GET) | ✅ LIKELY CORRECT | 214 |
| /sso/verifyMFA POST | `url.toString()` | Uses referrer=True | ✅ LIKELY CORRECT | 375 |
| OAuth1 preauthorized GET | Not set | Not set | ✅ CORRECT | 436 |
| OAuth2 exchange POST | Not set | Not set | ✅ CORRECT | 520 |

**Issue Found (Minor):** Line 133 doesn't set Referer header for the CSRF token fetch GET request, but Python sets `referrer=True` which includes the previous response's referrer. This might matter for security checks.

---

### 1.5 Origin Header

| Endpoint | C++ Code | Python Code | Status | Line |
|----------|----------|-------------|--------|------|
| /sso/embed GET | Not set | Not set | ✅ CORRECT | 94 |
| /sso/signin GET (CSRF fetch) | Not set | Not set | ✅ CORRECT | 133 |
| /sso/signin POST (login) | `ssoUrl()` | Not set | ⚠️ DIFFERS | 215 |
| /sso/verifyMFA POST | `ssoUrl()` | Not set | ⚠️ DIFFERS | 376 |

**Issue Found (Minor):** C++ adds Origin header to POST requests, but Python doesn't. Python's `requests` library might automatically add it. The C++ implementation may be more explicit, which is generally fine. However, if Garmin's API strictly checks for specific Origin values, this could matter.

---

## 2. QUERY PARAMETERS ANALYSIS

### 2.1 /sso/embed (First GET - Cookie Setting)

**C++ Code (Lines 88-91):**
```cpp
query.addQueryItem("id", "gauth-widget");
query.addQueryItem("embedWidget", "true");
query.addQueryItem("gauthHost", ssoUrl() + "/sso");
```

**Python Code:**
```python
SSO_EMBED_PARAMS = dict(
    id="gauth-widget",
    embedWidget="true",
    gauthHost=SSO,  # https://sso.{domain}/sso
)
```

**Status:** ✅ CORRECT

**Note:** C++ uses `ssoUrl() + "/sso"` which is `https://sso.{domain}/sso`, matching Python's SSO value.

---

### 2.2 /sso/signin (GET for CSRF Token)

**C++ Code (Lines 123-130):**
```cpp
query.addQueryItem("id", "gauth-widget");
query.addQueryItem("embedWidget", "true");
query.addQueryItem("gauthHost", ssoEmbedUrl);  // https://sso.{domain}/sso/embed
query.addQueryItem("service", ssoEmbedUrl);
query.addQueryItem("source", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
```

**Python Code:**
```python
SIGNIN_PARAMS = {
    **SSO_EMBED_PARAMS,
    **dict(
        gauthHost=SSO_EMBED,
        service=SSO_EMBED,
        source=SSO_EMBED,
        redirectAfterAccountLoginUrl=SSO_EMBED,
        redirectAfterAccountCreationUrl=SSO_EMBED,
    ),
}
```

**Status:** ✅ CORRECT - All parameters match exactly.

---

### 2.3 /sso/signin (POST for Login Credentials)

**C++ Code (Lines 193-200):**
```cpp
query.addQueryItem("id", "gauth-widget");
query.addQueryItem("embedWidget", "true");
query.addQueryItem("gauthHost", ssoEmbedUrl);
query.addQueryItem("service", connectApiUrl());  // USES CONNECTAPI!
query.addQueryItem("source", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountLoginUrl", ssoEmbedUrl);
query.addQueryItem("redirectAfterAccountCreationUrl", ssoEmbedUrl);
```

**Python Code:**
```python
# Uses the SAME SIGNIN_PARAMS for both GET and POST
client.post(
    "sso",
    "/sso/signin",
    params=SIGNIN_PARAMS,  # service=SSO_EMBED, not connectAPI!
    ...
)
```

**Status:** ❌ INCORRECT - Service parameter differs

**Issue Details:**
- **C++ Line 196:** Uses `connectApiUrl()` which is `https://connectapi.{domain}`
- **Python:** Uses `SSO_EMBED` which is `https://sso.{domain}/sso/embed`
- **Analysis Document:** Contradicts itself, but says POST should use connectApiUrl in section 3
- **Actual Python Code:** Uses SSO_EMBED consistently for both GET and POST

**Decision:** The Python code is the source of truth. The service parameter should be `ssoEmbedUrl`, not `connectApiUrl()`.

**Severity:** HIGH - This parameter affects the authentication handshake.

---

### 2.4 /sso/verifyMFA/loginEnterMfaCode (POST for MFA)

**C++ Code (Lines 365-368):**
```cpp
postData.addQueryItem("mfa-code", mfaCode);
postData.addQueryItem("embed", "true");
postData.addQueryItem("fromPage", "setupEnterMfaCode");
postData.addQueryItem("_csrf", m_csrfToken);
```

**Python Code:**
```python
client.post(
    "sso",
    "/sso/verifyMFA/loginEnterMfaCode",
    params=signin_params,  # Same query params as signin
    referrer=True,
    data={
        "mfa-code": mfa_code,
        "embed": "true",
        "_csrf": csrf_token,
        "fromPage": "setupEnterMfaCode",
    },
)
```

**Status:** ✅ CORRECT - POST body parameters match.

**Note:** The MFA endpoint also needs SIGNIN_PARAMS as query parameters, which the C++ code doesn't show. Let me check...

Looking at the C++ code, the MFA endpoint is called from `performMfaVerification()` at line 361:
```cpp
QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");
```

**Issue Found:** The URL doesn't include SIGNIN_PARAMS! Python includes `params=signin_params` for the MFA endpoint.

**Severity:** MEDIUM - The MFA endpoint may not work correctly without the query parameters.

---

### 2.5 /oauth-service/oauth/preauthorized (GET for OAuth1 Token)

**C++ Code (Lines 463-467):**
```cpp
QUrl url(connectApiUrl() + "/oauth-service/oauth/preauthorized");
QUrlQuery query;
query.addQueryItem("ticket", ticket);
query.addQueryItem("accepts-mfa-tokens", "true");
url.setQuery(query);
```

**Python Code:**
```python
login_url = f"https://sso.{client.domain}/sso/embed"
url = (
    f"{base_url}preauthorized?ticket={ticket}&login-url={login_url}"
    "&accepts-mfa-tokens=true"
)
```

**Status:** ❌ MISSING PARAMETER - Missing `login-url`

**Issue Details:**
- **C++ Code:** Only includes `ticket` and `accepts-mfa-tokens`
- **Python Code:** Includes `login-url` parameter set to `https://sso.{domain}/sso/embed`
- **Analysis Document:** Says this parameter is "not in the Python garth library" but it clearly IS in the code shown above!

**Decision:** The Python code is correct. The `login-url` parameter SHOULD be included.

**Severity:** HIGH - This parameter may be required by Garmin's API.

---

### 2.6 /oauth-service/oauth/exchange/user/2.0 (POST for OAuth2 Token)

**C++ Code (Lines 513-515):**
```cpp
QUrlQuery postData;
postData.addQueryItem("oauth_token", m_oauth1Token.oauth_token);
postData.addQueryItem("oauth_token_secret", m_oauth1Token.oauth_token_secret);
```

**Python Code:**
```python
data = dict(mfa_token=oauth1.mfa_token) if oauth1.mfa_token else {}
resp = sess.post(
    url,
    headers=headers,
    data=data,  # Only mfa_token if present, NOT oauth_token!
    timeout=client.timeout,
)
```

**Status:** ❌ CRITICALLY INCORRECT

**Issue Details:**
- **C++ Code:** Sends `oauth_token` and `oauth_token_secret` in POST body
- **Python Code:** Sends ONLY `mfa_token` IF it exists, otherwise empty body
- **Correct Implementation:** The OAuth1 credentials should be in the OAuth1 signature (Authorization header), NOT in the request body!

**Severity:** CRITICAL - The entire OAuth2 exchange endpoint is wrong.

---

### 2.7 /oauth-service/oauth/exchange/user/2.0 (POST for Token Refresh)

**C++ Code (Lines 566-568):**
```cpp
QUrlQuery postData;
postData.addQueryItem("refresh_token", m_oauth2Token.refresh_token);
postData.addQueryItem("grant_type", "refresh_token");
```

**Python Code:** (Not shown in garth, but this is standard OAuth2)

**Status:** ✅ CORRECT - This is standard OAuth2 refresh token flow.

---

## 3. POST DATA BODY PARAMETERS

### 3.1 /sso/signin POST (Login)

**C++ Code (Lines 204-207):**
```cpp
postData.addQueryItem("username", email);
postData.addQueryItem("password", password);
postData.addQueryItem("embed", "true");
postData.addQueryItem("_csrf", m_csrfToken);
```

**Python Code:**
```python
data=dict(
    username=email,
    password=password,
    embed="true",
    _csrf=csrf_token,
)
```

**Status:** ✅ CORRECT

---

### 3.2 /sso/verifyMFA/loginEnterMfaCode POST (MFA)

**C++ Code (Lines 365-368):**
```cpp
postData.addQueryItem("mfa-code", mfaCode);
postData.addQueryItem("embed", "true");
postData.addQueryItem("fromPage", "setupEnterMfaCode");
postData.addQueryItem("_csrf", m_csrfToken);
```

**Python Code:**
```python
data={
    "mfa-code": mfa_code,
    "embed": "true",
    "_csrf": csrf_token,
    "fromPage": "setupEnterMfaCode",
}
```

**Status:** ✅ CORRECT

---

## 4. COOKIE HANDLING

### 4.1 Cookie Jar Management

**C++ Implementation:**
```cpp
m_cookies = m_manager->cookieJar()->cookiesForUrl(url);  // Line 108
for (const QNetworkCookie &cookie : m_cookies) {
    m_manager->cookieJar()->insertCookie(cookie);  // Manually insert
}
```

**Python Implementation:**
```python
# requests.Session automatically handles cookies
client.get("sso", "/sso/embed", ...)  # Sets cookies automatically
```

**Status:** ⚠️ POTENTIALLY PROBLEMATIC

**Issue Details:**
- **C++ Code:** Manually manages cookies by extracting from jar, then re-inserting them
- **Python Code:** Uses requests.Session which automatically manages cookies transparently
- **Potential Issue:** Manual cookie management might not preserve all cookie attributes (domain, path, secure, httponly flags)

**Severity:** MEDIUM - Cookies might not be sent to subsequent requests if attributes are lost.

---

## 5. REQUEST ORDERING AND DEPENDENCIES

**C++ Implementation:**
```
1. fetchCookies() - GET /sso/embed
2. fetchCsrfToken() - GET /sso/signin
3. performLogin() - POST /sso/signin
4. (IF MFA) performMfaVerification() - POST /sso/verifyMFA/loginEnterMfaCode
5. exchangeForOAuth1Token() - GET /oauth-service/oauth/preauthorized
6. exchangeForOAuth2Token() - POST /oauth-service/oauth/exchange/user/2.0
```

**Python Implementation:**
```python
1. client.get("sso", "/sso/embed", ...) - Sets cookies
2. client.get("sso", "/sso/signin", ...) - Gets CSRF token
3. client.post("sso", "/sso/signin", ...) - Submit login
4. (IF MFA) handle_mfa(...) - Submits MFA code
5. get_oauth1_token(...) - GET preauthorized with OAuth1 session
6. exchange(...) - POST exchange with OAuth1 session
```

**Status:** ✅ CORRECT - Same ordering

---

## 6. RESPONSE PARSING

### 6.1 CSRF Token Extraction

**C++ Code (Lines 157-176):**
```cpp
QRegularExpression csrfRegex1("name=\"_csrf\"[^>]*value=\"([^\"]+)\"");
QRegularExpression csrfRegex2("value=\"([^\"]+)\"[^>]*name=\"_csrf\"");
QRegularExpression csrfRegex3("<input[^>]*name=\"csrf\"[^>]*value=\"([^\"]+)\"");
// Tries multiple patterns
```

**Python Code:**
```python
CSRF_RE = re.compile(r'name="_csrf"\s+value="(.+?)"')
def get_csrf_token(html: str) -> str:
    m = CSRF_RE.search(html)
    if not m:
        raise GarthException("Couldn't find CSRF token")
    return m.group(1)
```

**Status:** ✅ BETTER IN C++ - C++ handles more variations while Python only uses one pattern.

---

### 6.2 Page Title Extraction (Success Detection)

**C++ Code (Lines 241-249):**
```cpp
QRegularExpression titleRegex("<title>(.+?)</title>");
QRegularExpressionMatch titleMatch = titleRegex.match(response);
if (titleMatch.hasMatch()) {
    QString title = titleMatch.captured(1);
    if (title == "Success") {
        qDebug() << "GarminConnect: Login successful";
    }
}
```

**Python Code:**
```python
title = get_title(client.last_resp.text)
if title != "Success":
    raise GarthException(f"Unexpected title: {title}")
```

**Status:** ✅ CORRECT - Both extract and check for "Success" title.

---

### 6.3 Ticket Extraction

**C++ Code (Lines 317-330):**
```cpp
QRegularExpression ticketRegex("embed\\?ticket=([^\"]+)\"");
QRegularExpressionMatch match = ticketRegex.match(response);
// Also has fallback patterns
```

**Python Code:**
```python
m = re.search(r'embed\?ticket=([^"]+)"', client.last_resp.text)
if not m:
    raise GarthException("Couldn't find ticket in response")
ticket = m.group(1)
```

**Status:** ✅ CORRECT - Both use same primary pattern. C++ has additional fallback patterns which is safer.

---

### 6.4 OAuth1 Response Parsing

**C++ Code (Lines 450-451, 489):**
```cpp
QJsonDocument consumerDoc = QJsonDocument::fromJson(consumerReply->readAll());
// ... Extract oauth_token, oauth_token_secret, mfa_token from JSON
```

**Python Code:**
```python
OAUTH_CONSUMER = requests.get(OAUTH_CONSUMER_URL).json()
# ... Parse using parse_qs (URL query string format, not JSON!)
parsed = parse_qs(resp.text)
```

**Status:** ⚠️ POTENTIALLY INCORRECT

**Issue Details:**
- **C++ Code:** Assumes OAuth1 response from preauthorized endpoint is JSON
- **Python Code:** Uses `parse_qs()` which parses URL-encoded query string format

**Potential Issue:** If the response is URL-encoded (`oauth_token=...&oauth_token_secret=...`) instead of JSON, the C++ code will fail to parse it.

**Severity:** CRITICAL - The response format mismatch could cause the entire OAuth1 exchange to fail.

---

## 7. ERROR HANDLING

### 7.1 HTTP Error Handling

**C++ Code Pattern:**
```cpp
if (reply->error() != QNetworkReply::NoError) {
    m_lastError = "Error message: " + reply->errorString();
    return false;
}
```

**Python Code Pattern:**
```python
resp.raise_for_status()  # Raises exception on HTTP errors
```

**Status:** ✅ BOTH HANDLE ERRORS - Different approaches but both valid.

---

### 7.2 MFA Detection

**C++ Code (Lines 284-305):**
```cpp
if (response.contains("MFA", Qt::CaseInsensitive) ||
    response.contains("Enter MFA Code", Qt::CaseInsensitive)) {
    m_lastError = "MFA Required";
    // Extract new CSRF token
}
```

**Python Code:**
```python
title = get_title(client.last_resp.text)
if "MFA" in title:
    if return_on_mfa or prompt_mfa is None:
        return "needs_mfa", {...}
```

**Status:** ✅ CORRECT - Both detect MFA requirement.

---

## 8. OAUTH1 SIGNING - CRITICAL ISSUE

This is the most serious issue in the implementation.

### 8.1 What Python Does

```python
class GarminOAuth1Session(OAuth1Session):
    def __init__(self, /, parent: Session | None = None, **kwargs):
        global OAUTH_CONSUMER
        if not OAUTH_CONSUMER:
            OAUTH_CONSUMER = requests.get(OAUTH_CONSUMER_URL).json()
        super().__init__(
            OAUTH_CONSUMER["consumer_key"],
            OAUTH_CONSUMER["consumer_secret"],
            **kwargs,
        )
```

When calling:
```python
sess = GarminOAuth1Session(
    resource_owner_key=oauth1.oauth_token,
    resource_owner_secret=oauth1.oauth_token_secret,
)
resp = sess.post(url, headers=headers, data=data)
```

The `requests_oauthlib.OAuth1Session` **automatically calculates and signs the request** using:
1. Consumer Key + Consumer Secret (from oauth_consumer.json)
2. Resource Owner Key (oauth_token) + Resource Owner Secret (oauth_token_secret)
3. HMAC-SHA1 signature algorithm
4. Includes OAuth1 parameters: oauth_nonce, oauth_timestamp, oauth_signature_method, oauth_version

The Authorization header becomes:
```
Authorization: OAuth oauth_consumer_key="...", oauth_token="...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"
```

### 8.2 What C++ Does

**For OAuth1 preauthorized (GET):**
```cpp
QString authHeader = QString("OAuth oauth_consumer_key=\"%1\"").arg(consumerKey);
request.setRawHeader("Authorization", authHeader.toUtf8());
```

**Result:** Only includes oauth_consumer_key, missing all other required parameters.

**For OAuth2 exchange (POST):**
```cpp
QUrlQuery postData;
postData.addQueryItem("oauth_token", m_oauth1Token.oauth_token);
postData.addQueryItem("oauth_token_secret", m_oauth1Token.oauth_token_secret);
```

**Result:** Puts credentials in request body instead of signing the request.

### 8.3 The Problem

The C++ implementation:
1. ❌ Does NOT calculate OAuth1 HMAC-SHA1 signature
2. ❌ Does NOT generate random nonce
3. ❌ Does NOT generate timestamp
4. ❌ Does NOT include oauth_signature in Authorization header
5. ❌ Sends credentials in POST body for exchange endpoint instead of using them for signing

**Expected Authorization Headers:**
```
GET /oauth-service/oauth/preauthorized?ticket=...&login-url=...&accepts-mfa-tokens=true
Authorization: OAuth oauth_consumer_key="cid_...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"

POST /oauth-service/oauth/exchange/user/2.0 [with mfa_token in body IF present]
Authorization: OAuth oauth_consumer_key="cid_...", oauth_token="...", oauth_token_secret="...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"
```

**Actual C++ Authorization Headers:**
```
GET /oauth-service/oauth/preauthorized?ticket=...&accepts-mfa-tokens=true  [MISSING login-url]
Authorization: OAuth oauth_consumer_key="..."  [INCOMPLETE]

POST /oauth-service/oauth/exchange/user/2.0
[No Authorization header - sends oauth_token and oauth_token_secret in body instead]
```

**Severity:** CRITICAL - The entire OAuth flow will fail if Garmin's API validates the OAuth1 signature.

---

## 9. DETAILED ISSUE SUMMARY

### ✅ CORRECTLY IMPLEMENTED

1. **User-Agent for SSO/OAuth requests** - Correct Android user-agent used
2. **Content-Type for POST requests** - Correct application/x-www-form-urlencoded
3. **CSRF token extraction** - Multiple patterns with good fallback handling
4. **Ticket extraction** - Multiple patterns with good fallback handling
5. **MFA detection** - Correctly identifies when MFA is required
6. **Token expiration calculation** - Correctly calculates expires_at and refresh_token_expires_at
7. **Cookie jar setup** - Uses Qt's built-in cookie management
8. **Request ordering** - Correct sequence of authentication steps
9. **POST parameters for login/MFA** - All required parameters present and correct
10. **OAuth2 refresh token flow** - Correct parameters for standard OAuth2 refresh

---

### ❌ MISSING OR INCORRECT

#### CRITICAL Issues:

1. **OAuth1 Signature Not Implemented** (Lines 469-475, 514-521)
   - **C++ sends:** Incomplete Authorization header with only oauth_consumer_key, or no header at all
   - **Should send:** Full OAuth1 HMAC-SHA1 signed Authorization header
   - **Impact:** OAuth1 token exchange will likely fail
   - **Fix Required:** Implement or use an OAuth1 library to calculate signatures

2. **Wrong POST Parameters for OAuth2 Exchange** (Lines 514-515)
   - **C++ sends:** oauth_token, oauth_token_secret in POST body
   - **Should send:** Only mfa_token IF present (credentials go in OAuth1 signature)
   - **Impact:** OAuth2 exchange will fail
   - **Fix Required:** Remove these parameters, use them only for OAuth1 signature

3. **Missing login-url Parameter in OAuth1 Request** (Line 465-466)
   - **C++ sends:** Only ticket and accepts-mfa-tokens
   - **Should send:** Also include login-url=https://sso.{domain}/sso/embed
   - **Impact:** OAuth1 exchange may fail if parameter is validated
   - **Fix Required:** Add login-url query parameter

4. **Wrong Service Parameter in Login POST** (Line 196)
   - **C++ sends:** service=https://connectapi.{domain}
   - **Should send:** service=https://sso.{domain}/sso/embed
   - **Impact:** May cause login form submission to fail or redirect incorrectly
   - **Fix Required:** Change connectApiUrl() to ssoEmbedUrl

5. **OAuth1 Response Format Mismatch** (Line 450)
   - **C++ assumes:** JSON format from oauth_consumer.json endpoint (correct)
   - **But OAuth1 preauthorized response:** Likely URL-encoded format, not JSON
   - **Should use:** parse_qs() or equivalent URL query string parser
   - **Impact:** OAuth1 token parsing will fail
   - **Fix Required:** Parse response as URL-encoded, not JSON

#### MEDIUM Issues:

6. **MFA Endpoint Missing Query Parameters** (Line 361)
   - **C++ sends:** Only POST body, no SIGNIN_PARAMS query string
   - **Should send:** Include SIGNIN_PARAMS as query string
   - **Impact:** MFA verification may fail
   - **Fix Required:** Add query parameters to MFA endpoint URL

7. **Unnecessary Content-Type on GET Request** (Line 471)
   - **C++ sends:** Content-Type header on GET request
   - **Should be:** No Content-Type header for GET
   - **Impact:** Likely none, but non-standard
   - **Fix Required:** Remove Content-Type from OAuth1 preauthorized GET

8. **Incomplete Referer for CSRF Fetch** (Line 133)
   - **C++ sends:** No Referer header for /sso/signin GET
   - **Should send:** Referer from previous /sso/embed response
   - **Impact:** May fail if Garmin checks Referer header
   - **Fix Required:** Add Referer header

#### MINOR Issues:

9. **Wrong User-Agent for Activity Upload** (Line 639)
   - **C++ sends:** GCM-iOS-5.7.2.1 (iOS user-agent)
   - **Should send:** com.garmin.android.apps.connectmobile (Android user-agent)
   - **Impact:** May be intentional or may cause issues
   - **Fix Required:** Verify if upload endpoint requires iOS user-agent

10. **Explicit Origin Header** (Lines 215, 376)
    - **C++ sends:** Origin header for POST requests
    - **Python doesn't send:** Origin header
    - **Impact:** Likely none, but differs from Python implementation
    - **Fix Required:** Verify if required or can be removed

---

## 10. AUTHENTICATION FLOW GAPS

### Missing Steps

1. **No explicit request signing for OAuth endpoints**
   - C++ doesn't use any OAuth1 library
   - Python uses requests-oauthlib which handles signing

2. **No cryptographic signature calculation**
   - HMAC-SHA1 signature not computed
   - No random nonce generation
   - No timestamp generation

3. **No proper Authorization header construction**
   - OAuth1 spec requires specific format
   - C++ only partially implements this

---

## 11. DEPENDENCY ISSUES

### Python Uses:
- `requests` - HTTP client with automatic cookie handling
- `requests_oauthlib` - OAuth1 request signing (automatic HMAC-SHA1 signing)

### C++ Uses:
- Qt's `QNetworkAccessManager` - Good HTTP client
- No OAuth1 library - Manual signature calculation needed but NOT IMPLEMENTED

**Missing Dependency:** C++ needs an OAuth1 signing library or manual HMAC-SHA1 implementation

---

## 12. VERIFICATION CHECKLIST

- [x] Cookie persistence across requests - **Questionable (manual management)**
- [x] CSRF token correctly extracted and sent - **CORRECT**
- [x] Login POST includes all required parameters - **CORRECT**
- [x] MFA flow detects and extracts new CSRF token - **CORRECT**
- [x] Ticket extraction works with various formats - **CORRECT (better than Python)**
- [ ] OAuth1 signing is correct for token exchange - **❌ NOT IMPLEMENTED**
- [x] Token expiration calculation is accurate - **CORRECT**
- [x] Token refresh works before expiration - **CORRECT**
- [ ] OAuth1 response parsing uses correct format - **⚠️ JSON vs URL-encoded mismatch**
- [ ] All query parameters match Python implementation - **❌ Multiple mismatches**
- [ ] Authorization headers correctly formatted - **❌ CRITICALLY INCOMPLETE**

---

## 13. RECOMMENDATIONS - PRIORITY ORDER

### CRITICAL - Must Fix (Login Will Fail):

1. **Implement OAuth1 Signature Calculation**
   - Option A: Integrate a C++ OAuth1 library
   - Option B: Implement HMAC-SHA1 signing manually (complex)
   - Details: See section 8

2. **Fix OAuth2 Exchange POST Parameters**
   - Remove oauth_token and oauth_token_secret from POST body
   - Keep only mfa_token if present
   - Line 514-515

3. **Fix Response Parsing for OAuth1 Token**
   - Parse as URL-encoded query string, not JSON
   - Use `QUrlQuery` to parse response text
   - Line 489-495

### HIGH - Should Fix (May Fail in Some Cases):

4. **Add login-url Parameter to OAuth1 Request**
   - Add: `query.addQueryItem("login-url", ssoUrl() + SSO_EMBED_PATH);`
   - Line 465-466

5. **Fix Service Parameter in Login POST**
   - Change from `connectApiUrl()` to `ssoEmbedUrl`
   - Line 196

6. **Add SIGNIN_PARAMS to MFA Endpoint**
   - Include query parameters: id, embedWidget, gauthHost, service, source, redirectAfterAccountLoginUrl, redirectAfterAccountCreationUrl
   - Line 361

### MEDIUM - Should Consider:

7. **Add Referer Header to CSRF Fetch**
   - Include Referer from previous response
   - Line 133

8. **Remove Content-Type from OAuth1 GET Request**
   - GET requests don't need Content-Type header
   - Line 471

### LOW - May Be Intentional:

9. **Verify User-Agent for Activity Upload**
   - Check if upload endpoint requires iOS user-agent
   - Line 639

10. **Consider Removing Explicit Origin Header**
    - Verify if Garmin API requires it or if automatic handling is sufficient
    - Lines 215, 376

---

## 14. IMPLEMENTATION STRATEGY

### Phase 1: Critical Fixes (Do First)
1. Fix OAuth1 signature implementation
2. Fix OAuth2 exchange POST parameters
3. Fix OAuth1 response parsing format

### Phase 2: High Priority Fixes
4. Add login-url parameter
5. Fix service parameter in POST
6. Add SIGNIN_PARAMS to MFA endpoint

### Phase 3: Polish
7-10. Address medium and low priority items

### Testing After Fixes:
- Test login without MFA
- Test login with MFA required
- Test token refresh
- Test activity upload
- Test with different domain (garmin.cn)

---

## 15. CONCLUSION

The C++ implementation is **mostly correct** in terms of:
- Request flow and ordering
- Form parameters and CSRF handling
- Basic error handling
- Cookie management (though manual)

However, it has **CRITICAL ISSUES** with:
- OAuth1 signature calculation (completely missing)
- OAuth2 exchange parameters (wrong format)
- OAuth1 response parsing (format mismatch)
- Several query parameters and headers (missing or incorrect)

The **OAuth1 signing is the biggest blocker** to successful authentication. Without proper OAuth1 HMAC-SHA1 signature calculation, the token exchange endpoints will likely reject the requests.

---

**Last Updated:** 2025-12-03
**Comparison Source:** Python garth library from https://github.com/matin/garth
**C++ Source:** /home/user/qdomyos-zwift/src/garminconnect.cpp
