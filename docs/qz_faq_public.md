# QZ (qdomyos-zwift) - Frequently Asked Questions

## Getting Started / Builds

### 1. Joining the beta program (Android)
**Q: How can I join the QZ beta program / get the latest beta build?**

For Android, join the beta here:
https://play.google.com/apps/testing/org.cagnulen.qdomyoszwift
Then update the app from the Play Store as usual.

### 2. Trying a nightly/test build (APK)
**Q: Can I try a fix or new feature before it's officially released?**

Nightly debug builds are published here (replace the date with a recent one):
https://github.com/cagnulein/qdomyos-zwift/releases/download/nightly-YYYY-MM-DD/android-debug.apk
You can install this over your existing app without losing your settings.

---

## Integrations (Garmin, Google Fit, Zwift)

### 3. Garmin integration
**Q: How do I use QZ with my Garmin watch (e.g. to control treadmill speed/incline from a Garmin workout)?**

Set your Garmin credentials in QZ. QZ will then download Garmin workouts
directly and you can also push completed workouts back to Garmin
automatically. You can also create workouts directly inside QZ.

### 18. Apple Health - reading age/weight/heart rate zones automatically
**Q: Can QZ read my age, weight, resting heart rate and HR zones automatically from Apple Health, so I don't have to update them manually?**

Not currently - these values are entered manually under Settings > User and
won't auto-update from Apple Health. If yours are out of date, update them
there. Automatic syncing of this profile data from Apple Health is a
possible future improvement.

---

## Device Compatibility

### 7. Compatibility questions (new bike/treadmill/rower brand)
**Q: Is my [bike/treadmill/rower] compatible with QZ / Zwift via QZ?**

Most FTMS-compatible and many proprietary BLE trainers/treadmills/rowers are
supported. You can try the trial version first. If your device isn't
recognized, send a debug log so support for it can be added or fixed.

### 19. Kinomap-compatible bikes (Kettler, Merach, and others)
**Q: Does QZ work with bikes that connect to Kinomap (e.g. Kettler Racer S, Merach S28)?**

Yes - any indoor bike that supports the Kinomap BLE protocol can connect
to QZ using that protocol, with working auto resistance. This includes
Kettler bikes and other brands such as Merach that use the same Kinomap
BLE stack. Once QZ is connected, apps like Zwift, MyWhoosh and Rouvy can
control resistance automatically through QZ.

Try the trial first; if your bike isn't detected or resistance doesn't
behave correctly, send a debug log.

### 21. Speed shown in training app differs from bike console
**Q: My bike console shows (e.g.) 25 km/h but the training app (Kinomap, Zwift, etc.) shows only 8 km/h - why?**

These are two different things. The console speed is the actual flywheel/wheel
speed. The training app's speed is a virtual estimate calculated from power
output (watts) — it represents how fast you'd go on a flat road at that effort.
QZ passes through whatever the bike reports over BLE; the discrepancy isn't
something QZ can correct because it comes from how each app converts power to
virtual speed.

### 22. HR-linked speed control on treadmill
**Q: Can QZ automatically adjust treadmill speed based on heart rate (e.g. reduce speed when HR is too high, restore it when HR drops)?**

Yes - QZ has a heart rate linked speed feature in the treadmill settings. You
can configure an HR threshold and speed adjustment so that when your HR goes
above the target, QZ automatically reduces speed, and when it drops back down,
QZ restores the original pace. This achieves the same effect as a simple
HR-controlled interval loop.

Sound alerts triggered by HR zone are not currently supported.

---

## Troubleshooting & Support

### 9. Getting a debug log
**Q: How do I get a debug log to send for troubleshooting?**

Follow this guide:
https://github.com/cagnulein/qdomyos-zwift/wiki/How-do-i-get-the-debug-log-in-case-something-doesn't-work%3F
### 14. Calorie burn estimate looks way too high
**Q: QZ shows an unrealistically high calorie burn (e.g. 30+ kcal/minute) - what's wrong?**

Without a heart-rate monitor connected, QZ estimates calories from your
speed/incline and the weight set in your profile. A very high value usually
means the weight setting is wrong (e.g. entered in the wrong unit, kg vs
lb) or the speed/incline readings themselves are unrealistic. Check
Settings > User for your weight, and check the live speed/incline tiles. If
those look correct and calories are still off, send a debug log.

### 24. BLE advertising error on Android ("Cannot start advertising in ClosingState")
**Q: QZ shows an "AdvertisingError / Cannot start advertising in ClosingState" in the debug log and another device (e.g. a second phone/tablet running a training app) can't find QZ via Bluetooth.**

This is an Android BLE limitation triggered by long Bluetooth device names. Workaround:
1. Go to Android Settings > About phone (or About tablet) > Device name.
2. Rename the device to a **short name of 4 characters or fewer** (e.g. "tab1").
3. Reboot the device.

After the reboot, QZ should advertise correctly and be visible to other apps/devices.

---

### 25. Running QZ and Zwift on the same Mac (Apple Silicon / macOS)
**Q: Can I run QZ and Zwift on the same MacBook (Apple Silicon, macOS Sequoia) to bridge my trainer?**

Yes, QZ has a macOS build that runs on Apple Silicon. If you run into issues with your trainer not being detected or metrics not updating when both QZ and Zwift are on the same Mac, please share a debug log from a session that reproduces the problem so it can be diagnosed:
https://github.com/cagnulein/qdomyos-zwift/wiki/How-do-i-get-the-debug-log-in-case-something-doesn't-work%3F
