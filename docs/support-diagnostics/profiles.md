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
