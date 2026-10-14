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

## Current verification before the URI fix

- Commit tested: `06d4cda5fc0dc69a06d7f2aaca1b719b67053a3c`.
- Local `make -j1`, `qmllint src/profiles.qml`, `git diff --check`, and `ProfileRegressionTestSuite` passed (21 tests, 19 passed, 2 known skips).
- GitHub Actions run `38556230098` produced artifact `11890760814` (`peloton-v1-grupetto`), SHA-256 `2f4a7a0f1c4dc528d7943a5f48e49d45c42d1997d40726938b8952fe884ccf5`.
- APK package: `org.cagnulen.qdomyoszwift`; ABIs include `arm64-v8a`; installed on `qz_arm64_api29` without clearing app data.

The exact artifact showed the chooser correctly listing `Jay` and `Jan`, while the emulator had both canonical rows and old duplicate MediaStore rows (`Jan (1).qzs`, `Jay (1).qzs`). Selecting Jan showed the expected `Profile loaded` restart dialog. The subsequent restart exposed a URI transport bug: the pending marker contained `file:[object%20Object]` instead of the selected `content://.../Jan.qzs` URI. QZ therefore retained Jay's values (`81 kg`, `180 cm`, `Female`) while the profile label had become Jan. This is a confirmed reproduction, not a successful persistence result.

## URI transport correction

The pending restart now carries only the selected profile base name from QML. C++ resolves that name through `getProfileFiles()` in the active canonical root and writes the resolved `QUrl` to the marker. The Android import path uses the same source-marker helper directly. This avoids both the lossy JavaScript string conversion that created `file:[object%20Object]` and unreliable QML-to-`QUrl` QVariant conversion.

The URI correction is locally built and tested; its exact Android artifact and simulator verification are pending. No production Bluetooth workaround was added, and no profile files or app data were deleted during testing.
