# Android Testing Documentation

This directory contains procedures for testing QZ (qdomyos-zwift) with Android emulators and physical devices on the local network.

## Structure

- [qz.md](qz.md) - Installing and testing QZ on an emulator or device
- [zwift.md](zwift.md) - Testing the QZ <-> Zwift integration through Android notifications, without Bluetooth
- [automation.md](automation.md) - UI Automator and adb scripts for automated tests

## Current Status (2026-06-19)

| App | `Pixel_8_API_36` Emulator | Notes |
|---|---|---|
| QZ v2.21.5 | Installed and configured | Fake Device and Android Notification enabled |
| Zwift | Not installed | Install from the Play Store to test the integration |

## Initial Setup

### Local Emulator (Android Studio)

Available AVD: `Pixel_8_API_36`

Start it from the command line:
```bash
~/Library/Android/sdk/emulator/emulator -avd Pixel_8_API_36 -no-snapshot-load
```

Wait until the emulator is ready:
```bash
~/Library/Android/sdk/platform-tools/adb wait-for-device shell getprop sys.boot_completed
# returns "1" when it is ready
```

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
