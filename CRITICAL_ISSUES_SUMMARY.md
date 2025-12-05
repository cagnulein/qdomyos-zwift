# CRITICAL ISSUES SUMMARY - C++ vs Python Garth

**Status:** Production blocking issues identified
**Severity Distribution:** 5 CRITICAL | 3 HIGH | 2 MEDIUM | 2 LOW

---

## CRITICAL ISSUES (AUTHENTICATION WILL FAIL)

### 1. OAuth1 Signature Not Implemented

**Location:** Lines 469-475 (OAuth1 preauthorized GET) + Lines 514-521 (OAuth2 exchange POST)

**Current C++ Code:**
```cpp
// OAuth1 preauthorized:
QString authHeader = QString("OAuth oauth_consumer_key=\"%1\"").arg(consumerKey);
request.setRawHeader("Authorization", authHeader.toUtf8());  // INCOMPLETE!

// OAuth2 exchange:
postData.addQueryItem("oauth_token", m_oauth1Token.oauth_token);
postData.addQueryItem("oauth_token_secret", m_oauth1Token.oauth_token_secret);
// No signature at all!
```

**Expected (Python):**
```
Authorization: OAuth oauth_consumer_key="...", oauth_token="...", oauth_signature_method="HMAC-SHA1", oauth_signature="...", oauth_nonce="...", oauth_timestamp="...", oauth_version="1.0"
```

**Impact:** ❌ **COMPLETE AUTHENTICATION FAILURE**

The Garmin Connect API REQUIRES valid OAuth1 signatures. Without them:
- OAuth1 token exchange will fail
- OAuth2 token exchange will fail
- Entire login flow blocked

**Fix Required:** Implement HMAC-SHA1 OAuth1 signature calculation or integrate an OAuth1 library

**Complexity:** HIGH - Requires cryptographic operations

---

### 2. Wrong POST Body for OAuth2 Exchange

**Location:** Lines 514-515 in `exchangeForOAuth2Token()`

**Current C++ Code:**
```cpp
postData.addQueryItem("oauth_token", m_oauth1Token.oauth_token);
postData.addQueryItem("oauth_token_secret", m_oauth1Token.oauth_token_secret);
```

**Expected (Python):**
```python
data = dict(mfa_token=oauth1.mfa_token) if oauth1.mfa_token else {}
# Only mfa_token if present, NOT oauth_token/oauth_token_secret!
```

**Impact:** ❌ **COMPLETE OAUTH2 FAILURE**

Garmin Connect API will reject the request because:
- oauth_token and oauth_token_secret should NEVER be in request body
- They should be used ONLY for OAuth1 signature calculation
- Request body should contain only mfa_token (if MFA was used)

**Fix Required:** Remove these two lines entirely. Add only mfa_token if it exists.

**Complexity:** LOW - Simple removal

---

### 3. OAuth1 Response Parsing Format Mismatch

**Location:** Lines 489-495 in `exchangeForOAuth1Token()`

**Current C++ Code:**
```cpp
QJsonDocument consumerDoc = QJsonDocument::fromJson(consumerReply->readAll());
QJsonObject consumerObj = consumerDoc.object();
QString consumerKey = consumerObj["consumer_key"].toString();

// Later for OAuth1 token response:
QJsonObject jsonResponse = extractJsonFromResponse(reply);
m_oauth1Token.oauth_token = jsonResponse["oauth_token"].toString();
```

**Expected (Python):**
```python
# For oauth_consumer.json - CORRECT as JSON
OAUTH_CONSUMER = requests.get(OAUTH_CONSUMER_URL).json()  # ✓ JSON

# For preauthorized response - WRONG in C++, should be URL-encoded!
parsed = parse_qs(resp.text)  # URL query string format like: oauth_token=value&oauth_token_secret=value
token = {k: v[0] for k, v in parsed.items()}
```

**Impact:** ❌ **OAUTH1 TOKEN PARSING FAILURE**

The `/oauth-service/oauth/preauthorized` endpoint returns URL-encoded form data, NOT JSON:
```
oauth_token=abc123&oauth_token_secret=xyz789&mfa_token=...&mfa_expiration_timestamp=...
```

C++ tries to parse this as JSON, which will fail.

**Fix Required:** Parse the response using `QUrlQuery` instead of `QJsonDocument`

**Complexity:** MEDIUM - Requires changing response parsing logic

