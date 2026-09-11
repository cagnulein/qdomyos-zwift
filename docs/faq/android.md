# Android

## Zwift loses its connection to QZ after about a minute when QZ is in the background. What should I do?

On Android, enable **Settings > Experimental Settings > Android Notification** in QZ, then restart QZ before starting your session.

This setting creates an active QZ notification while the app is running and is required for same-device Zwift operation on Android. In a confirmed support case, enabling it stopped a repeatable disconnect that occurred about every 60 seconds whenever Zwift was in the foreground.

This workaround is Android-specific. iOS does not provide QZ with the same background mechanism; if your iOS setup requires QZ and the training app to remain active at the same time, use separate devices.

## Debug logging is enabled, but QZ has stopped creating new log files. What should I try?

If older debug logs are still present but QZ no longer creates new ones, delete the **entire QZ log folder**, not only the files inside it. Then start QZ again so the app can recreate the folder and generate fresh logs.

This recovery step was confirmed to restore debug-log creation in an Android support case where the debug-log setting was already enabled but no new files were being written.
