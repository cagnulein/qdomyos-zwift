# Test and beta build verification

## A fix or test behaves differently from the build support expects

### Provide first

- The QZ version and build number that is actually installed and running.
- Where the build came from: App Store/Play Store, TestFlight, nightly release, or a specific test artifact.
- If support requested a particular build, confirm its exact build number before reproducing the issue.
- A screenshot of the installed build/version screen when there is any doubt.

### If still unclear

- Restart QZ after installing the requested build and reproduce the issue once more.
- Provide a fresh debug log from that reproduction. Do not reuse a log captured with an older build.
- For TestFlight, check the QZ app page and the available previous builds to verify that the requested build is the one currently installed.
- For an artifact or nightly APK, provide the artifact/release identifier or link used for installation.

### Why it matters

- A debug log from a different build cannot validate a fix or behavior change that exists only in the requested build.
- TestFlight can expose multiple current or previous builds, so having access to a test build does not by itself prove that it is the installed version.
- Confirming the exact running build before collecting another log prevents repeated tests against stale code and makes comparisons between reproductions meaningful.
