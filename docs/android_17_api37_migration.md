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

### Confirmed Android 17 real-device regression (October 30, 2026)

- QZ log `debug-Fri_Oct_30_08_11_07_2026.log` demonstrates that the
  bundled Qt 5.15 native Bluetooth backend **never starts its BLE scan**:
  `Requesting ACCESS_*_LOCATION permission` then
  `Search not possible due to missing permission (ACCESS_COARSE|FINE_LOCATION)`,
  followed by `BTLE scanning error 100`. This is NOT an advertisement filter
  or Echelon device-type issue; it occurs before any scan results.
- Upstream Qt 5.15 `src/bluetooth/qbluetoothdevicediscoveryagent_android.cpp`
  checks whether either `ACCESS_COARSE_LOCATION` or `ACCESS_FINE_LOCATION`
  is granted **for every Android SDK >= 23**, including API 37, and refuses to
  scan if both are denied. It also checks the system-wide Location toggle.
  Android 12+ itself does **not** require location for `BLUETOOTH_SCAN` when
  `neverForLocation` applies; this additional check is an old Qt constraint.
- **Compatibility workaround on this draft PR:** declare/request
  `ACCESS_COARSE_LOCATION` (approximate only) on Android 12+; keep
  `ACCESS_FINE_LOCATION` constrained to `maxSdkVersion=30` for Android
  11 and older. `LocationHelper` again checks the system-wide Location toggle
  using `LocationManager.isLocationEnabled()` on API 28+, matching Qt's
  native guard. Home.qml explains that the requirement comes from Qt.
- Keep `BLUETOOTH_SCAN` + `neverForLocation` and `BLUETOOTH_CONNECT` for
  modern Android. Google warns that `neverForLocation` may filter some BLE
  beacons; first validate that scanning starts, then check whether the Echelon
  EX-5S and proprietary BLE advertisements are visible. Do not remove the
  privacy assertion without evidence that it blocks devices.
- **Permanent resolution:** update/patch Qt Bluetooth's Android discovery
  native backend so API 31+ checks the Nearby devices permissions and does
  not require a location permission or Location toggle solely for Bluetooth
  scanning. The shipped `libQt5Bluetooth` is part of a custom 16K-page
  aligned Qt distribution, so this is NOT solved by editing an app Java file
  or the Android manifest alone; it requires a validated Qt library rebuild.
- `PathController` uses `QGeoPositionInfoSource::lastKnownPosition()` as
  an initial map center, while the GPX preview subsequently centers on the
  selected route. Review this separately from the Bluetooth location request.
- Google Play's January 2027 policy is particularly strict for **precise**
  location. Approximate permission avoids restoring unrestricted
  `ACCESS_FINE_LOCATION`, but it is still an unnecessary permission caused
  by outdated Qt, so investigate removal before release. Check the merged
  Play AAB manifest for any transitive precise location grant.

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
7. **Location privacy / Play policy:** Fine location remains limited to
   API 30 and earlier, but Qt 5.15 still requires an approximate location
   permission **and the Location toggle** on Android 12+ solely to start
   Bluetooth scanning. This is a draft-branch compatibility workaround, not
   the intended final permission model. Audit the merged AAB manifest and
   remove the Qt-specific check in a future rebuilt QtBluetooth library.

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

### Qt 5 androiddeployqt / AGP 9 manifest collision

- Qt 5.15's `<!-- %%INSERT_PERMISSIONS -->` expands to module-derived Android
  permissions, including an unrestricted `ACCESS_FINE_LOCATION` and `INTERNET`.
  When these duplicate QZ's explicit permissions, AGP 9 rejects the merged
  manifest (first observed in CI run 41006927150).
- QZ now omits that expansion marker and explicitly declares the legacy
  `BLUETOOTH` / `BLUETOOTH_ADMIN` permissions with `maxSdkVersion=30`, along
  with its network and legacy storage requirements. It retains
  `ACCESS_FINE_LOCATION` only through API 30, and relies on Nearby devices on
  modern Android.
- **Validation:** inspect the final APK/AAB merged manifest, not only the
  source XML. This replacement needs a real-device legacy-Bluetooth/storage
  regression check because Qt's inferred permission list is no longer used.

