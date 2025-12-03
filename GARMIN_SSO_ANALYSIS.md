# Garmin SSO Login Implementation Analysis

## Overview
This document provides a detailed analysis of the Python garth library's SSO login implementation and compares it with the C++ Qt implementation in QDomyos-Zwift.

---

## 1. Python Garth Library Structure

### Module Organization
```
src/garth/
├── sso.py          # SSO login implementation
├── http.py         # HTTP client and request handling
├── auth_tokens.py  # Token management
└── __init__.py     # Public API
```

### Key Constants (sso.py)
```python
CSRF_RE = re.compile(r'name="_csrf"\s+value="(.+?)"')
TITLE_RE = re.compile(r"<title>(.+?)</title>")
OAUTH_CONSUMER_URL = "https://thegarth.s3.amazonaws.com/oauth_consumer.json"
USER_AGENT = {"User-Agent": "com.garmin.android.apps.connectmobile"}
```

---

## 2. Complete SSO Login Flow

### Step 1: Set Cookies via /sso/embed

**Request:**
```
GET https://sso.{domain}/sso/embed
Query Parameters:
  - id=gauth-widget
  - embedWidget=true
  - gauthHost=https://sso.{domain}/sso
```

**Headers:**
```
User-Agent: com.garmin.android.apps.connectmobile
```

**Purpose:** Establishes initial session cookies

**C++ Qt Implementation:** `/home/user/qdomyos-zwift/src/garminconnect.cpp` lines 82-113 (`fetchCookies()`)

---

### Step 2: Get CSRF Token via /sso/signin

**Request:**
```
GET https://sso.{domain}/sso/signin
Query Parameters:
  - id=gauth-widget
  - embedWidget=true
  - gauthHost=https://sso.{domain}/sso/embed
  - service=https://sso.{domain}/sso/embed
  - source=https://sso.{domain}/sso/embed
  - redirectAfterAccountLoginUrl=https://sso.{domain}/sso/embed
  - redirectAfterAccountCreationUrl=https://sso.{domain}/sso/embed
```

**Headers:**
```
User-Agent: com.garmin.android.apps.connectmobile
Cookie: [previously set cookies]
Referer: [from previous response URL]
```

**Response HTML Parsing:**
Extract CSRF token using regex:
```
Pattern: name="_csrf"\s+value="(.+?)"
```

The CSRF token is found in a hidden form field like:
```html
<input type="hidden" name="_csrf" value="[TOKEN_VALUE]" />
```

**C++ Qt Implementation:** `/home/user/qdomyos-zwift/src/garminconnect.cpp` lines 115-175 (`fetchCsrfToken()`)

---

### Step 3: Submit Login Credentials via /sso/signin (POST)

**URL:**
```
POST https://sso.{domain}/sso/signin?service=https://connectapi.{domain}&...
```

**Query Parameters:**
```
- service=https://connectapi.{domain}
- id=gauth-widget
- embedWidget=true
- gauthHost=https://sso.{domain}/sso/embed
- source=https://sso.{domain}/sso/embed
- redirectAfterAccountLoginUrl=https://sso.{domain}/sso/embed
- redirectAfterAccountCreationUrl=https://sso.{domain}/sso/embed
```

**POST Data (Form-Encoded):**
```
Content-Type: application/x-www-form-urlencoded

username={email}
password={password}
embed=true
_csrf={csrf_token}
```

**Headers:**
```
User-Agent: com.garmin.android.apps.connectmobile
Content-Type: application/x-www-form-urlencoded
Cookie: [session cookies]
Referer: https://sso.{domain}/sso/signin?[previous query]
```

**Response:**
- On success: HTML page with title "Success" and ticket in embedded URL
- On MFA required: HTML page with title containing "MFA" and new CSRF token
- On failure: Redirect back to login page or error message

**Ticket Extraction:**
```regex
Pattern: embed\?ticket=([^"]+)"
```

The response contains a redirect URL like:
```html
<a href="https://sso.{domain}/sso/embed?ticket=[TICKET_VALUE]">...</a>
```

**C++ Qt Implementation:** `/home/user/qdomyos-zwift/src/garminconnect.cpp` lines 177-317 (`performLogin()`)

---

### Step 4: MFA Verification (if required)

**URL:**
```
POST https://sso.{domain}/sso/verifyMFA/loginEnterMfaCode
Query Parameters: Same as signin
```

**POST Data:**
```
mfa-code={user_provided_code}
embed=true
_csrf={csrf_token}
fromPage=setupEnterMfaCode
```

