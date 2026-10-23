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
- Before the global fix, the chooser's storage selection and the persisted routing setting could disagree, so the visible source and the load source were not guaranteed to be the same.
- The old `loadSettings()` path also staged every Android input in `files/settings/`, including already-local profiles. That staging was unnecessary for local files and obscured which file was canonical.
- The runtime path therefore had two independent problems: inconsistent root selection and unnecessary staging of already-local files.

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

## Final Android verification

- Final commit: `b2aa08198e678eee7e84bb258e313de7c5913e78`.
- Exact GitHub Actions run: `38569634375`; Android build job `115871110989` passed and produced artifact `11899214353` (`peloton-v1-grupetto`).
- Exact artifact SHA-256: `22dcd59e7f5780a57b8655c70003a95eacb02f13c33fc285b26840907e10fc87`.
- CI Android API 29 job `115895160867` passed for the same head SHA; all nine Android emulator matrix jobs in the run passed. The aggregate run remained red because unrelated Windows/Linux/Raspberry Pi jobs failed.
- The artifact was installed on `qz_arm64_api29` without clearing app data. The observed target was API 29, `arm64-v8a`.
- MediaStore read-back showed exactly `Jay.qzs` and `Jan.qzs` under `Documents/QZ/profiles/`, with no `(1)` duplicate rows in the QZ chooser.
- The chooser UI dump exposed separate `Jan` and `Jay` rows. Selecting Jan, confirming the restart dialog, and launching QZ again produced `content-desc="Profile: Jan"`.
- The QZ log recorded `Loading pending profile QUrl("content://media/external_primary/file/39")`; MediaStore read-back mapped ID 39 to `Jan.qzs`.
- Persisted Jan settings read back after restart were `weight=75`, `height=175`, `age=35`, `sex=Male`, and `ftp=200`; the stored routing setting remained `android_documents_folder=true` and was not copied into the profile snapshot.

This is historical evidence from the b2 artifact, not a final validation: the independent review found that the public-storage routing, exact URI identity, failure handling, and non-Android restart behavior still required correction. The b2 artifact must therefore not be treated as validated.

## Review iteration after b2

The external review identified these blocking defects in b2:

- API 29 public profiles were skipped when `android_documents_folder=false`.
- Restart re-resolved by base name instead of carrying the selected URI identity.
- A missing or unreadable source could clear active settings before the failure was known.
- The QML restart path regressed on non-Android platforms.
- Public profile writes bypassed MediaStore and did not check write status.
- MediaStore replacement could truncate the existing row before a replacement was complete.

The follow-up iteration addresses those cases by enumerating Android 10+ public profiles independently of the routing toggle, carrying an explicit encoded URI through QML/C++, retaining the pending marker until a successful load, staging public writes through MediaStore, restoring the desktop restart path, and using a temporary MediaStore row for replacements. Local verification after the iteration: forced C++ build passed; `qmllint` and `git diff --check` passed; the profile suite passed 19/21 with the two opt-in process tests skipped; with `QZ_PROFILE_RUN_PROCESS_RESTART=1`, 20 tests passed; and `FailedLoadPreservesActiveSettings` passed.

## Current review iteration

- Head before the MediaStore ordering correction: `1ddd8f44a99c02e83e90d968cfc0cb50064df11c`.
- CI run `39964082082` checked out PR merge commit `225e10ac64d220f01eb08ab4be0268d9471ae30f`, whose first parent is the base and whose second parent is the head above. The Android build job `120449264432` passed and the Android emulator jobs passed, but the aggregate workflow was red for unrelated jobs.
- Artifact `peloton-v1-grupetto` was downloaded and verified with SHA-256 `b531a56caa1ff09ce8d42adf2d9436d5e497554c676de2013aec024a2fbac9c2`; package `org.cagnulen.qdomyoszwift`, version code `1404`, with `arm64-v8a`, `armeabi-v7a`, `x86`, and `x86_64` libraries.
- The API 29 emulator job used `x86_64`, not `arm64-v8a`, and only installed/launched the app, checked the process, captured the legal disclaimer/orientation screenshots, and collected logcat. It did not enumerate or select profiles, restart, or verify persisted settings. No current-run `Jan`/`Jay` or profile-flow evidence exists.
- The current head was therefore not validated end to end. The remaining gates are a real profile-flow run on the exact artifact, public/legacy-root coverage, complete setting round trips, and API 29 ARM64 runtime provenance.

The latest source-only correction publishes the MediaStore replacement row before deleting superseded rows, so a publish failure leaves the previous public profile intact. It also orders same-name public rows by descending MediaStore ID, so a cleanup failure deterministically prefers the newest published row and can be retried on a later save. Fresh local verification on `167b5fe6197c5e991a1c3be913ad808cc8032529`: `make -j1`, `qmllint src/profiles.qml`, `git diff --check`, and the profile suite with `QZ_PROFILE_RUN_PROCESS_RESTART=1` passed (`21 passed, 1 opt-in skip`). Android artifact and runtime verification are still required.
