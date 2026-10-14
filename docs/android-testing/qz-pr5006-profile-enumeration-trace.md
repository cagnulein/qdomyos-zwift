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

## Follow-up fix

The runtime test showed why `settings.sync()` alone was insufficient: the QML `Settings.profile_name` property still held `default` and could overwrite the C++ value during component teardown. `selectProfile()` now assigns `settings.profile_name` from the selected filename before calling `loadSettings()`.

## Final Android verification

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

## Follow-up diagnosis — profile fields contaminating each other

Jason's next report was reproducible on `qz_arm64_api29`: changing `weight`, `height`, and `sex` on `Jay`, loading `Jan`, and saving it caused the sentinel values to appear in both app-private profile files. The public MediaStore files remained distinct, so filesystem enumeration was no longer the failing layer.

The failure was caused by two settings stores in the same process. `loadSettings()` updates native `QSettings` directly, while the long-lived QML `Settings` object in `settings.qml` retains cached `weight`, `height`, and `sex` values. If the user selects `No` on the required-restart dialog, or if the QML object is torn down after selecting `Yes`, those stale properties can be written back over the newly loaded profile before `saveProfile()` or restart completes.

The follow-up change adds an allowlisted `homeform::profileSetting()` accessor and synchronizes `profile_name`, `weight`, `height`, and `sex` in the QML `Settings` instance when `userProfileChanged` is emitted. It does not expose credentials, change licensing/time/data, or alter Bluetooth behavior. The Android runtime-only `virtual_device_bluetooth=false` setting remains test-only.

## Follow-up verification on Jason's profile set

Commit `b8efd47156fa287ee25be6309f40e88aa6e8a8a9` was built by GitHub Actions run `38518063745`; the `android-build` job `115701251759` and steps 19–24 all passed. Artifact `11875102515` (`peloton-v1-grupetto`) produced an APK with SHA-256 `b72efdb773ffa5b0f4a0512eec7c437b5fab631cef4a0d48102cc2192ceebbc2`, package `org.cagnulen.qdomyoszwift`, and `arm64-v8a` support. The overall workflow remained `failure` because unrelated matrix jobs failed; this does not invalidate the successful Android job.

On `qz_arm64_api29` (`emulator-5558`, API 29, `arm64-v8a`), without clearing app data:

1. The public MediaStore files were confirmed as `Jay.qzs` and `Jan.qzs`, each 33.77 kB in `Documents/QZ/profiles/`.
2. `Jay` was saved with sentinels `81 kg / 180 cm / Female`; its effective private profile remained distinct.
3. Loading the public `Jan.qzs` through `Other folders`, choosing `No` on the restart prompt, and opening General Options showed `75 kg / 175 cm / Male` rather than the stale Jay values.
4. Saving `Jan` left the effective private files distinct: `Jay=81/180/Female`, `Jan=175/Male`.
5. After force-stop/restart (with only the emulator runtime Bluetooth flag set false), General Options still showed `75 kg / 175 cm / Male`; Profiles showed `Jan` selected and both `Jay` and `Jan` listed.

This closes the reported profile-field contamination on the real ARM64/API29 emulator. The Jason draft remains unsent.
