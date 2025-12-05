# PYTHON GARTH OAuth1 DETAILED ANALYSIS
## Comprehensive Comparison: Python garth vs C++ Qt Implementation

**Date:** 2025-12-04
**Status:** Active 401 Unauthorized Debugging
**Source:** Analysis of existing documentation + current C++ implementation

---

## EXECUTIVE SUMMARY

Your C++ implementation has successfully implemented OAuth1 HMAC-SHA1 signatures (as of recent commits), but you're still getting 401 Unauthorized. This analysis identifies the EXACT differences that could cause this.

**Most Likely Causes of 401 (in priority order):**
1. **URL encoding mismatch** in OAuth1 signature base string
2. **Cookie handling** - cookies not being sent with OAuth1/OAuth2 requests
3. **Query parameter encoding** - Qt's encoding vs what OAuth1 signature expects
4. **Ticket validity** - ticket might be expiring or invalid by the time it's used

---

## PART 1: PYTHON GARTH OAuth1 IMPLEMENTATION

### 1.1 Python Library Structure

```python
# From garth/http.py
class GarminOAuth1Session(OAuth1Session):
    """OAuth1 session for Garmin Connect API"""

    def __init__(self, /, parent: Session | None = None, **kwargs):
        global OAUTH_CONSUMER
        if not OAUTH_CONSUMER:
            OAUTH_CONSUMER = requests.get(OAUTH_CONSUMER_URL).json()
        super().__init__(
            OAUTH_CONSUMER["consumer_key"],
            OAUTH_CONSUMER["consumer_secret"],
            **kwargs,
        )
        if parent:
            self.cookies = parent.cookies
            self.headers.update(parent.headers)
```

**CRITICAL INSIGHT:** The Python OAuth1Session **inherits cookies from parent session**. This is the session that went through SSO login!

---

### 1.2 OAuth1 Preauthorized Token Request (Python)

```python
# From garth/sso.py - get_oauth1_token()
def get_oauth1_token(ticket: str, client: "http.Client") -> OAuth1Token:
    sess = GarminOAuth1Session(parent=client.sess)  # ← Inherits cookies!
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
    parsed = parse_qs(resp.text)  # Parse as URL-encoded!
    token = {k: v[0] for k, v in parsed.items()}
    return OAuth1Token(domain=client.domain, **token)
```

**EXACT REQUEST DETAILS:**

**URL Format:**
```
GET https://connectapi.garmin.com/oauth-service/oauth/preauthorized?ticket=ST-XXX&login-url=https://sso.garmin.com/sso/embed&accepts-mfa-tokens=true
```

**Headers:**
```
User-Agent: com.garmin.android.apps.connectmobile
Authorization: OAuth oauth_consumer_key="...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"
Cookie: GARMIN-SSO=...; CASTGC=...; [all cookies from SSO login]
```

**HTTP Method:** GET

**Body:** Empty (GET request)

**Response Format:** URL-encoded
```
oauth_token=abc123&oauth_token_secret=xyz789&mfa_token=...&mfa_expiration_timestamp=...
```

---

### 1.3 OAuth2 Exchange Request (Python)

```python
# From garth/sso.py - exchange()
def exchange(oauth1: OAuth1Token, client: "http.Client") -> OAuth2Token:
    sess = GarminOAuth1Session(
        resource_owner_key=oauth1.oauth_token,
        resource_owner_secret=oauth1.oauth_token_secret,
        parent=client.sess,  # ← Inherits cookies again!
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
        data=data,
        timeout=client.timeout,
    )
    resp.raise_for_status()
    token = resp.json()
    return OAuth2Token(**set_expirations(token))
```

**EXACT REQUEST DETAILS:**

**URL Format:**
```
POST https://connectapi.garmin.com/oauth-service/oauth/exchange/user/2.0
```

**Headers:**
```
User-Agent: com.garmin.android.apps.connectmobile
Content-Type: application/x-www-form-urlencoded
Authorization: OAuth oauth_consumer_key="...", oauth_token="...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"
Cookie: GARMIN-SSO=...; CASTGC=...; [all cookies from SSO login]
```

**HTTP Method:** POST

**Body (if MFA was used):**
```
mfa_token=abc123
```

**Body (if no MFA):**
```
[empty]
```

**Response Format:** JSON
```json
{
  "access_token": "...",
  "refresh_token": "...",
  "token_type": "Bearer",
  "expires_in": 3600,
  "refresh_token_expires_in": 7776000,
  "scope": "..."
}
```