**Code Example:**
```cpp
QUrlQuery responseQuery(QString::fromUtf8(reply->readAll()));
m_oauth1Token.oauth_token = responseQuery.queryItemValue("oauth_token");
m_oauth1Token.oauth_token_secret = responseQuery.queryItemValue("oauth_token_secret");
m_oauth1Token.mfa_token = responseQuery.queryItemValue("mfa_token");
```

---

### 4. Missing login-url Parameter in OAuth1 Request

**Location:** Lines 465-466 in `exchangeForOAuth1Token()`

**Current C++ Code:**
```cpp
query.addQueryItem("ticket", ticket);
query.addQueryItem("accepts-mfa-tokens", "true");
// MISSING: login-url parameter
```

**Expected (Python):**
```python
login_url = f"https://sso.{client.domain}/sso/embed"
url = (
    f"{base_url}preauthorized?ticket={ticket}&login-url={login_url}"
    "&accepts-mfa-tokens=true"
)
```

**Impact:** ⚠️ **LIKELY OAUTH1 FAILURE** (Depends on API validation)

Garmin Connect API expects the login-url parameter. Without it:
- Token exchange may fail if API validates this parameter
- At minimum, it differs from the working Python implementation

**Fix Required:** Add this line:
```cpp
query.addQueryItem("login-url", ssoUrl() + SSO_EMBED_PATH);
```

**Complexity:** LOW - One line addition

---

### 5. Wrong Service Parameter in Login POST

**Location:** Line 196 in `performLogin()`

**Current C++ Code:**
```cpp
query.addQueryItem("service", connectApiUrl());  // WRONG!
```

**Expected (Python):**
```python
SIGNIN_PARAMS = {
    ...
    "service": SSO_EMBED,  # https://sso.{domain}/sso/embed
    ...
}
client.post("sso", "/sso/signin", params=SIGNIN_PARAMS, ...)
```

**Impact:** ⚠️ **LOGIN FAILURE**

The service parameter determines where the user is redirected after successful login:
- **C++ sends:** `https://connectapi.{domain}` (WRONG)
- **Should send:** `https://sso.{domain}/sso/embed` (CORRECT)

Garmin's server may reject the request or cause authentication to fail.

**Fix Required:** Change line 196 from:
```cpp
query.addQueryItem("service", connectApiUrl());
```
to:
```cpp
query.addQueryItem("service", ssoEmbedUrl);
```

**Complexity:** LOW - One line change

---

## HIGH PRIORITY ISSUES

### 6. MFA Endpoint Missing Query Parameters

**Location:** Line 361 in `performMfaVerification()`

**Current C++ Code:**
```cpp
QUrl url(ssoUrl() + "/sso/verifyMFA/loginEnterMfaCode");
// No query parameters added!
```

**Expected (Python):**
```python
client.post(
    "sso",
    "/sso/verifyMFA/loginEnterMfaCode",
    params=signin_params,  # These query parameters are required!
    referrer=True,
    data={...},
)
```

**Impact:** ⚠️ **MFA VERIFICATION FAILURE**

The MFA endpoint requires the same query parameters as the signin endpoint:
- id
- embedWidget
- gauthHost
- service
- source
- redirectAfterAccountLoginUrl
- redirectAfterAccountCreationUrl

Without them, MFA verification will likely fail.

**Fix Required:** Add query parameters to MFA URL (copy from performLogin, lines 193-200)

**Complexity:** LOW - Copy-paste from existing code

---

## MEDIUM PRIORITY ISSUES

### 7. Unnecessary Content-Type Header on GET Request

**Location:** Line 471 in `exchangeForOAuth1Token()`

**Current C++ Code:**
```cpp
request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

QNetworkReply *reply = m_manager->get(request);  // GET request!
```

**Issue:** GET requests should not have Content-Type headers

**Impact:** ⚠️ MINOR - Likely no functional impact, but non-standard

**Fix Required:** Remove line 471

**Complexity:** LOW - One line removal

---

### 8. Missing Referer Header for CSRF Token Fetch

**Location:** Line 133 in `fetchCsrfToken()`

**Current C++ Code:**
```cpp
QNetworkRequest request(url);
request.setRawHeader("User-Agent", USER_AGENT);
// No Referer header
```

**Expected (Python):**
```python
client.get("sso", "/sso/signin", params=SIGNIN_PARAMS, referrer=True)
```

**Impact:** ⚠️ POTENTIAL FAILURE - If API validates Referer header

**Fix Required:** Add Referer header from previous /sso/embed response

**Complexity:** MEDIUM - Requires tracking previous response URL

---

## LOW PRIORITY ISSUES

### 9. Wrong User-Agent for Activity Upload

