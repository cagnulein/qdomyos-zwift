# Testing the QZ <-> Zwift Integration on Android

## Quick Reference — Pixel 8a (192.168.0.35)

Tested 2026-06-21 with QZ 2.21.5 and Zwift on Pixel 8a. Everything below is confirmed working.

### ADB connection

ADB port changes after every wireless debug session. Last known port: `45407`.

```bash
ADB="~/Library/Android/sdk/platform-tools/adb -s 192.168.0.35:45407"

# If that port is stale, check mDNS first:
~/Library/Android/sdk/platform-tools/adb mdns services
# Look for _adb-tls-connect._tcp. entries for 192.168.0.35

# If nothing, scan in batches:
for port in $(seq 44000 1000 51000); do
  nc -w 1 192.168.0.35 $port </dev/null 2>/dev/null && echo "open: $port"
done
```

### Zwift package on Pixel 8a

On this device `com.zwift.android.prod` is the **Zwift game** (not Companion).
Zwift Companion is a separate package not installed here.

```bash
$ADB shell monkey -p com.zwift.android.prod 1   # launch Zwift
```

### Screen orientation

Zwift runs in **landscape** on the Pixel 8a: the resolution is **2400×1080**.
QZ runs in **portrait**: **1080×2400**.
Always check orientation before using hardcoded coordinates.

### Zwift BLE pairing coordinates (landscape 2400×1080)

| Step | Tap coordinates | Notes |
|---|---|---|
| Login `>` button | `(1655, 629)` | After the Zwift home screen appears |
| POTENZA → Cerca | `(900, 650)` | On the DISPOSITIVI ABBINATI screen |
| Switch to TUTTI filter | `(1450, 200)` | "Wahoo KICKR 0000" only appears here, not under CONSIGLIATI |
| Select Wahoo KICKR 0000 | appears after ~15 s | Tap the device row |
| CHIUDI | `(1200, 950)` | Closes the pairing modal |

### One-time dialogs on first launch (portrait 1080×2400)

| Dialog | Tap | Coordinates |
|---|---|---|
| Health Connect permission | "Don't allow" | `(270, 2217)` |
| 16KB compatibility warning | "Don't Show Again" | `(810, 2245)` |

### Live power verification

With QZ running (`applewatch_fakedevice=true`, `tile_target_power_enabled=true`):

To set T.Power from adb — scroll QZ dashboard down then use exact uiautomator bounds:
```bash
# Find current bounds (scroll position changes them):
$ADB shell uiautomator dump /sdcard/ui.xml
$ADB pull /sdcard/ui.xml /tmp/ui.xml
grep -o '"Increase T.Power.*bounds="[^"]*"' /tmp/ui.xml

# Tap + 20 times to reach ~200W (confirmed bounds 2026-06-21):
for i in $(seq 1 20); do $ADB shell input tap 918 1716; sleep 0.25; done
```

- Each tap adds ~10W; Zwift POTENZA tile updates in real time
- **Confirmed 2026-06-21:** QZ T.Power=200W → Zwift shows **183W POTENZA + 61 rpm CADENZA** live

### Zwift account

Account "aaa ssss" has no active Zwift subscription. The pairing screen and live power readout work, but you cannot start a ride. This is enough for sensor validation tests.

---

## Zwift App Architecture (Pixel 8a)

There are **two separate packages** installed:

| Package | Role | How to launch |
|---|---|---|
| `com.zwift.android.prod` | Zwift Home app (portrait, web-like UI) | `monkey -p com.zwift.android.prod 1` |
| `com.zwift.zwiftgame` | Zwift game/engine (landscape, Unity) | `am start com.zwift.zwiftgame/.ZwiftMainActivity` |

The **DISPOSITIVI ABBINATI** (sensor pairing) screen is only accessible inside `com.zwift.zwiftgame`. From the Zwift Home app there is no direct route to the pairing screen without starting a ride (which requires a subscription). Launch the game activity directly instead:

```bash
$ADB shell am start com.zwift.zwiftgame/.ZwiftMainActivity
```

If the game has never been launched in this session, it shows the **login screen** first. Tap the `>` button at **(1655, 629)** in landscape 2400×1080 to proceed to the pairing screen.

If the game was already running, it brings the DISPOSITIVI ABBINATI screen back to the foreground immediately.

---

## Pairing All Sensors — Pixel 8a (2400×1080 landscape)

Tile centers found via Pillow pixel analysis on the orange tiles:

| Sensor | Tile center (device px) |
|---|---|
| POTENZA | `(620, 337)` |
| RESISTENZA | `(1424, 337)` |
| CADENZA | `(659, 689)` |
| CONTROLLER | `(1200, 689)` |
| FREQUENZA CARDIACA | `(1739, 689)` |