**Headers:**
```
User-Agent: com.garmin.android.apps.connectmobile
Content-Type: application/x-www-form-urlencoded
Cookie: [session cookies]
Referer: [MFA page URL]
```

**Response:** Same as Step 3 - extracts ticket and proceeds

**C++ Qt Implementation:** `/home/user/qdomyos-zwift/src/garminconnect.cpp` lines 319-386 (`performMfaVerification()`)

---

### Step 5: Exchange Ticket for OAuth1 Token

**URL:**
```
GET https://connectapi.{domain}/oauth-service/oauth/preauthorized
Query Parameters:
  - ticket={ticket_from_sso}
  - accepts-mfa-tokens=true
```

**Headers:**
```
User-Agent: com.garmin.android.apps.connectmobile
```

**OAuth1 Consumer Credentials:**
Fetched from: `https://thegarth.s3.amazonaws.com/oauth_consumer.json`
```json
{
  "consumer_key": "string",
  "consumer_secret": "string"
}
```

**Response Format:**
```
oauth_token=value
oauth_token_secret=value
mfa_token=value (optional)
mfa_expiration_timestamp=value (optional)
```

**C++ Qt Implementation:** `/home/user/qdomyos-zwift/src/garminconnect.cpp` lines 388-464 (`exchangeForOAuth1Token()`)

---

### Step 6: Exchange OAuth1 for OAuth2 Token

**URL:**
```
POST https://connectapi.{domain}/oauth-service/oauth/exchange/user/2.0
```

**POST Data:**
```
oauth_token={oauth1_token}
oauth_token_secret={oauth1_token_secret}
```

**Headers:**
```
User-Agent: com.garmin.android.apps.connectmobile
Content-Type: application/x-www-form-urlencoded
```

**OAuth1 Signature:**
The request must be signed using OAuth1 signature algorithm with:
- Consumer Key: (from oauth_consumer.json)
- Consumer Secret: (from oauth_consumer.json)
- OAuth1 Token: (from preauthorized response)
- OAuth1 Token Secret: (from preauthorized response)

**Response JSON:**
```json
{
  "access_token": "string",
  "refresh_token": "string",
  "token_type": "Bearer",
  "expires_in": number_of_seconds,
  "refresh_token_expires_in": number_of_seconds,
  "scope": "string"
}
```

**Token Expiration Calculation:**
```
expires_at = current_timestamp + expires_in
refresh_token_expires_at = current_timestamp + refresh_token_expires_in
```

**C++ Qt Implementation:** `/home/user/qdomyos-zwift/src/garminconnect.cpp` lines 466-512 (`exchangeForOAuth2Token()`)

---

## 3. Complete HTTP Headers Summary

### Standard Headers Used Throughout
```
User-Agent: com.garmin.android.apps.connectmobile
```

### Request-Specific Headers

#### GET Requests (embed, signin, oauth)
```
User-Agent: com.garmin.android.apps.connectmobile
Cookie: [session cookies from jar]
Referer: [from previous response URL when applicable]
```

#### POST Requests (signin, MFA, oauth)
```
User-Agent: com.garmin.android.apps.connectmobile
Content-Type: application/x-www-form-urlencoded
Cookie: [session cookies]
Referer: [from previous response]
```

#### OAuth1 Requests
```
User-Agent: com.garmin.android.apps.connectmobile
Authorization: OAuth oauth_consumer_key="...", oauth_token="...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"
```

---

## 4. Cookie Handling

### Cookie Jar Management
- **Automatic:** Python's `requests.Session` automatically manages cookies
- **Persistence:** Cookies are maintained across all requests in a single session
- **C++ Qt Implementation:** Uses `QNetworkAccessManager::cookieJar()` with automatic cookie persistence

### Cookie Flow
1. First GET to `/sso/embed` sets initial session cookies
2. All subsequent requests automatically include cookies
3. Cookie jar is updated after each response
4. Cookies must be preserved throughout the entire flow until OAuth tokens are obtained

---

## 5. Form Field Extraction

### Hidden CSRF Field
**Location:** In HTML response from `/sso/signin` GET request
**Pattern:**
```regex
name="_csrf"\s+value="(.+?)"
```

**Variations Handled:**
```html
<!-- Standard order -->
<input type="hidden" name="_csrf" value="token123" />

<!-- Reversed order -->
<input type="hidden" value="token123" name="_csrf" />

<!-- Extra attributes -->
<input type="hidden" name="_csrf" id="csrf-field" value="token123" class="csrf" />
```