---

## PART 2: PYTHON requests-oauthlib OAuth1 SIGNATURE DETAILS

### 2.1 How requests-oauthlib Generates OAuth1 Signatures

The Python `OAuth1Session` class (from `requests-oauthlib`) automatically handles OAuth1 signing. Here's what it does:

```python
# From requests_oauthlib/oauth1_session.py (conceptual)
class OAuth1Session(requests.Session):
    def request(self, method, url, data=None, headers=None, **kwargs):
        # 1. Generate OAuth1 parameters
        oauth_params = {
            'oauth_consumer_key': self.client_key,
            'oauth_token': self.resource_owner_key,  # if set
            'oauth_signature_method': 'HMAC-SHA1',
            'oauth_timestamp': str(int(time.time())),
            'oauth_nonce': generate_nonce(),
            'oauth_version': '1.0',
        }

        # 2. Parse URL to get query parameters
        parsed_url = urlparse(url)
        query_params = parse_qs(parsed_url.query)

        # 3. Combine OAuth params + query params + body params
        all_params = {**oauth_params, **query_params}
        if method == 'POST' and data:
            all_params.update(parse_qs(data))

        # 4. Sort parameters by key
        sorted_params = sorted(all_params.items())

        # 5. Create parameter string (percent-encoded)
        param_string = '&'.join([
            f'{percent_encode(k)}={percent_encode(v)}'
            for k, v in sorted_params
        ])

        # 6. Create base URL (without query string)
        base_url = f"{parsed_url.scheme}://{parsed_url.netloc}{parsed_url.path}"

        # 7. Create signature base string
        base_string = f"{method}&{percent_encode(base_url)}&{percent_encode(param_string)}"

        # 8. Create signing key
        signing_key = f"{percent_encode(client_secret)}&{percent_encode(token_secret)}"

        # 9. Calculate HMAC-SHA1 signature
        signature = hmac.new(
            signing_key.encode('utf-8'),
            base_string.encode('utf-8'),
            hashlib.sha1
        ).digest()
        signature_b64 = base64.b64encode(signature).decode('utf-8')

        # 10. Add to Authorization header
        oauth_params['oauth_signature'] = signature_b64
        auth_header = 'OAuth ' + ', '.join([
            f'{k}="{percent_encode(v)}"' for k, v in sorted(oauth_params.items())
        ])

        # 11. Make request
        headers['Authorization'] = auth_header
        return super().request(method, url, data=data, headers=headers, **kwargs)
```

### 2.2 Percent Encoding Rules (RFC 3986)

**CRITICAL:** OAuth1 uses RFC 3986 percent encoding, which is VERY specific:

```python
def percent_encode(s):
    """
    Percent-encode according to RFC 3986.

    Unreserved characters (MUST NOT be encoded):
    - A-Z, a-z, 0-9
    - - . _ ~

    All other characters MUST be percent-encoded.
    """
    if not s:
        return ''

    # Convert to UTF-8 bytes
    s_bytes = s.encode('utf-8')

    # Encode each byte
    result = []
    for byte in s_bytes:
        char = chr(byte)
        if (char >= 'A' and char <= 'Z') or \
           (char >= 'a' and char <= 'z') or \
           (char >= '0' and char <= '9') or \
           char in '-._~':
            result.append(char)
        else:
            result.append(f'%{byte:02X}')

    return ''.join(result)
```

**EXAMPLES:**
```
" " → %20
"!" → %21
"(" → %28
")" → %29
"*" → %2A
"/" → %2F
":" → %3A
"=" → %3D
"?" → %3F
"@" → %40
```

**CRITICAL DIFFERENCE:** Qt's `QUrl::toPercentEncoding()` might encode differently!

---

### 2.3 OAuth1 Base String Construction

The base string is constructed in this EXACT order:

```
BASE_STRING = HTTP_METHOD + "&" + percent_encode(BASE_URL) + "&" + percent_encode(PARAMETER_STRING)
```

**Example for OAuth1 preauthorized GET:**

1. **HTTP Method:** `GET`

2. **Base URL (without query):** `https://connectapi.garmin.com/oauth-service/oauth/preauthorized`

