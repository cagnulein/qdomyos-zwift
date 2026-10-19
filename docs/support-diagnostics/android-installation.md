# Android installation and embedded consoles

## QZ crashes, restarts, or behaves unexpectedly after manually installing an APK

### Provide first

- The exact fitness machine make and model.
- Whether QZ is running on the machine's built-in Android console/tablet or on a separate Android device.
- The Android version, if known.
- The exact APK filename or download link that was installed, and whether it came from Google Play, a QZ GitHub release/nightly, or another source.
- Describe what happens from launch through the failure, including whether the problem starts after a permission request or restart.

### If still unclear

- Confirm that any previous QZ installation was removed before installing the intended build, including clearing its app data/cache when appropriate.
- Provide a screenshot or photo of the error shown by Android.
- If the app still exits or crashes, collect an Android bug report/crash report in addition to any QZ debug log that can be produced before the failure.

### Why it matters

- QZ has machine-specific Android builds in addition to the general Android build. Knowing the machine and the exact APK immediately distinguishes an installation/build mismatch from a runtime device problem.
- Built-in fitness-machine consoles can differ substantially from normal phones/tablets even when both run Android, so identifying where QZ is installed is important before changing QZ settings.
- The exact failure point helps distinguish an Android permission/restart problem from Bluetooth discovery or machine-protocol problems.
- A QZ debug log may end before an application crash is explained; an Android bug/crash report can provide the platform-level reason for the exit.

## Android refuses to install a QZ test APK

### Provide first

- Device model and Android version.
- Installed QZ version and the source of both the installed version and the attempted test APK.
- Exact installer error text or screenshot, and available storage.

### If still unclear

- Back up QZ settings before considering uninstalling a working installation.
- If ADB is available, collect the Android package manager's full install error code with the same APK.

### Why it matters

- Package installation errors occur before QZ starts and cannot be explained by QZ runtime logs.
- Build source and installer error help distinguish signing, compatibility, and storage problems.

## QZ runs on an embedded Android treadmill console but receives no telemetry

### Provide first

- Exact treadmill and console model, Android version, QZ build, and whether QZ is installed directly on the console.
- State whether the console itself displays changing speed or incline while QZ remains at zero; distinguish physical treadmill buttons from QZ buttons.
- Confirm whether the manufacturer's dashboard or an overlay remains running when QZ starts, and whether QZ is expected to use a serial/UART connection rather than Bluetooth.

### If still unclear

- Run a short controlled test with QZ in the foreground and the manufacturer application stopped, where safe and supported; note if the manufacturer overlay reappears after pressing Start.
- Share the relevant QZ machine-specific setting and a debug log covering startup plus a single speed or incline change.
- If ADB access is available, report whether the manufacturer app/service restarts or continues to hold the serial port. Do not disable safety controls or permanently remove system packages.

### Why it matters

- A proprietary console may show live values through its own application while QZ cannot read the shared serial interface.
- Physical controls, QZ commands, and telemetry are separate paths; testing them independently avoids mistaking a port-ownership conflict for an external training-app problem.
- A startup log and the manufacturer's app state distinguish serial access contention from missing decoding or control support.

## No QZ debug file appears on an embedded Android console

### Provide first

- Confirm Debug Log was enabled in QZ Experimental Settings and that a short reproduction was completed.
- State whether any QZ log appeared in the accessible app files or Documents directory, and identify the console model and Android version.

### If still unclear

- If ADB access is available, support may request a short timestamped Android logcat capture covering one safe test action, alongside the status of manufacturer apps using the serial interface.
- Do not disable manufacturer safety controls or services solely to gather logs.

### Why it matters

- Android system logs may expose serial-port access or process conflicts when QZ's own logging is unavailable.