### ABBINA DISPOSITIVI modal (per sensor)

After tapping a tile, the search modal opens:

| Action | Coordinates |
|---|---|
| Select "Wahoo KICKR 0000" row | `(1200, 357)` |
| CHIUDI (confirm/close) | `(1199, 952)` |

"Wahoo KICKR 0000" appears under **CONSIGLIATI** for POTENZA, RESISTENZA, CADENZA (no need to switch to TUTTI).

### FREQUENZA CARDIACA — Consent Dialog Limitation ⚠️

When pairing a heart rate sensor for the first time, Zwift shows a **"CONSENSO DATI SUI SENSORI"** dialog with NON ADESSO and ACCONSENTO buttons. This dialog is rendered inside Unity's `SurfaceView` and its buttons **do not respond to `adb shell input tap`** synthetic events — not even after delays or raw event attempts.

**Workaround:** tap **ACCONSENTO** physically on the device screen (once accepted, it is remembered and the dialog will not appear again).

Screenshots:
- [`zwift_pairing_3sensors.png`](zwift_pairing_3sensors.png) — 3 sensors paired, QZ not yet sending data
- [`zwift_pairing_live_data.png`](zwift_pairing_live_data.png) — **live data confirmed: POTENZA 183W, CADENZA 61 rpm** from QZ T.Power=200W target
- [`qz_tpower_200w.png`](qz_tpower_200w.png) — QZ dashboard showing T.Power=200W, FTP Zone Z4.9, timer running

### Sequence to pair all sensors from scratch

```bash
ADB="~/Library/Android/sdk/platform-tools/adb -s 192.168.0.35:45407"

# 1. Ensure QZ is running and advertising BLE
$ADB shell am start org.cagnulen.qdomyoszwift/.CustomQtActivity
sleep 8

# 2. Launch Zwift game (brings DISPOSITIVI ABBINATI to front)
$ADB shell am start com.zwift.zwiftgame/.ZwiftMainActivity
sleep 5

# 3. If the login screen appears, tap >
$ADB shell input tap 1655 629
sleep 12   # wait for game to load to pairing screen

# 4. Pair POTENZA
$ADB shell input tap 620 337   # open POTENZA modal
sleep 3
$ADB shell input tap 1200 357  # select Wahoo KICKR 0000
sleep 2
$ADB shell input tap 1199 952  # CHIUDI
sleep 2

# 5. Pair RESISTENZA
$ADB shell input tap 1424 337
sleep 3
$ADB shell input tap 1200 357
sleep 2
$ADB shell input tap 1199 952
sleep 2

# 6. Pair CADENZA
$ADB shell input tap 659 689
sleep 3
$ADB shell input tap 1200 357
sleep 2
$ADB shell input tap 1199 952
sleep 2

# 7. FREQUENZA CARDIACA — requires physical tap on ACCONSENTO button (once only)
# After first consent, use the same flow: tap (1739,689) -> (1200,357) -> (1199,952)
```

## Overview

On Android, Zwift can receive QZ data **without Bluetooth** by using the Android notification mechanism. QZ publishes sensor data, such as power, cadence, and heart rate, through a system notification. Zwift reads that notification as if it were a BLE sensor.

This also works when both apps run on the **same device**, either an emulator or a physical device.

## Prerequisites

### QZ Configured

1. `android_notification = true` enables the foreground service that publishes the sensor notification.
2. `applewatch_fakedevice = true` simulates a bike when no real bike is connected.

Quick setup while QZ is stopped:
```bash
ADB=~/Library/Android/sdk/platform-tools/adb
$ADB shell am force-stop org.cagnulen.qdomyoszwift
$ADB shell "run-as org.cagnulen.qdomyoszwift sh -c '
  mkdir -p \"/data/data/org.cagnulen.qdomyoszwift/files/.config/Roberto Viola\"
  printf \"[General]\nandroid_notification=true\napplewatch_fakedevice=true\n\" \
    > \"/data/data/org.cagnulen.qdomyoszwift/files/.config/Roberto Viola/qDomyos-Zwift.conf\"
'"
$ADB shell am start -n org.cagnulen.qdomyoszwift/.CustomQtActivity
sleep 5
```

Verify that the foreground notification is active:
```bash
$ADB shell dumpsys notification | grep -A3 "qdomyos"
# It should show ForegroundServiceChannel with numPostedByApp=1.
```

### Zwift Installed

```bash
# Check which Zwift packages are installed.
$ADB shell pm list packages | grep zwift

# Important:
# package:com.zwift.android.prod is Zwift Companion, not the Zwift game.
# Companion is not valid for this test.

# Install from the Play Store. A Zwift account is required.
# Credentials should stay in the device password manager.
```