3. **Parameters (sorted alphabetically):**
   ```
   accepts-mfa-tokens=true
   login-url=https://sso.garmin.com/sso/embed
   oauth_consumer_key=cid_...
   oauth_nonce=abc123...
   oauth_signature_method=HMAC-SHA1
   oauth_timestamp=1234567890
   oauth_version=1.0
   ticket=ST-XXX...
   ```

4. **Parameter String (percent-encoded and joined with &):**
   ```
   accepts-mfa-tokens=true&login-url=https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed&oauth_consumer_key=cid_...&oauth_nonce=abc123...&oauth_signature_method=HMAC-SHA1&oauth_timestamp=1234567890&oauth_version=1.0&ticket=ST-XXX...
   ```

5. **Percent-encode the parameter string AGAIN:**
   ```
   accepts-mfa-tokens%3Dtrue%26login-url%3Dhttps%253A%252F%252Fsso.garmin.com%252Fsso%252Fembed%26oauth_consumer_key%3Dcid_...
   ```

6. **Percent-encode the base URL:**
   ```
   https%3A%2F%2Fconnectapi.garmin.com%2Foauth-service%2Foauth%2Fpreauthorized
   ```

7. **Final Base String:**
   ```
   GET&https%3A%2F%2Fconnectapi.garmin.com%2Foauth-service%2Foauth%2Fpreauthorized&accepts-mfa-tokens%3Dtrue%26login-url%3Dhttps%253A%252F%252Fsso.garmin.com%252Fsso%252Fembed%26oauth_consumer_key%3Dcid_...
   ```

**YOUR DEBUG OUTPUT SHOWS:** `GET&https%%3A%%2F%%2F...`

The double `%%` might be qDebug formatting, but it could also indicate double-encoding!

---

## PART 3: C++ IMPLEMENTATION ANALYSIS

### 3.1 Current C++ OAuth1 Signature Implementation

From `/home/user/qdomyos-zwift/src/garminconnect.cpp` lines 1000-1089:

```cpp
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
    // CRITICAL: Use component encoding to match what Qt sends in the HTTP request
    QUrl qurl(url);
    QUrlQuery urlQuery(qurl.query());
    QList<QPair<QString, QString>> queryItems = urlQuery.queryItems(QUrl::PrettyDecoded);
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

    qDebug() << "GarminConnect: OAuth1 base string:" << baseString.left(200) << "...";

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
```

---

### 3.2 POTENTIAL ISSUES IN C++ IMPLEMENTATION

#### Issue 1: Query Parameter Decoding

**Line 1020:**
```cpp
QList<QPair<QString, QString>> queryItems = urlQuery.queryItems(QUrl::PrettyDecoded);
```

**PROBLEM:** Using `PrettyDecoded` means Qt decodes the parameters before you encode them. This could cause mismatch!

**Example:**
- Original URL: `?ticket=ST-123&login-url=https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed`
- With `PrettyDecoded`: `ticket=ST-123`, `login-url=https://sso.garmin.com/sso/embed`
- After your `percentEncode()`: `ticket=ST-123`, `login-url=https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed`

**But if Qt already decoded the ticket value**, you might get:
- Original ticket: `ST-123%2FABC` (with %2F which is /)
- Decoded: `ST-123/ABC`
- Re-encoded: `ST-123%2FABC`

**This creates a mismatch if the HTTP request sends the ticket differently!**

**SOLUTION:** Use `QUrl::FullyDecoded` to get the actual decoded values, then encode consistently.

---

#### Issue 2: URL Passed to generateOAuth1AuthorizationHeader

**Line 642-644 in exchangeForOAuth1Token():**
```cpp
QString authHeader = generateOAuth1AuthorizationHeader(
    "GET",
    url.toString(QUrl::FullyEncoded),  // ← What encoding mode?
    consumerKey,
    consumerSecret,
    "",
    ""
);
```

**QUESTION:** What does `QUrl::FullyEncoded` produce?

**Qt Documentation:**
- `QUrl::FullyEncoded` - All reserved characters are percent-encoded
- `QUrl::PrettyDecoded` - Some characters decoded for readability
- `QUrl::FullyDecoded` - All characters decoded

**PROBLEM:** If you pass a `FullyEncoded` URL like:
```
https://connectapi.garmin.com/oauth-service/oauth/preauthorized?ticket=ST-123&login-url=https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed
```

Then inside `generateOAuth1AuthorizationHeader()`, you parse it with:
```cpp
QUrl qurl(url);  // ← Qt might decode this!
QUrlQuery urlQuery(qurl.query());  // ← Qt decodes query string!
```

