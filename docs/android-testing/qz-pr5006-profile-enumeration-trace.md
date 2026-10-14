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
- The old `loadSettings()` path also staged every Android input in `files/settings/`, including already-local profiles. That staging was unnecessary for local files and obscured which file was canonical.
- The runtime path therefore had two independent problems: selecting the wrong physical root and creating an unnecessary staging copy for an already-local file.

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

## Current verification

- Commit: `4f4aa073bf60ed5795a74ca9701d5aa277fbd37b`.
- `make -j1`: passed.
- `qmllint src/profiles.qml`: passed.
- `git diff --check`: passed.
- `ProfileRegressionTestSuite`: 21 tests run, 19 passed, 2 known skips; no failures.
- GitHub Actions run `38548393050`: Android build passed and Android emulator tests passed on API 24, 26, 28, 29, 30, 31, 33, 35, and 36.
- Artifact `11886102214` (`peloton-v1-grupetto`): SHA-256 `9f0086c5c025cd3f6a5dd67db105f3e6257554da7da368b97cd75e18490c15ef`.
- APK package: `org.cagnulen.qdomyoszwift`; activity: `org.cagnulen.qdomyoszwift.CustomQtActivity`; ABIs include `arm64-v8a`.

On `qz_arm64_api29` (`API 29`, `arm64-v8a`, `emulator64_arm64`), the exact artifact was installed without clearing app data. The test routing was `android_documents_folder=false`; Bluetooth was disabled only in the emulator test fixture/runtime.

1. The Profiles page showed both `Jay` and `Jan`, with the active selector matching the loaded profile.
2. `Jan → Jay`: tapping the selected row twice showed `Profile loaded`; confirming `Yes` exited QZ and wrote only the private pending-restart marker containing the canonical `Jay.qzs` path. No second profile copy was created.
3. After a normal manual relaunch, the marker was consumed, `homeform::loadSettings` loaded `Jay.qzs`, and the active configuration showed `Jay`, `81 kg`, `180 cm`, `Female`, `FTP 200`, and `virtual_device_bluetooth=false`.
4. `Jay → Jan`: the same flow consumed the marker and loaded `Jan.qzs`. General Options visibly showed `75 kg`, `175 cm`, age `35`, `Male`; after scrolling, `FTP 200` and Critical Power Run `200` were visible.
5. The new process stayed on QZ after both relaunches; no new Bluetooth crash occurred. The old process logs a pre-existing Qt/QML teardown SIGSEGV while exiting after `Yes` (`QQmlMetaType::qmlRemoveModuleRegistration`). It does not prevent the requested manual relaunch, but it should be tracked separately from profile persistence.

The overall GitHub Actions workflow may still remain non-green because unrelated Linux/Windows/Raspberry jobs have independent failures; all Android build and emulator jobs for this commit passed.
