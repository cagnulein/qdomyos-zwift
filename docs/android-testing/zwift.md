# Testing the QZ <-> Zwift Integration on Android

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
# Check whether Zwift is installed.
$ADB shell pm list packages | grep zwift
# package:com.zwift.android.prod  (when installed)

# Install from the Play Store. A Zwift account is required.
# Credentials should stay in the device password manager.
```

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
$ADB shell monkey -p com.zwift.android.prod -c android.intent.category.LAUNCHER 1
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
```