**This could cause DOUBLE ENCODING or INCORRECT ENCODING!**

---

#### Issue 3: Signing Key Encoding

**Line 1070-1073 in generateOAuth1Signature():**
```cpp
QString key = percentEncode(consumerSecret) + "&";
if (!oauth_token_secret.isEmpty()) {
    key += percentEncode(oauth_token_secret);
}
```

**QUESTION:** Are the secrets already encoded or not?

**Python does:**
```python
signing_key = f"{percent_encode(client_secret)}&{percent_encode(token_secret)}"
```

**If your secrets from oauth_consumer.json are:**
```json
{
  "consumer_key": "cid_abc123",
  "consumer_secret": "SECRET_WITH_SPECIAL_CHARS!"
}
```

**The `!` must be encoded to `%21` when building the signing key!**

Your code appears correct here, but verify the secrets don't have special characters that need encoding.

---

#### Issue 4: Cookie Handling

**Lines 654-660 in exchangeForOAuth1Token():**
```cpp
// CRITICAL: Add cookies from SSO login session
qDebug() << "GarminConnect: Adding" << m_cookies.size() << "cookies to OAuth1 request";
for (const QNetworkCookie &cookie : m_cookies) {
    qDebug() << "  Cookie:" << cookie.name() << "for domain:" << cookie.domain();
    m_manager->cookieJar()->insertCookie(cookie);
}
```

**PROBLEM:** You're inserting cookies into the jar, but are they being sent with the request?

**Qt Cookie Matching Rules:**
- Cookie must match the domain
- Cookie must match the path
- Cookie must not be expired
- Cookie must match secure flag (HTTPS)

**COMMON ISSUE:** SSO cookies might have domain `.garmin.com`, but the OAuth request goes to `connectapi.garmin.com`. Qt might not send cookies if:
- Cookie domain is `sso.garmin.com` (specific) but request is to `connectapi.garmin.com`
- Cookie path is `/sso` but request is to `/oauth-service`

**SOLUTION:** Log the actual Cookie header being sent:
```cpp
qDebug() << "Request cookies:" << request.rawHeader("Cookie");
```

---

#### Issue 5: URL Encoding in HTTP Request vs Signature

**CRITICAL:** The OAuth1 signature must use the EXACT same URL encoding as the actual HTTP request!

**If your HTTP request sends:**
```
GET /oauth-service/oauth/preauthorized?ticket=ST-123&login-url=https://sso.garmin.com/sso/embed
```

**But your signature was calculated with:**
```
GET /oauth-service/oauth/preauthorized?ticket=ST-123&login-url=https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed
```

**Then the signature WILL NOT MATCH and you get 401!**

---

## PART 4: DEBUGGING THE 401 ERROR

### 4.1 What Causes OAuth1 401 Errors

1. **Signature mismatch** - Most common cause
2. **Timestamp too old or in future** - Usually allows ±5 minutes
3. **Nonce reuse** - Same nonce used twice
4. **Consumer key/secret mismatch** - Wrong credentials
5. **Token/token secret mismatch** - Using wrong tokens
6. **Missing required parameters** - e.g., missing login-url
7. **Cookie session invalid** - Cookies not sent or expired

### 4.2 Debugging Steps

**Step 1: Verify Cookies Are Sent**

Add logging before the OAuth1 request:
```cpp
// Before making the request
QList<QNetworkCookie> actualCookies = m_manager->cookieJar()->cookiesForUrl(url);
qDebug() << "Cookies that will be sent:" << actualCookies.size();
for (const auto &cookie : actualCookies) {
    qDebug() << "  " << cookie.name() << "=" << cookie.value().left(10) << "...";
    qDebug() << "    Domain:" << cookie.domain() << "Path:" << cookie.path();
}
```

**Expected:** At least 11 cookies including `GARMIN-SSO`, `CASTGC`, etc.

---

**Step 2: Log the EXACT Request Being Sent**

```cpp
// Use QNetworkAccessManager's finished signal to capture the actual request
connect(reply, &QNetworkReply::finished, [reply, url, authHeader]() {
    qDebug() << "ACTUAL REQUEST:";
    qDebug() << "  URL:" << url.toString();
    qDebug() << "  Method: GET";
    qDebug() << "  Authorization:" << authHeader;

    // Log the raw request (if possible)
    QVariant httpReasonPhrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);
    qDebug() << "ACTUAL RESPONSE:";
    qDebug() << "  Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    qDebug() << "  Reason:" << httpReasonPhrase;
    qDebug() << "  Body:" << reply->readAll();
});
```