**C++ Qt Implementation:** `/home/user/qdomyos-zwift/src/garminconnect.cpp` lines 148-169 handles multiple regex patterns

---

## 6. URL Structure Details

### Base URL Construction
```
SSO Base:      https://sso.{domain}
ConnectAPI:    https://connectapi.{domain}
```

### Query Parameter Consistency
The `SIGNIN_PARAMS` dictionary is used for multiple endpoints:
```python
SIGNIN_PARAMS = {
    "id": "gauth-widget",
    "embedWidget": "true",
    "gauthHost": "https://sso.{domain}/sso/embed",
    "service": "https://sso.{domain}/sso/embed",
    "source": "https://sso.{domain}/sso/embed",
    "redirectAfterAccountLoginUrl": "https://sso.{domain}/sso/embed",
    "redirectAfterAccountCreationUrl": "https://sso.{domain}/sso/embed",
}
```

### Critical Point
The `service` parameter varies depending on context:
1. In `/sso/embed` GET: Uses SSO base URL
2. In `/sso/signin` GET/POST: Uses SSO embed URL OR ConnectAPI URL (varies by Garmin)
3. In OAuth requests: Uses ConnectAPI base URL

---

## 7. Response Parsing and Error Handling

### Success Detection
```python
# From response title
title = re.search(r"<title>(.+?)</title>", response).group(1)
if title == "Success":
    # Extract ticket
    ticket = re.search(r'embed\?ticket=([^"]+)"', response).group(1)
```

### MFA Detection
```python
if "MFA" in title:
    # MFA required
    # Extract new CSRF token and prompt user
```

### Ticket Extraction
```regex
Pattern: embed\?ticket=([^"]+)"
```

The ticket appears in an href attribute:
```html
<a href="https://sso.{domain}/sso/embed?ticket=ABC123DEF456">...</a>
```

---

## 8. Comparison with C++ Qt Implementation

### Current Implementation Status

**Fully Implemented:**
- Cookie fetching via /sso/embed (lines 82-113)
- CSRF token extraction (lines 115-175)
- Login POST request (lines 177-317)
- MFA verification (lines 319-386)
- OAuth1 token exchange (lines 388-464)
- OAuth2 token exchange (lines 466-512)
- Token refresh (lines 514-561)
- Token persistence (lines 645-703)

**Potential Issues Identified:**

1. **Line 183 (fetchCsrfToken):**
   ```cpp
   query.addQueryItem("service", connectApiUrl());
   ```
   May need to use SSO URL instead of ConnectAPI URL

2. **Line 425 (exchangeForOAuth1Token):**
   ```cpp
   query.addQueryItem("login-url", ssoUrl() + SSO_EMBED_PATH);
   ```
   This parameter is not in the Python garth library - may be unnecessary

3. **Line 474-475 (exchangeForOAuth2Token):**
   ```cpp
   postData.addQueryItem("oauth_token", m_oauth1Token.oauth_token);
   postData.addQueryItem("oauth_token_secret", m_oauth1Token.oauth_token_secret);
   ```
   These are being sent as POST data, but the Python library uses OAuth1 signing instead

4. **Missing OAuth1 Signature:**
   The C++ implementation doesn't properly implement OAuth1 signing for the `/oauth-service/oauth/exchange/user/2.0` endpoint

5. **User-Agent Inconsistency:**
   Current code uses "com.garmin.android.apps.connectmobile" (correct) but some paths might use "GCM-iOS-5.7.2.1"

---

## 9. Required POST Parameters Summary

### /sso/signin (Login)
```
username={email}
password={password}
embed=true
_csrf={csrf_token}
```

### /sso/verifyMFA/loginEnterMfaCode
```
mfa-code={code}
embed=true
_csrf={csrf_token}
fromPage=setupEnterMfaCode
```

### /oauth-service/oauth/exchange/user/2.0
```
[OAuth1 Signed Request with]:
  oauth_token={value}
  oauth_token_secret={value}
  [or include mfa_token if present]
```

---

## 10. SIGNIN_PARAMS Query Parameters

These parameters are used in multiple GET/POST requests:
```
id=gauth-widget
embedWidget=true
gauthHost=https://sso.{domain}/sso/embed
service=https://sso.{domain}/sso/embed
source=https://sso.{domain}/sso/embed
redirectAfterAccountLoginUrl=https://sso.{domain}/sso/embed
redirectAfterAccountCreationUrl=https://sso.{domain}/sso/embed
```

