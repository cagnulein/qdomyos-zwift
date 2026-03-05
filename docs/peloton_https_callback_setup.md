# Peloton OAuth HTTPS callback setup (robertoviola.cloud)

This document describes the mobile-side setup for migrating Peloton OAuth callback URLs to HTTPS with verified deep links.

## Redirect URL to register with Peloton

Use the following callback URL in Peloton OAuth client configuration:

- `https://robertoviola.cloud/peloton/callback`

## iOS (Universal Links)

The iOS app enables Associated Domains with:

- `applinks:robertoviola.cloud`

The domain must serve an `apple-app-site-association` file over HTTPS at one of:

- `https://robertoviola.cloud/apple-app-site-association`
- `https://robertoviola.cloud/.well-known/apple-app-site-association`

Example content:

```json
{
  "applinks": {
    "apps": [],
    "details": [
      {
        "appID": "<APPLE_TEAM_ID>.org.cagnulein.qdomyoszwift",
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

- host: `robertoviola.cloud`
- path prefix: `/peloton/callback`

The domain must serve this file exactly:

- `https://robertoviola.cloud/.well-known/assetlinks.json`

Example content (replace certificate fingerprint):

```json
[
  {
    "relation": ["delegate_permission/common.handle_all_urls"],
    "target": {
      "namespace": "android_app",
      "package_name": "org.cagnulen.qdomyoszwift",
      "sha256_cert_fingerprints": [
        "<RELEASE_KEY_SHA256_FINGERPRINT>"
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

The app now handles `https://robertoviola.cloud/peloton/callback` directly on mobile:

- Android: `CustomQtActivity` forwards incoming App Link intents to C++ (`nativeOnOAuthCallback`).
- iOS: the Qt app event filter intercepts `QEvent::FileOpen` URL events and forwards them to OAuth handling.
- C++: Peloton callback URL parsing/exchange is handled in `peloton::handleOAuthCallbackUrl`.
