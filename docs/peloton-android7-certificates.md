# Peloton login on older Android (ISRG root certificates)

Some NordicTrack consoles on Android 7.0 do not trust Let's Encrypt's ISRG Root X1.
A QZ user with a 2021 NordicTrack Commercial 1750 reported WebView
`ERR_CERT_AUTHORITY_INVALID (-202)` failures during Peloton OAuth login.
Bundling ISRG Root X1 and X2 in a repackaged QZ 2.21.5 APK fixed login on that
console. The original report observed roughly 25 certificate errors before the
change and none afterwards; Peloton tokens were saved successfully.

The production implementation is deliberately **narrower** than the test APK,
both by Android version and by domain:

- **Android 7.0–7.1 (API 24–25):**
  `src/android/res/xml/network_security_config.xml` adds ISRG Root X1/X2
  only for `assets.onepeloton.com` and `images.onepeloton.com`.
- **Android 8.0+ (API 26+):**
  `src/android/res/xml-v26/network_security_config.xml` trusts only system
  certificates for those domains (and everywhere else), without loading
  the extra ISRG roots. The Android resource version qualifier selects this
  file automatically, without runtime branching or additional permissions.
- **Android 6.0 and earlier (API <=23):** Android does not support Network
  Security Configuration. The packaged certificate files cannot change the
  system trust store there.

Those two Peloton hosts were suspected in the reporter's investigation; the
console logs did **not** associate each failed certificate request with its
hostname. HTTPS certificate validation remains enabled, and both XML variants
disallow cleartext for the two Peloton domains. All other destinations retain
the Android system trust store. Let's Encrypt reports Android 7.1.1+ normally
already trusts ISRG Root X1; API 25 is retained in the workaround to cover
earlier 7.1 builds and vendor-modified device images.

Both XML variants keep cleartext enabled in `base-config` for QZ's existing
local-device integrations, matching the prior `usesCleartextTraffic="true"`
manifest setting. On Android 7.0+, `networkSecurityConfig` takes precedence
over that manifest flag.

Bundled root certificates:
- ISRG Root X1 SHA-256:
  `96:BC:EC:06:26:49:76:F3:74:60:77:9A:CF:28:C5:A7:CF:E8:A3:C0:AA:E1:1A:8F:FC:EE:05:C0:BD:DF:08:C6`
- ISRG Root X2 SHA-256:
  `69:72:9B:8E:15:A8:6E:FC:17:7A:57:AF:B7:17:1D:FC:64:AD:D2:8C:2F:CA:8C:F1:50:7E:34:45:3C:CB:14:70`

Both certificates are published by Let's Encrypt:
https://letsencrypt.org/certificates/

## Manual regression checklist

1. On Android 7.0 (2021 NordicTrack 1750), open QZ Peloton login. Verify
   OAuth page assets load without certificate errors, and an authorized
   login saves access and refresh tokens.
2. If login still fails, log the exact failing WebView request hostname
   before expanding the allowlist; do not bypass SSL errors.
3. Verify resource selection in an Android build: API 24/25 selects
   `res/xml/network_security_config.xml` (Peloton roots added), while API
   26+ selects `res/xml-v26/network_security_config.xml` (system roots only).
4. On Android 8+ and Android 14+, verify Peloton OAuth login, routine HTTPS
   API calls, and system-only trust anchors.
5. Confirm QZ can still access localhost and local network equipment via
   plain HTTP (where the integrations require it).
6. Verify that non-Peloton hosts still use system trust anchors on Android 7.

Credit: Jonathan Kaplan (endlessthinker@gmail.com), for the original
NordicTrack Android 7.0 investigation, patched APK and change record.
