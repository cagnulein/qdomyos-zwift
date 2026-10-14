# Temporary Zwift authentication diagnostics for Dirk (2026-10-13)

**Private diagnostic build / DO NOT MERGE into master.** The diagnostic code is enabled directly on this branch with `QZ_ZWIFT_AUTH_DIAGNOSTICS=1` in `src/zwift-api/zwift_client_auth.h`. It is intentionally not controlled by a user setting.

## Observed symptom

- iOS user, treadmill, Zwift API auto-inclination enabled.
- `AuthToken::handleTokenResponse` reports HTTP 400 Bad Request from `https://secure.zwift.com/auth/realms/zwift/tokens/access/codes`.
- Re-entering a changed Zwift password did not resolve the problem.
- The QZ incline calculation code cannot execute without a successful access token and player status.
- Original QZ request construction is **unchanged** here. In particular, the password grant percent-encodes `username` before passing it to `QUrlQuery`; this is worth verifying using the one-pass wire decoding diagnostics.

## How to collect one useful log

1. Install a diagnostic iOS build produced from **this branch**, not an App Store release. Ensure the testing build uses the new branch commit.
2. Keep the working Zwift credentials set in QZ. Close and reopen QZ after any credential change to ensure the authentication object reads the latest values.
3. Enable **Debug Log** in **Experimental Settings** and enter a destination email under **General Options**.
4. Connect the treadmill, start QZ and your normal Zwift session with an incline change, then press **Stop** in QZ and forward the resulting log to the developer.
5. Look for log lines starting `[DIRK-ZWIFT-AUTH]`.

## What the instrumentation records

| Stage | Safe diagnostic values |
| --- | --- |
| Request construction | password/refresh grant, method, endpoint without query string, lengths, leading/trailing whitespace and non-ASCII indicators, percent/form special-character flags |
| Wire encoding | encoded field sizes, whether username/password decode correctly after one pass, doubled-encoded `@` marker |
| Network | HTTP code/reason, Qt network error, duration, content type, selected correlation headers, TLS cipher and protocol, SSL errors, sanitized redirect |
| Server error | JSON `error`, `error_description`, `message`, `status`, `code` (sanitized) |
| Token result | token presence, length, short SHA-256 fingerprint, `expires_in`, `refresh_expires_in`, `token_type` |
| Incline gating | changes to device eligibility, auth-object presence, access-token presence/validity, world object/player ID and auto-inclination setting |

**Never log**: raw username, password, access/refresh/ID token, complete form body, Authorization header, cookies, unfiltered server body or token-bearing query URLs. Even with these safeguards, debug logs can contain unrelated personal data from standard QZ logging; treat attachments as private.

## Interpretation

- HTTP 400 + `invalid_grant`: inspect credential correctness and wire encoding, as well as account-specific authentication requirements.
- One-pass username mismatch or double-encoded `@`: investigate `QUrl::toPercentEncoding(username)` followed by `QUrlQuery::toString(QUrl::FullyEncoded)` without changing production behavior prematurely.
- HTTP 200 + missing token: inspect token JSON structure or a server-side contract change.
- HTTP 200 + token present, but auto-incline gate blocked: inspect the post-auth world/player state.
- Success at each auth step but no slope: investigate player status and device-side incline requests separately.

**Cleanup:** after reproducing and diagnosing this case, remove the temporary logging and the macro before merging any production change. This is not a proposed permanent production feature.
