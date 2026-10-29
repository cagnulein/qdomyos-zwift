# Android 17 (target SDK 37) migration checklist

This branch changes the **Google Play Android** build to target/compile SDK 37. It should
remain a **draft** until the Android CI and the device-level regressions below have
been exercised. This is not a Play Console permission declaration.

## Local network permission

- `src/android/AndroidManifest.xml` declares
  `android.permission.ACCESS_LOCAL_NETWORK`.
- `src/main.cpp` requests it at runtime on **Android 17+ only**, after the existing
  Bluetooth Nearby devices requests and **before** QZ starts its device managers.
  Older Android versions continue using `INTERNET` without a new prompt.
- It belongs to Android's `NEARBY_DEVICES` permission group. A Bluetooth grant
  can pregrant local-network access, so test both fresh installs and pregranted
  scenarios.
- If denied or subsequently revoked, QZ must continue working for pure BLE
  devices, but local TCP/UDP/mDNS functionality is unavailable. Current startup
  logs explain the denial; validate a user-facing recovery path and permission
  settings access before considering production release.

Broad permission is justified by QZ's direct, sometimes concurrent LAN traffic:
DIRCON TCP server and mDNS advertisement (`src/devices/dircon/dirconprocessor.cpp`),
MyWhoosh OpenBikeControl TCP/UDP and service advertisement
(`src/mywhooshlink.cpp`), machine-specific direct IP/Wi-Fi connections, and
UDP protocols such as SmartSpin2K. A system discovery picker cannot substitute
for all of these, notably unsolicited inbound traffic and service publishing.

## Build-system changes

- Use AGP **9.1.1** (minimum supported for API 37), Gradle **9.3.1**,
  AGP built-in Kotlin, protobuf plugin **0.10.0** (supports AGP 9).
- `src/android/gradle.properties` sets compile SDK 37, matching
  `src/android/build.gradle` and the app manifest target SDK 37.
- Android CI, the Play Store release workflow, and other Android variants install
  platform 37 and use it with `androiddeployqt`. API 37 emulator testing is
  added as initially non-blocking because the image/Qt integration must be confirmed.
- The legacy Qt **5.15** Android deployment and plugin upgrade could reveal
  new build issues: check generated manifest, Gradle and native library packaging
  and ensure the Play release AAB actually targets SDK 37.

## Position permission / Bluetooth discovery

- Android 12+ (API 31+) uses `BLUETOOTH_SCAN` (Nearby devices) with
  `neverForLocation`; `ACCESS_FINE_LOCATION` in the manifest has
  `maxSdkVersion=30` and the runtime request is limited to API <= 30.
- `LocationHelper` no longer demands the device-wide Location toggle on Android
  12+. Older versions still need the legacy location flow to discover Bluetooth.
  Home.qml explains the two cases.
- **Regression caveat:** Android warns that `neverForLocation` can filter some
  BLE beacons. Validate device discovery against representative FTMS devices and
  proprietary advertisements (including devices not broadcasting standard FTMS).
- `PathController` uses `QGeoPositionInfoSource::lastKnownPosition()` as an
  initial map center, but the GPX preview sets its center to the route's center
  in `homeform::gpxpreview_open_clicked()`. Review this fallback with location
  denied; avoid silently assuming there is no location-related source code.
- Play's January 27, 2027 location policy means dropping precise location on
  modern Android avoids an otherwise difficult-to-justify declaration. Inspect
  the final merged AAB manifest in case dependency manifests re-add location.

## Background TTS / Android 17 audio hardening

- `homeform` uses `QTextToSpeech::say()` to announce workout values and commands
  while the app may be backgrounded.
- QZ starts a `connectedDevice` foreground service via
  `NotificationClient.notify()`, when `android_notification_v2` is enabled.
  The setting is **on by default**, but existing users can disable it.
