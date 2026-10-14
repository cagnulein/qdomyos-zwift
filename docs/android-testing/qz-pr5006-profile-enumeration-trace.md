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

## Evidence before the global fix

- MediaStore contained `Jay.qzs` and `Jan.qzs` under `Documents/QZ/profiles/`.
- The installed QZ configuration contained `android_documents_folder=false`, so the active root was the legacy app-data directory.
- The same names also existed in `/storage/emulated/0/Android/data/org.cagnulen.qdomyoszwift/files/profiles/`.
- The chooser nevertheless listed the public MediaStore copies because `getProfileFiles()` queried public storage independently of the routing setting.
- `loadSettings()` also copied a local profile into `files/settings/` before reading it, leaving a second snapshot such as `settings/Jan.qzs`.
- The runtime path therefore had two independent problems: selecting the wrong physical file and creating an unnecessary staging copy for an already-local file.

## Verification completed locally

- `git diff --check`: passed.
- `qmllint src/profiles.qml`: passed.
- Correct worktree library rebuild/relink: passed (`make libqdomyos-zwift.a -j1`).
- Profile regression executable built from the correct worktree: passed.
- `ProfileRegressionTestSuite`: 21 tests run, 19 passed, 2 known skips:
  - `ChildProcessLoadsProfilesAfterRestart`
  - `RealProcessRestartPreservesBothProfiles`

## Global profile-loading fix in the current worktree

The current patch keeps one storage root end to end:

- `android_documents_folder=true`, or Android 14+, uses `Documents/QZ` and MediaStore enumeration.
- Otherwise the chooser uses only the legacy app-data root.
- A local profile file is loaded directly; only a `content://` import is copied into the active root.
- Loading a profile replaces the active snapshot instead of overlaying it, while preserving storage routing, the profile encryption key, and account-scoped token records.
- The profile page does not load into the live process. It records the selected profile path in a small private pending-restart marker and exits; the next normal launch consumes that marker before QML `Settings` instances are created. This avoids stale QML values being written back during teardown.

No production Bluetooth workaround was added. No profile files or app data were deleted by the test procedure.

## Current local verification

- `make -j1`: passed.
- `qmllint src/profiles.qml`: passed.
- `git diff --check`: passed.
- `ProfileRegressionTestSuite`: 21 tests run, 19 passed, 2 known skips; no failures.

The Android artifact and simulator verification for this new patch are still pending; the historical artifact verification below refers to the earlier enumeration-only build.
The exact remote head `8772d08d8f3c7dd325fcc6ab96c4b923f9a87c7f` produced GitHub Actions run `38245337234`, Android build job `114797671560`, with the Android build step successful. The downloaded APK was verified with:

- SHA-256: `91364448648f415886ed11ee03e3092f1d54aeaa81adf48fa69170259ff0e74b`
- Package: `org.cagnulen.qdomyoszwift`
- Activity: `org.cagnulen.qdomyoszwift.CustomQtActivity`
- ABIs: `armeabi-v7a`, `arm64-v8a`, `x86`, `x86_64`

On `qz_arm64_api29` (`API 29`, `arm64-v8a`, `emulator64_arm64`), with `android_documents_folder=false` and `virtual_device_bluetooth=false` only in the emulator runtime:

1. MediaStore contained `Jay.qzs` and `Jan.qzs` in `Documents/QZ/profiles/`.
2. A fresh UIAutomator dump and screenshot showed both `Jay` and `Jan` in the QZ Profiles page.
3. The first tap selected `Jay`; the second tap displayed `Profile loaded` and the quit/restart prompt.
4. After confirming restart and dismissing the first-run disclaimer, a fresh Profiles screenshot and UIAutomator dump showed the profile name and selector as `Jay`, with both `Jay` and `Jan` still listed.

This closes the requested enumeration, selection, load, and restart-persistence verification. The overall GitHub Actions workflow was not declared fully green: unrelated matrix jobs were still queued/in progress or had independent failures when this verification completed.