---

## 11. Key Differences from Expected Implementation

1. **Python uses automatic OAuth1 signing** via `requests-oauthlib` library
2. **C++ needs explicit OAuth1 signature calculation** if not using an OAuth1 library
3. **Domain flexibility:** Both support custom domains (garmin.com, garmin.cn, etc.)
4. **Token expiration handling:** Both calculate expiration timestamps from `expires_in` values
5. **MFA support:** Both handle MFA with additional POST request and CSRF token refresh

---

## 12. Recommendations for C++ Qt Implementation

### Critical Fixes Needed:
1. **Implement proper OAuth1 signing** for the OAuth2 exchange endpoint
2. **Remove the "login-url" query parameter** from OAuth1 token exchange (line 425)
3. **Verify the "service" parameter** in fetchCsrfToken matches Garmin's current requirements
4. **Add OAuth1 library support** (e.g., oauth-cpp or implement HMAC-SHA1 signing manually)

### Testing Checklist:
- [ ] Cookie persistence across all requests
- [ ] CSRF token correctly extracted and sent
- [ ] Login POST includes all required parameters
- [ ] MFA flow extracts new CSRF token and submits correctly
- [ ] Ticket extraction works with various HTML formats
- [ ] OAuth1 signing is correct for token exchange
- [ ] Token expiration calculation is accurate
- [ ] Token refresh works before expiration

---

## Reference Files

- **Python Implementation:** `https://github.com/matin/garth/blob/main/src/garth/sso.py`
- **C++ Qt Implementation:** `/home/user/qdomyos-zwift/src/garminconnect.cpp`
- **Header:** `/home/user/qdomyos-zwift/src/garminconnect.h`

---

## Constants Reference

### Domain-Based URLs
```python
SSO_BASE = f"https://sso.{domain}"
SSO_EMBED = f"{SSO_BASE}/sso/embed"
CONNECT_API = f"https://connectapi.{domain}"
```

### Endpoints
```
/sso/embed                                    (GET) - Set cookies
/sso/signin                                   (GET) - Get CSRF token
/sso/signin                                   (POST) - Submit credentials
/sso/verifyMFA/loginEnterMfaCode              (POST) - Submit MFA code
/oauth-service/oauth/preauthorized             (GET) - Get OAuth1 token
/oauth-service/oauth/exchange/user/2.0        (POST) - Get OAuth2 token
```

---

## Python Garth Source Code Snippets

### Login Function Signature
```python
def login(
    email: str,
    password: str,
    /,
    client: "http.Client | None" = None,
    prompt_mfa: Callable | None = lambda: input("MFA code: "),
    return_on_mfa: bool = False,
) -> tuple[OAuth1Token, OAuth2Token] | tuple[Literal["needs_mfa"], dict[str, Any]]:
```

### SIGNIN_PARAMS Construction
```python
SIGNIN_PARAMS = {
    "id": "gauth-widget",
    "embedWidget": "true",
    "gauthHost": SSO_EMBED,
    "service": SSO_EMBED,
    "source": SSO_EMBED,
    "redirectAfterAccountLoginUrl": SSO_EMBED,
    "redirectAfterAccountCreationUrl": SSO_EMBED,
}
```

### CSRF Token Extraction
```python
CSRF_RE = re.compile(r'name="_csrf"\s+value="(.+?)"')
def get_csrf_token(html: str) -> str:
    m = CSRF_RE.search(html)
    if not m:
        raise GarthException("Couldn't find CSRF token")
    return m.group(1)
```

### Ticket Extraction
```python
m = re.search(r'embed\?ticket=([^"]+)"', client.last_resp.text)
if not m:
    raise GarthException("Couldn't find ticket in response")
ticket = m.group(1)
```

---

## Summary

The Garmin SSO login flow is a 6-step process:
1. **Set cookies** via `/sso/embed` GET
2. **Get CSRF token** via `/sso/signin` GET
3. **Submit credentials** via `/sso/signin` POST
4. **Handle MFA** (if required) via `/sso/verifyMFA/loginEnterMfaCode` POST
5. **Get OAuth1 token** via `/oauth-service/oauth/preauthorized` GET
6. **Get OAuth2 token** via `/oauth-service/oauth/exchange/user/2.0` POST (OAuth1 signed)

All requests use consistent User-Agent and automatic cookie management. The implementation must properly handle CSRF token extraction, MFA flow, and OAuth1 signing for the final token exchange.
