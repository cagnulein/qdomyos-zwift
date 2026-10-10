# Peloton login on older Android (ISRG root certificates)

Some NordicTrack consoles on Android 7.0 do not trust Let's Encrypt's ISRG Root X1.
A QZ user with a 2021 NordicTrack Commercial 1750 reported WebView
`ERR_CERT_AUTHORITY_INVALID (-202)` failures during Peloton OAuth login.
Bundling ISRG Root X1 and X2 in a repackaged QZ 2.21.5 APK fixed login on that
console. The original report observed roughly 25 certificate errors before the
change and none afterwards; Peloton tokens were saved successfully.

The production source implementation is deliberately **narrower** than that
test APK: `src/android/res/xml/network_security_config.xml` adds those
public roots **only** for `assets.onepeloton.com` and
`images.onepeloton.com`. These were the two suspected Let's Encrypt hosts
in the reporter's investigation; the console logs did **not** associate each
failed certificate request with its host. All other destinations retain the
Android system trust store. HTTPS verification remains enabled, and these two
Peloton domains do not allow cleartext traffic.

The global `base-config` keeps cleartext enabled for QZ's existing local
device integrations, matching the prior `usesCleartextTraffic="true"`
manifest setting. Android otherwise ignores that manifest flag after
`networkSecurityConfig` is supplied.

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
3. On Android 14+ and other modern devices, verify Peloton OAuth login and
   routine HTTPS API calls still work.
4. Confirm QZ can still access localhost and local network equipment via
   plain HTTP (where the integrations require it).
5. Verify that non-Peloton hosts still use system trust anchors.

Credit: Jonathan Kaplan (endlessthinker@gmail.com), for the original
NordicTrack Android 7.0 investigation, patched APK and change record.