- Android 17 requires an active foreground service **with while-in-use (WIU)
  capability** for audio playback, audio focus or volume changes in background.
  Merely declaring or creating a service does not suffice. Services launched
  while QZ is visible (or from a qualified user action) receive WIU; starting
  from an ordinary background context generally does not.
- Background TTS can be **silenced without error** when the notification/FGS is
  disabled, stopped or loses WIU. Volume-button gear control also calls
  `AudioManager.setStreamVolume()` in `MediaButtonReceiver`, which may silently
  fail under the same restrictions.
- Avoid starting a foreground service against an explicit user preference.
  Document that background voice prompts require QZ's Android notification,
  and test grant/disable/re-enable scenarios. If needed, add an in-app warning
  or user-initiated remediation in a separate UX change.
- Manual ADB check on an Android 17 image with a TTS engine configured:
  `adb shell cmd audio set-enable-hardening throw`, start a workout with TTS
  active, background QZ, inspect `adb logcat -d | grep AudioHardening`, then
  repeat with the Android notification disabled. Verify TTS can still be heard
  with the service running and that volume-button gear changes restore volume.
  `throw` makes violations observable instead of silent.
- Source: https://developer.android.com/about/versions/17/changes/bg-audio

## Android 17 changes beyond LAN: review before release

1. **Bluetooth Classic RFCOMM:** `BluetoothSocket` InputStream `read()` now
   returns -1 at EOF. Exercise Kettler Classic, iConsole, iConcept and other
   QBluetoothSocket-based devices, especially disconnect/reconnect. Qt 5's
   Android Bluetooth backend may need fixes.
2. **Background audio:** background playback, focus and volume APIs have tighter
   requirements for target 37. Verify QZ voice/TTS prompts, accessibility,
   workouts in background and the connected-device foreground service.
3. **Native loading:** JNI `System.load()` requires read-only native libraries.
   Verify the Qt 5 native library loader and any dynamically loaded .so files.
4. **Large screens:** Android 17 ignores requested orientation/resizability
   restrictions on screens of at least 600dp. Test tablets and landscape,
   especially Qt Quick overlays, floating windows and back navigation.
5. **Certificate transparency:** CT is on by default for target 37; test any
   HTTP clients using platform TLS/WebView and connections to local equipment.
6. **Static final / MessageQueue:** reflective modifications to static finals
   fail, and a new MessageQueue implementation is used by target 37. Smoke-test
   Qt Java glue, accessibility, activity lifecycle and plugin integrations.
7. **Location privacy / Play policy:** QZ now limits `ACCESS_FINE_LOCATION` to
   API 30 and earlier; audit the merged manifest and confirm BLE discovery on
   Android 12+ with `neverForLocation` (some beacon advertisements are filtered).

## Required validation

- CI Android build for all ABIs, API 37 emulator smoke and Play release AAB.
- Fresh Android 17 install: grant / deny local-network permission; reboot app;
  revoke it from system settings while running and verify BLE still works.
- DIRCON with Zwift and Rouvy, MyWhoosh TCP/UDP/mDNS, IP-connected treadmills,
  SmartSpin2K and other LAN fitness equipment. Check discovery, connect,
  telemetry, control, reconnection, and local broadcast/advertisement.
- BLE FTMS scan, pairing, auto-resistance, disconnect/reconnect, and classic
  Bluetooth RFCOMM on Android 17, plus at least one Android 15/16 regression.
- Background workout, notifications, TTS/audio (on Android 17, verify FGS is
  active/WIU; include denial, disable and reboot scenarios), screenshots/overlays, Health
  Connect, Garmin integrations and Play Billing.
- Tablet portrait/landscape and Qt 5 bootstrap / native library loading.
- Play Console: submit local-network permission declaration with concrete
  evidence that pickers cannot replace QZ direct socket communication before
  production release.

References:
- https://developer.android.com/about/versions/17/behavior-changes-17
- https://developer.android.com/privacy-and-security/local-network-permission
- https://developer.android.com/about/versions/17/setup-sdk
- https://support.google.com/googleplay/android-developer/answer/18100766
