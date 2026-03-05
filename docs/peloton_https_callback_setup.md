# Peloton OAuth HTTPS callback setup (www.qzfitness.com)

This document describes the mobile-side setup for migrating Peloton OAuth callback URLs to HTTPS with verified deep links.

## Redirect URL to register with Peloton

Use the following callback URL in Peloton OAuth client configuration:

- `https://www.qzfitness.com/peloton/callback`

## iOS (Universal Links)

The iOS app enables Associated Domains with:

- `applinks:www.qzfitness.com`

The domain must serve an `apple-app-site-association` file over HTTPS at one of:

- `https://www.qzfitness.com/apple-app-site-association`
- `https://www.qzfitness.com/.well-known/apple-app-site-association`

Example content:

```json
{
  "applinks": {
    "apps": [],
    "details": [
      {
        "appID": "6335M7T29D.org.cagnulein.qdomyoszwift",
        "paths": [
          "/peloton/callback",
          "/peloton/callback/*"
        ]
      }
    ]
  }
}
```

## Android (App Links)

The Android manifest declares an HTTPS App Link for:

- host: `www.qzfitness.com`
- path prefix: `/peloton/callback`

The domain must serve this file exactly:

- `https://www.qzfitness.com/.well-known/assetlinks.json`

Example content (replace certificate fingerprint):

```json
[
  {
    "relation": ["delegate_permission/common.handle_all_urls"],
    "target": {
      "namespace": "android_app",
        "package_name": "org.cagnulen.qdomyoszwift",
        "sha256_cert_fingerprints": [
          "FB:87:F4:97:72:0C:07:D4:04:7D:CB:A0:CA:DB:00:35:F8:C6:B0:4C:7D:8D:99:ED:20:2B:78:1E:5B:11:D6:B7"
        ]
    }
  }
]
```

## WordPress hosting notes

On WordPress/VPS, ensure both files are publicly served as raw JSON with no redirects:

- `/.well-known/assetlinks.json`
- `/apple-app-site-association` (or `/.well-known/apple-app-site-association`)

Recommended approach is to place physical files in webroot and configure web server rewrite exceptions so WordPress does not intercept these paths.

## In-app callback handling

The app now handles `https://www.qzfitness.com/peloton/callback` directly on mobile:

- Android: `CustomQtActivity` forwards incoming App Link intents to C++ (`nativeOnOAuthCallback`).
- iOS: the Qt app event filter intercepts `QEvent::FileOpen` URL events and forwards them to OAuth handling.
- C++: Peloton callback URL parsing/exchange is handled in `peloton::handleOAuthCallbackUrl`.
