# Profiles and settings storage

## A saved profile disappears from the QZ profile list

### Provide first

- The platform and QZ version/build.
- Clarify whether the **entire profile** disappears from the profile list or only one or more settings revert.
- On Android, check the QZ profile storage folder with the Files app and report whether the missing profile file still exists.
- Provide a screenshot of the Profiles screen before and after the problem.
- Give the exact sequence used to create, save, load, switch, and restart QZ.

### If still unclear

- On Android, provide the current value of **Experimental Settings > Android Documents Folder** for each affected profile.
- Confirm whether all profile files are stored under the same QZ profiles folder.
- Reproduce with two minimal test profiles and change only one small setting before switching.
- If a requested test build is involved, also follow the test-build verification guide and confirm the exact running build.

### Why it matters

- A profile file that still exists while the profile disappears from the QZ list points to loading, enumeration, or storage-path behavior rather than file deletion.
- Distinguishing a whole-profile failure from one setting reverting avoids investigating unrelated profile serialization fields.
- The Android Documents Folder option affects which storage location QZ uses, so its value is important when files exist on disk but are not presented as expected.
- A precise save/load/restart sequence makes profile lifecycle problems reproducible without requiring a long workout.

## Switching profiles appears to mix user settings or connected-account state

### Provide first

- QZ platform and **exact installed build**, whether it was installed from a store or a test APK, and whether the problem occurs on a shared Android fitness console.
- Identify **two newly created, non-personal test profiles** with visibly different harmless settings (for example, different display preferences). State whether those settings remain distinct after **saving each profile, switching, and restarting QZ when prompted**. Do not send real account credentials or personal profile measurements.
- On Android, screenshot **Experimental Settings > Android Documents Folder** and confirm its value **after a complete QZ restart**, not just immediately after changing it. State whether old and newly created profiles coexist in the list and whether any were imported from a different storage location.
- Say whether the apparent crossover occurs **before** any Garmin/Strava sign-in, or only after connecting an account. Distinguish live metrics missing in QZ itself from an upload or authentication failure.

### If still unclear

- Repeat with only the two fresh test profiles in the **same storage mode**; keep existing profiles/files intact. Avoid uninstalling, clearing app data, moving profile files, or importing older profiles merely to reproduce the issue.
- Capture one short debug log per profile **immediately after loading/restarting**, noting the active test profile and the Android Documents Folder value. If the option reverts after restart, stop account-authentication tests and report that behavior first.
- Only after basic profile isolation is established, test the connected accounts separately and report **success/failure of authentication versus workout upload** for each profile. Redact tokens and account identifiers from any shared logs.

### Why it matters

- A selected profile name is not enough to prove that the expected settings file was loaded. The Android Documents Folder option can change which profile storage location is in use; its persisted value and the profile actually loaded must be checked together.
- A successful token refresh for one account does not prove that the other profile's credentials or upload path work. Testing harmless settings first separates **profile persistence** from **third-party authentication**.
- Logs from a multi-profile investigation showed unexpected active profile names and a storage option value inconsistent with the intended test setup. Verifying the active profile and persisted option before repeating sign-ins prevents misleading conclusions about cross-account contamination.
- Preserve existing user data while diagnosing: a clean reinstall is not an appropriate first diagnostic step. See the [profiles FAQ](../faq/profiles.md) for normal profile-management instructions.

## Imported settings file appears to load, but profile, tiles, or options are not restored

### Provide first

- QZ platform, operating-system version, exact installed QZ build, and whether this is a new phone or a different QZ installation.
- Confirm whether the imported file is an **original saved configuration** or a newer export containing only selected settings. Identify the file by a non-personal filename and date; do not send account credentials.
- Describe precisely how the file was selected (including **Other folders** if used), whether QZ displayed a success/error message, and which harmless visible settings were present before import, immediately afterward, and after restarting QZ.
- State whether the active profile name changes or only individual settings (for example, tile layout) fail to return.

### If still unclear

1. Preserve the original `.qzs` file outside QZ's app storage. Enable **Experimental Settings > Debug Log**, then completely close and reopen QZ **before** starting the test.
2. In that same running session, use the settings **load/up-arrow** action and select the **original file**, not a default-settings export generated during troubleshooting. Record the selected filename and any confirmation or error shown.
3. **Before closing or restarting QZ**, save the debug log covering the actual file-selection/import action. A log captured only after restart cannot show whether the import was attempted or rejected.
4. Then restart QZ, check the active profile and two harmless settings, and export a fresh `.qzs` file. Compare the original and post-import exports with the same test settings; redact account identifiers, tokens, and personal information before sharing.
5. If the issue differs by phone or QZ build, repeat the same import with one change at a time and identify which log belongs to which attempt. Do not uninstall or clear data merely to gather evidence.

### Why it matters

- A valid export may intentionally contain only part of the expected configuration, so an incomplete source file and a failed import are different problems.
- The exact file selected and the **log covering the import itself** distinguish file-picker/path failures, load-time parsing, and settings that fail to persist across restart.
- Comparing pre-import, immediate post-import, and post-restart state isolates loading from persistence or profile selection; an unrelated later startup log does not establish what happened during import.
- See the [settings migration FAQ](../faq/settings.md) for normal export/import behavior and the [profiles FAQ](../faq/profiles.md) for profile-management semantics.