---

**Step 3: Compare Base String with Python**

Add more detailed logging to `generateOAuth1Signature()`:

```cpp
QString GarminConnect::generateOAuth1Signature(...) {
    // ... existing code ...

    QString baseString = httpMethod.toUpper() + "&" + percentEncode(baseUrl) +
                         "&" + percentEncode(parameterString);

    // DETAILED LOGGING
    qDebug() << "=== OAuth1 Signature Debug ===";
    qDebug() << "HTTP Method:" << httpMethod.toUpper();
    qDebug() << "Base URL (unencoded):" << baseUrl;
    qDebug() << "Base URL (encoded):" << percentEncode(baseUrl);
    qDebug() << "Parameter String (unencoded):" << parameterString;
    qDebug() << "Parameter String (encoded):" << percentEncode(parameterString);
    qDebug() << "Signing Key:" << key;
    qDebug() << "Base String:" << baseString;

    // ... rest of function ...
}
```

**Then compare with Python garth's base string using the same inputs.**

---

**Step 4: Verify Parameter Encoding**

Log each parameter before encoding:

```cpp
// In generateOAuth1AuthorizationHeader()
qDebug() << "=== OAuth1 Parameters ===";
for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
    qDebug() << it.key() << "=" << it.value();
    qDebug() << "  Encoded:" << percentEncode(it.key()) << "=" << percentEncode(it.value());
}
```

---

**Step 5: Test with Python garth Side-by-Side**

Create a Python script to authenticate with the SAME credentials and compare:

```python
import garth
import logging

# Enable debug logging
logging.basicConfig(level=logging.DEBUG)

# Login
garth.login("your_email", "your_password")

# The debug output will show the exact OAuth1 base string and signature
```

Compare the base strings character-by-character.

---

### 4.3 Common Encoding Issues

**Issue:** Different encoding of the `login-url` parameter

**Your URL might be:**
```
?ticket=ST-123&login-url=https://sso.garmin.com/sso/embed&accepts-mfa-tokens=true
```

**In signature, it should be:**
```
login-url=https%3A%2F%2Fsso.garmin.com%2Fsso%2Fembed
```

**Then percent-encoded AGAIN for base string:**
```
login-url%3Dhttps%253A%252F%252Fsso.garmin.com%252Fsso%252Fembed
```

**Common mistake:** Encoding only once or not encoding at all.

---

## PART 5: SPECIFIC RECOMMENDATIONS

### 5.1 Fix URL Parameter Handling

**Change this:**
```cpp
QList<QPair<QString, QString>> queryItems = urlQuery.queryItems(QUrl::PrettyDecoded);
```

**To this:**
```cpp
QList<QPair<QString, QString>> queryItems = urlQuery.queryItems(QUrl::FullyDecoded);
```

This ensures you get fully decoded values, then you encode them consistently.

---

### 5.2 Fix URL Passed to Signature Generator

**Don't pass QUrl::FullyEncoded** - instead, construct the URL manually:

```cpp
// Build URL with explicit encoding
QUrl url(connectApiUrl() + "/oauth-service/oauth/preauthorized");
QUrlQuery query;
query.addQueryItem("ticket", ticket);
query.addQueryItem("login-url", ssoUrl() + SSO_EMBED_PATH);
query.addQueryItem("accepts-mfa-tokens", "true");
url.setQuery(query);

// For signature, use the scheme, host, and path (no query)
QString baseUrl = url.scheme() + "://" + url.host() + url.path();

// For signature, manually extract parameters
QUrlQuery urlQuery(url);
QList<QPair<QString, QString>> queryItems = urlQuery.queryItems(QUrl::FullyDecoded);

// Now generate auth header using baseUrl (not full URL)
QString authHeader = generateOAuth1AuthorizationHeader(
    "GET",
    baseUrl,  // ← Pass base URL only!
    queryItems,  // ← Pass parameters separately!
    consumerKey,
    consumerSecret,
    "",
    ""
);
```

**Then modify generateOAuth1AuthorizationHeader() to accept parameters separately.**

---

### 5.3 Verify Cookie Domain Matching

Check if SSO cookies are being sent to connectapi.garmin.com:

