# PR #5006 — profile enumeration trace

## User request

The user reported that `Jan.qzs` and `Jay.qzs` were visible in Android's `Documents/QZ/profiles` directory but were absent from QZ's profile chooser. The user then asked for the issue to be resolved and for an accurate record of requested work and actions.

## Target

- PR: #5006
- Application branch: `feat/peloton-strava-combined`
- Local worktree branch: `fix/jason-profile-storage-pr5006`
- Android host: `192.168.0.136` (Mac ARM64)
- AVD: `qz_arm64_api29`
- Android: API 29, `arm64-v8a`
- Package: `org.cagnulen.qdomyoszwift`
- Public profile directory: `Documents/QZ/profiles`

## Evidence before the fix

- MediaStore contained `Jay.qzs` and `Jan.qzs` under `Documents/QZ/profiles/`.
- The installed QZ configuration contained `android_documents_folder=false`.
- `homeform::getProfileFiles()` queried MediaStore only when that setting was true (or on Android 14+).
- The QML page correctly called `getProfileFiles()`, so the empty chooser was caused upstream in the C++ storage-selection gate.
- This distinguished filesystem presence from UI model enumeration; the earlier claim of a complete UI pass was therefore incomplete.

## Change made locally

`src/homeform.cpp` now enumerates public MediaStore profiles on Android 10+ independently of `android_documents_folder`. The setting still controls migration and writes. The legacy app-private profile directory remains a fallback, and duplicate filenames are deduplicated.

No production Bluetooth workaround was added. No profile files or app data were deleted.

## Verification completed locally

- `git diff --check`: passed.
- `qmllint src/profiles.qml`: passed.
- Correct worktree library rebuild/relink: passed (`make libqdomyos-zwift.a -j1`).
- Profile regression executable built from the correct worktree: passed.
- `ProfileRegressionTestSuite`: 21 tests run, 19 passed, 2 known skips:
  - `ChildProcessLoadsProfilesAfterRestart`
  - `RealProcessRestartPreservesBothProfiles`

## Not yet verified

The changed C++ code is not yet in a new Android APK on the Mac. A new Android artifact from the exact branch/commit must be built, verified, installed on `qz_arm64_api29`, and tested with `android_documents_folder=false` while `Jan.qzs` and `Jay.qzs` remain in MediaStore. The UI verdict must include a fresh screenshot and UIAutomator dump showing both names, followed by selecting and loading `Jay`.

## Follow-up fix

The follow-up runtime test also found that the loaded profile was not durable across an immediate Android quit/relaunch: `loadSettings()` updated the in-process field and showed the dialog, but the next process read `default`. `homeform::loadSettings()` now calls `settings.sync()` before emitting the profile-change signal, flushing `profile_name` before the restart path.
