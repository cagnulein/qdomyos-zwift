# Android

## Zwift loses its connection to QZ after about a minute or a few minutes when QZ is in the background. What should I do?

On Android, check both the QZ foreground notification setting and Android's battery restrictions:

1. In QZ, enable **Settings > Experimental Settings > Android Notification**.
2. In Android settings, open the app settings for QZ and set its battery usage to **Unrestricted** or **No restrictions**.
3. Restart QZ before starting your session, then open Zwift.

The exact Android menu names vary by device and manufacturer. On some devices, the unrestricted battery option is hidden inside additional battery optimization or background usage settings, so make sure QZ is really excluded from battery restrictions.

The QZ Android Notification setting helps keep QZ active while Zwift is in the foreground, but Android's own battery management can still suspend or stop QZ in the background. In a confirmed support case, QZ repeatedly disconnected after roughly 2 to 5 minutes until the user found the additional Android battery setting and changed QZ to unrestricted battery usage. See [issue #5034](https://github.com/cagnulein/qdomyos-zwift/issues/5034).

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


## QZ cannot find my Bluetooth fitness device on Android. Which permissions should I check?

If the device is powered on, is not connected to another app, and still does not appear in QZ on Android, verify that Bluetooth and Location are enabled and that QZ has the requested Bluetooth and location permissions. If Android offers precise location access, allow it, then return to QZ and refresh the device list.

QZ's Android build requests Bluetooth discovery/connection and location permissions. In a confirmed support case, a bike that was otherwise ready to connect appeared immediately after precise location permission was granted.

## How can I resize QZ's floating window on Android to show more metrics?

If QZ's floating window is too short and you have to scroll to see the metrics, change the window dimensions rather than pinching its contents:

1. Open **QZ Settings > General UI Options**.
2. Increase **Floating Window Height** from the default **210** to, for example, **400** or **450**. Adjust **Floating Window Width** if necessary.
3. Save the settings, then close and reopen the floating window.

Pinching changes the displayed content size, not the floating window dimensions. The width and height settings are implemented in QZ's Android floating-window code. This is verified setup guidance; a customer confirmation is not yet available.

## A QZ Android test build will not extract or says it conflicts with the installed package. What should I do?

Two different installation problems need different fixes:

1. If the downloaded artifact ZIP reports **Unexpected end of archive** or fails to extract completely, **download it again** from the build's original GitHub Actions artifact link and extract the new download before installing the APK.
2. If the APK extracts but Android reports that the new package **conflicts with the existing package**, an existing QZ installation may use a different signing certificate. Android will not install a differently signed APK as an in-place update.
3. **Back up or export important QZ settings, profiles, and workout data first**, if they are stored locally and you need to keep them. Uninstalling the current QZ app can delete its local data.
4. Only if the normal update is blocked by that package/signature conflict, uninstall the existing QZ installation, install the extracted test APK, and restore your saved settings as needed.

Normally, compatible QZ Android builds can be installed over an existing installation without losing settings. **Do not uninstall by default.** In a confirmed support case, downloading the artifact again fixed the incomplete ZIP, and removing the conflicting existing installation allowed the test APK to be installed and run.
