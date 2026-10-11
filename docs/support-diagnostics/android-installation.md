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