# Android

## Zwift loses its connection to QZ after about a minute when QZ is in the background. What should I do?

On Android, enable **Settings > Experimental Settings > Android Notification** in QZ, then restart QZ before starting your session.

This setting creates an active QZ notification while the app is running and is required for same-device Zwift operation on Android. In a confirmed support case, enabling it stopped a repeatable disconnect that occurred about every 60 seconds whenever Zwift was in the foreground.

This workaround is Android-specific. iOS does not provide QZ with the same background mechanism; if your iOS setup requires QZ and the training app to remain active at the same time, use separate devices.

## Debug logging is enabled, but QZ has stopped creating new log files. What should I try?

If older debug logs are still present but QZ no longer creates new ones, delete the **entire QZ log folder**, not only the files inside it. Then start QZ again so the app can recreate the folder and generate fresh logs.

This recovery step was confirmed to restore debug-log creation in an Android support case where the debug-log setting was already enabled but no new files were being written.

## QZ does not appear in Google Health Connect. How do I enable workout export?

QZ can write completed workouts to **Google Health Connect** on Android 8 / API 26 or newer. Health Connect must be available on the device and QZ must be granted its requested write permissions.

1. Make sure Health Connect is installed or available on the Android device.
2. Update QZ to a current build that includes the Health Connect integration. If the stable Play Store build on your device does not contain it yet, use the current beta build.
3. Fully close and reopen QZ. When Health Connect is available, QZ checks for missing permissions at startup and opens the Health Connect permission request automatically.
4. Grant the requested Health Connect write permissions.
5. Start a workout in QZ, record some activity, and then **stop the workout in QZ**. The Health Connect workout is written when the completed session is saved, not merely when live metrics begin updating.

In a confirmed support case, an older Play Store build did not appear in Health Connect; after updating to the beta that contained the integration, QZ immediately requested the Health Connect permissions and became available for workout export.