Set the package name for the Zwift game after installing it:
```bash
ZWIFT_PACKAGE=<ZWIFT_GAME_PACKAGE>

# Do not use Companion here.
test "$ZWIFT_PACKAGE" != "com.zwift.android.prod"
```

If only `com.zwift.android.prod` appears in `pm list packages`, the emulator only has Zwift Companion installed and this test cannot continue yet.

### Emulator GPU Acceleration

Start the emulator with host GPU acceleration:
```bash
~/Library/Android/sdk/emulator/emulator -avd Pixel_8_API_36 -no-snapshot-load -gpu host
```

Verify the renderer inside Android:
```bash
$ADB shell cmd gpu vkjson | grep -E '"deviceName"|"driverName"|SwiftShader'
```

The tested macOS host reported `driverName=MoltenVK` and `deviceName=Apple M2 Pro` when host GPU acceleration was active. If the output mentions `SwiftShader`, Android is still using software rendering and Zwift is likely to fail.

## Test Procedure

### 1. Start QZ First

QZ must be in the foreground and have the notification active before Zwift starts.

```bash
$ADB shell am start -n org.cagnulen.qdomyoszwift/.CustomQtActivity
sleep 5
# Verify the active notification.
$ADB shell dumpsys notification | grep "ForegroundServiceChannel"
```

### 2. Start Zwift

```bash
$ADB shell monkey -p "$ZWIFT_PACKAGE" -c android.intent.category.LAUNCHER 1
```

You can also tap the Zwift icon manually.

### 3. Pair the Sensor in Zwift

On the Zwift sensor pairing screen:
- Search for "QZ" or "FTMS" in the sensor list.
- The device should appear as a power/cadence sensor.
- Select it and start a session.

### 4. Verify Data

With `applewatch_fakedevice=true`, QZ generates synthetic data. In Zwift, you should see:
- Power: a simulated value, based on heart rate if available
- Cadence: 0 when no real sensor is connected

To show simulated cadence and resistance data too, also use:
```ini
virtual_device_enabled=true
virtual_device_bluetooth=true
```

## Technical Structure

```text
+---------------------------------+
|         Android Device          |
|                                 |
|  +----------+ Android           |
|  |    QZ    | notification      |
|  | (FTMS    |------------------>| System Notification
|  |  server) |                   | "QZ Fitness Sensor"
|  +----------+                   |
|                    ^            |
|  +----------+      |            |
|  |  Zwift   |------'            |
|  | reads the notification as    |
|  | a BLE sensor                 |
|  +----------+                   |
+---------------------------------+
```

Zwift on Android can read power and heart-rate data from notifications posted by other apps. QZ uses that mechanism through `android_notification=true`.

## Notes and Known Issues

- **Bluetooth name too long:** the emulator shows the "Bluetooth name too long" warning, but this does not affect notification-based testing.
- **Display over other apps permission:** QZ may ask for the "Display over other apps" permission, visible in Android Settings -> Apps -> QZ -> Special app access. It is not strictly required for notification-based testing.
- **Startup order:** always start QZ before Zwift. The notification must already be present when Zwift scans for sensors.
- **Restart required:** after changing `android_notification`, restart QZ to activate the foreground service.

## Automate the Test with adb

```bash
ADB=~/Library/Android/sdk/platform-tools/adb

# Full setup.
setup_qz_for_zwift() {
  $ADB shell am force-stop org.cagnulen.qdomyoszwift
  $ADB shell "run-as org.cagnulen.qdomyoszwift sh -c '
    mkdir -p \"/data/data/org.cagnulen.qdomyoszwift/files/.config/Roberto Viola\"
    printf \"[General]\nandroid_notification=true\napplewatch_fakedevice=true\n\" \
      > \"/data/data/org.cagnulen.qdomyoszwift/files/.config/Roberto Viola/qDomyos-Zwift.conf\"
  '"
  $ADB shell am start -n org.cagnulen.qdomyoszwift/.CustomQtActivity
  sleep 5
  # Verify the notification.
  POSTED=$($ADB shell dumpsys notification | grep "numPostedByApp" | head -1 | grep -o '[0-9]*')
  echo "Posted notifications: $POSTED"
}

setup_qz_for_zwift

# Start Zwift after setting ZWIFT_PACKAGE to the Zwift game package.
test "$ZWIFT_PACKAGE" != "com.zwift.android.prod"
$ADB shell monkey -p "$ZWIFT_PACKAGE" -c android.intent.category.LAUNCHER 1
```
