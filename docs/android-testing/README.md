# Android Testing Documentation

This directory contains procedures for testing QZ (qdomyos-zwift) with Android emulators and physical devices on the local network.

## Structure

- [qz.md](qz.md) - Installing and testing QZ on an emulator or device
- [zwift.md](zwift.md) - Testing the QZ <-> Zwift integration through Android notifications, without Bluetooth
- [automation.md](automation.md) - UI Automator and adb scripts for automated tests

## Current Status (2026-06-21)

| App | `Pixel_8_API_36` Emulator | Pixel 8a (192.168.0.35) | Notes |
|---|---|---|---|
| QZ v2.21.5 | Installed and configured | Installed and configured | Fake Device and Android Notification enabled |
| Zwift | Not installed | Installed and logged in | Full BLE pairing confirmed: "Wahoo KICKR 0000" |

## PR #4645 Test Results (2026-06-21, Pixel 8a)

Branch: `codex/fix-voiceover-accessibility-issues`

### Test 1 — QZ ↔ Zwift BLE integration ✅

- QZ configured with `applewatch_fakedevice=true` and `tile_target_power_enabled=true`
- Zwift paired to "Wahoo KICKR 0000" virtual BLE device under TUTTI filter
- Live power verified: 200W target in QZ → 167–213W received in Zwift POTENZA tile

Screenshots:
- [`pr4645_zwift_connected.png`](pr4645_zwift_connected.png) — Zwift "DISPOSITIVI ABBINATI" showing Wahoo KICKR 0000 CONNESSO at 167W

### Test 2 — `drawerInputBlocker` back gesture (PR-specific) ✅

The `drawerInputBlocker` MouseArea (`z: 9999`) in `main.qml` was verified to **not** intercept the native Android back gesture.

Procedure:
1. Opened QZ drawer (tap hamburger at top-left)
2. Performed Android back gesture: `adb shell input swipe 1050 1200 50 1200 300`
3. Drawer closed — `drawerInputBlocker` did NOT consume the gesture
4. QZ remained running (brought back with `am start`, dashboard visible)

Screenshots:
- [`pr4645_drawer_open.png`](pr4645_drawer_open.png) — QZ drawer fully open
- [`pr4645_drawer_closed_after_back_gesture.png`](pr4645_drawer_closed_after_back_gesture.png) — QZ dashboard after back gesture, drawer closed

### Bug Found During Testing — drawerInputBlocker blocked drawer item taps ⚠️ (Fixed)

During APK testing on Pixel 8a, discovered that `adb input tap` on drawer items below the first item ("Profile: default") had no effect. Root cause: `drawerInputBlocker` MouseArea used `anchors.fill: parent` and covered the full content area (y > toolbar), intercepting all taps before they reached the Drawer's QML overlay. Only the first drawer item (in the toolbar overlap region) was reachable.

**Fixed in commit `9791d57`:** constrained the MouseArea to `x: drawer.width`, `width: parent.width - drawer.width` so taps on the drawer area pass through to the drawer's own event handlers.

**Root cause follow-up (fixed in commit `10c87e2`):** even after restricting `drawerInputBlocker`, drawer items in the ApplicationWindow *content area* (y > toolbar boundary ≈ 246 physical px) were still unreachable. Investigation showed the boundary is exact: tapping within the Profile item at y=240 worked; tapping at y=250 (start of the Settings item, first fully-content-area item) did not. Root cause: `dim: false` on the Qt Quick `Drawer` prevents the overlay layer from placing an event-interception item in the content area, so Android touch events at y > toolbar fall through the Drawer popup entirely and hit disabled dashboard controls (no visible effect). Fix: `dim: true` + `Overlay.modal: Rectangle { color: "transparent" }` — the overlay is placed and routes events to Drawer ItemDelegates, but remains visually invisible.

### Known Issue — Settings Tile Toggles Have No content-desc ⚠️

QML settings toggles (`AccordionCheckElement` in `settings-tiles.qml`, `IndicatorOnlySwitch` in `settings.qml`) do not expose `Accessible.name` to Android, so uiautomator shows no `content-desc` for them and TalkBack cannot announce their label.

**Fixed in this PR:** `AccordionCheckElement.qml` and `IndicatorOnlySwitch.qml` now set:
```qml
Accessible.name: <title or text property>
Accessible.role: Accessible.CheckBox
Accessible.checkState: Qt.Checked / Qt.Unchecked
```

Workaround (before fix): edit the config file directly via `run-as` — see [automation.md](automation.md).

## Initial Setup

### Local Emulator (Android Studio)

Available AVD: `Pixel_8_API_36`

Start it from the command line:
```bash
~/Library/Android/sdk/emulator/emulator -avd Pixel_8_API_36 -no-snapshot-load -gpu host
```

The `-gpu host` flag is required for Zwift testing. With the default `auto` GPU mode, the emulator may expose SwiftShader software rendering to Android, which is not enough for the Zwift app.

Wait until the emulator is ready:
```bash
~/Library/Android/sdk/platform-tools/adb wait-for-device shell getprop sys.boot_completed
# returns "1" when it is ready
```

Verify that the emulator is using host GPU acceleration:
```bash
~/Library/Android/sdk/platform-tools/adb shell cmd gpu vkjson \
  | grep -E '"deviceName"|"driverName"|SwiftShader'
```

For the tested macOS host, the expected renderer was `MoltenVK` on `Apple M2 Pro`. If the output mentions `SwiftShader`, the emulator is still using software rendering.

### Physical Devices on the Network

Connect through ADB over Wi-Fi:
```bash
adb connect <DEVICE_IP>:5555
adb devices
```

To enable wireless debugging on the device (Android 11+):
```text
Settings -> Developer options -> Wireless debugging -> Pair device with pairing code
```

## Password Notes

Account credentials (Google Play, Zwift, and similar services) should stay in the device password manager. Never store passwords in these files.