**Location:** Line 639 in `uploadActivity()`

**Current C++ Code:**
```cpp
request.setRawHeader("User-Agent", "GCM-iOS-5.7.2.1");  // iOS user-agent
```

**All other requests use:**
```cpp
request.setRawHeader("User-Agent", USER_AGENT);  // com.garmin.android.apps.connectmobile
```

**Impact:** ⚠️ UNCLEAR - May be intentional, may be wrong

**Fix Required:** Verify if upload endpoint requires iOS user-agent or should use standard Android user-agent

**Complexity:** LOW - One line change (if needed)

---

## SUMMARY TABLE

| Issue | Priority | Type | Lines | Impact | Complexity |
|-------|----------|------|-------|--------|------------|
| OAuth1 Signature Missing | CRITICAL | Implementation | 469-475, 514-521 | Auth fails | HIGH |
| OAuth2 Wrong POST Body | CRITICAL | Parameters | 514-515 | Auth fails | LOW |
| Response Format Mismatch | CRITICAL | Parsing | 489-495 | Token parse fails | MEDIUM |
| Missing login-url | CRITICAL | Parameters | 465-466 | Token exchange fails | LOW |
| Wrong service param (POST) | CRITICAL | Parameters | 196 | Login fails | LOW |
| MFA missing query params | HIGH | Parameters | 361 | MFA fails | LOW |
| Unnecessary Content-Type GET | MEDIUM | Headers | 471 | None (cosmetic) | LOW |
| Missing Referer header | MEDIUM | Headers | 133 | May fail | MEDIUM |
| Wrong upload User-Agent | LOW | Headers | 639 | Unknown | LOW |

---

## TESTING IMPACT

**Current Status:** ❌ **LOGIN WILL FAIL**

Due to the OAuth1 signature issue alone, authentication will fail. Even if other issues are fixed:

### Issue 1 (OAuth1 Signature) - Blocks Everything
- Without this fix, nothing works
- Requires cryptographic implementation

### Issues 2-5 (Without OAuth1 Signature Fixed)
- Issue 2: Will fail in OAuth2 exchange even if signature is implemented
- Issue 3: Will fail when trying to extract OAuth1 token
- Issues 4-5: Will fail in token exchange
- Issue 6: MFA users cannot login

### If Issues 1-5 Fixed (Even without 6-9)
- ✅ Users without MFA can login
- ❌ Users with MFA cannot login (issue 6)
- ⚠️ May have intermittent failures (issues 7-9 depend on API strictness)

### If All Issues Fixed
- ✅ Complete authentication should work
- ✅ MFA support works
- ✅ Token refresh works
- ✅ Activity upload works

---

## PRIORITY FIX ORDER

### Must Fix First (Login Won't Work Without These):
1. **Issue 1:** Implement OAuth1 signature (CRITICAL)
2. **Issue 3:** Fix response parsing to use URL-encoded format (CRITICAL)

### Must Fix Second (Will Fail After Issue 1):
3. **Issue 2:** Remove oauth_token/oauth_token_secret from POST body (CRITICAL)
4. **Issue 4:** Add login-url parameter (CRITICAL)
5. **Issue 5:** Fix service parameter to use ssoEmbedUrl (CRITICAL)

### Should Fix Before Release:
6. **Issue 6:** Add query parameters to MFA endpoint (HIGH)
7. **Issue 8:** Add Referer header to CSRF fetch (MEDIUM)

### Can Fix Later or Investigate:
8. **Issue 7:** Remove Content-Type from GET (MEDIUM)
9. **Issue 9:** Verify/fix upload User-Agent (LOW)

---

## QUICK REFERENCE - What Each Issue Blocks

| Issue | Blocks |
|-------|--------|
| OAuth1 Signature | Everything - No tokens can be obtained |
| OAuth2 Wrong Params | OAuth2 token exchange - Can't proceed past OAuth1 |
| Response Parsing | OAuth1 token extraction - Can't parse response |
| Missing login-url | Token exchange - May be rejected by API |
| Wrong service param | Login form submission - May fail to redirect |
| MFA missing params | MFA verification - Can't complete MFA |
| Content-Type GET | Nothing (cosmetic) |
| Missing Referer | Possibly CSRF fetch - May be rejected |
| Wrong Upload UA | Activity upload - May be rejected by upload API |

---

**Document Generated:** 2025-12-03
**Analysis Basis:** Line-by-line comparison with working Python garth library
**Recommendation:** Fix in priority order starting with OAuth1 signature implementation