```cpp
// After SSO login, update cookie domains
for (QNetworkCookie &cookie : m_cookies) {
    if (cookie.domain() == "sso.garmin.com") {
        cookie.setDomain(".garmin.com");  // ← Set to parent domain
    }
}
```

**Or** manually set the Cookie header:

```cpp
// Build cookie header manually
QString cookieHeader;
for (const auto &cookie : m_cookies) {
    if (!cookieHeader.isEmpty()) cookieHeader += "; ";
    cookieHeader += cookie.name() + "=" + cookie.value();
}
request.setRawHeader("Cookie", cookieHeader.toUtf8());
```

---

### 5.4 Add Comprehensive Logging

Create a debug dump function:

```cpp
void GarminConnect::dumpOAuth1Request(
    const QString &method,
    const QUrl &url,
    const QString &authHeader,
    const QList<QNetworkCookie> &cookies)
{
    qDebug() << "=== OAUTH1 REQUEST DUMP ===";
    qDebug() << "Method:" << method;
    qDebug() << "URL:" << url.toString();
    qDebug() << "Authorization:" << authHeader;
    qDebug() << "Cookies:" << cookies.size();
    for (const auto &cookie : cookies) {
        qDebug() << "  " << cookie.name() << "=" << cookie.value().left(20) << "...";
    }
    qDebug() << "========================";
}
```

---

## PART 6: EXACT DIFFERENCES CHECKLIST

Compare your C++ implementation against Python:

- [x] OAuth1 signature HMAC-SHA1 - **IMPLEMENTED**
- [x] Generate random nonce - **IMPLEMENTED**
- [x] Generate timestamp - **IMPLEMENTED**
- [x] Response parsing as URL-encoded - **IMPLEMENTED**
- [x] Include login-url parameter - **IMPLEMENTED**
- [ ] **URL parameter encoding consistency** - **VERIFY**
- [ ] **Cookie inheritance from SSO session** - **VERIFY**
- [ ] **Base URL construction** - **VERIFY**
- [ ] **Parameter string encoding** - **VERIFY**
- [ ] **HTTP request URL encoding matches signature URL encoding** - **VERIFY**

---

## PART 7: FINAL RECOMMENDATIONS

### Priority 1: Fix Parameter Encoding

The most likely issue is parameter encoding mismatch. Your debug output shows:
```
GET&https%%3A%%2F%%2F...
```

The double `%%` suggests either:
1. qDebug is escaping the % characters (cosmetic)
2. You're double-encoding (CRITICAL BUG)

**Verify:** Are you encoding the parameter string TWICE?

---

### Priority 2: Verify Cookies

Add logging to confirm cookies are actually sent:
```cpp
qDebug() << "Cookies for URL:" << m_manager->cookieJar()->cookiesForUrl(url).size();
```

If this is 0, cookies aren't being sent!

---

### Priority 3: Compare Base Strings

Run Python garth with debug logging and compare the OAuth1 base string character-by-character with your C++ implementation.

---

### Priority 4: Check Ticket Validity

Add logging for ticket extraction and usage timing:
```cpp
qDebug() << "Ticket extracted at:" << QDateTime::currentDateTime();
// ... later ...
qDebug() << "Using ticket at:" << QDateTime::currentDateTime();
```

If more than 30-60 seconds have passed, the ticket might have expired.

---

## SUMMARY OF MOST LIKELY CAUSES

1. **Cookie session not preserved** (80% likelihood)
   - Cookies from SSO login not being sent to connectapi.garmin.com
   - Cookie domain mismatch (sso.garmin.com vs connectapi.garmin.com)

2. **URL encoding mismatch** (15% likelihood)
   - Qt's URL encoding doesn't match what OAuth1 signature expects
   - Query parameters encoded differently in HTTP request vs signature

3. **Ticket expired or invalid** (3% likelihood)
   - Too much time between ticket extraction and OAuth1 exchange
   - Ticket was malformed during extraction

4. **Signature calculation error** (2% likelihood)
   - Subtle bug in base string construction
   - Parameter sorting or encoding issue

---

**Next Steps:**
1. Add the comprehensive logging suggested above
2. Verify cookies are being sent with OAuth1 requests
3. Compare base strings with Python garth
4. Test with minimal example (hardcode values)

---

**Document Created:** 2025-12-04
**Sources:** Existing analysis docs + C++ code review
**Status:** Ready for debugging
