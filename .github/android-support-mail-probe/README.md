# Android support email integration tests

Run automatically inside the **existing** `android-emulator-test` CI job, for each Android emulator already in the matrix. This tests the Java handoff and Android sharing contract, not network delivery of an actual email.

| Android version | API | Emulator target | Cases |
| --- | ---: | --- | --- |
| Android 7.0 | 24 | default / x86 | All three |
| Android 8.0 | 26 | google_apis / x86 | All three |
| Android 9 | 28 | default / x86 | All three |
| Android 10 | 29 | default / x86 | All three |
| Android 11 | 30 | google_apis / x86 | All three |
| Android 12 | 31 | google_apis / x86_64 | All three |
| Android 13 | 33 | google_apis / x86_64 | All three |
| Android 15 | 35 | google_apis / x86_64 | All three |
| Android 16 | 36 | google_apis / x86_64 | All three |
| Android 17 | 37.0 | google_apis / x86_64 | All three |

The test calls the **actual production** `SupportEmail.compose()` from `SupportEmailTestActivity`, an exported Activity added **only to debug APKs** by Gradle's debug manifest overlay. It does **not** fork or duplicate `SupportEmail.java`.

An isolated `QZ Mail Probe` app is built with Android SDK tools and a temporary test key. It registers for `ACTION_SEND` with MIME `message/rfc822`. The UI automation selects it in Android's actual mail chooser and verifies what a **different UID** receives.

## Cases, per emulator

1. **with_log:** recipient `roberto.viola83@gmail.com`, subject, body, `ACTION_SEND`, MIME type; `EXTRA_STREAM` must be a `content://` URI under QZ's `FileProvider`, with a functioning cross-UID read grant; the recipient opens/decompresses gzip and checks its contents.
2. **without_log:** the same required email extras, but no `EXTRA_STREAM`.
3. **missing_log:** the same email extras and a nonexistent file path; the Java code must not produce an invalid or unreadable attachment URI.

An explicit **PASS** from the fake receiver is required for every scenario. No actual email is sent. Each matrix job uploads `support_email_evidence/` (results, Android chooser hierarchy, screenshots, logcat). Failures stop the job; API 33 and 37 retain the existing matrix-level `flaky` exemption, so consult their artifacts even if GitHub reports them as non-blocking.

## Visual evidence in each emulator artifact

For **every** scenario (`with_log`, `without_log`, `missing_log`) the test captures two **real screenshots from that emulator**, and fails if either PNG could not be created:

- `<scenario>_01_android_chooser.png`: the Android `ACTION_SEND` chooser **before** selecting the receiver, showing `QZ Mail Probe` as an available app.
- `<scenario>_02_email_compose_preview.png`: the **QZ Mail Probe** screen **after** selecting the receiver. It displays To, Subject, Body, attachment filename and URI, Android intent/MIME type and PASS/FAIL. This is an **email-compose preview in a test app, not a screenshot of Gmail**.
- `<scenario>_chooser.xml`: Android's UI hierarchy used to select the receiver.
- `<scenario>_99_failure.png` may be present if an interaction fails.

Do not interpret a successful probe result as verification that a specific third-party mail client (such as Gmail) renders the compose fields correctly; that still requires a separately configured device/client.


Run all the cases automatically with the existing `CI` workflow on a pull request that includes the new debug test activity; no manual setup is required.

These tests intentionally do not claim to validate the exact Gmail compose UI, which can vary independently of Android's documented `ACTION_SEND` contract, nor the QML layout or the native C++/JNI call site. They directly exercise the Java mail handoff in the packaged QZ APK.
