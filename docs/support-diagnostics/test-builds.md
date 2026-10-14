# Test and beta build verification

## A fix or test behaves differently from the build support expects

### Provide first

- The exact QZ version and build number that is **actually installed and running**.
- Where that build came from: App Store/Play Store, TestFlight, nightly release, or a specific GitHub Actions/test artifact.
- If support requested a particular build, confirm its exact build number before reproducing the issue.
- When a debug log is being reviewed, confirm that the log was captured **after** installing the requested build.
- A screenshot of the installed QZ build/version screen when there is any doubt.

### If still unclear

- Restart QZ after installing the requested build and reproduce the issue once more.
- Provide a fresh debug log from that reproduction. Do not reuse a log captured with an older build.
- For TestFlight, open the QZ app page and check the available builds/previous builds to verify that the requested one is the version currently installed.
- For a nightly or GitHub Actions build, provide the release/artifact link or identifier used for installation.

### Why it matters

- A debug log from a different build cannot validate a fix or behavior change that exists only in the requested build.
- TestFlight can expose multiple current or previous builds, so having access to a test build does not prove that it is the version currently installed.
- Confirming the running build before collecting another log prevents repeated tests against stale code and makes comparisons between reproductions meaningful.
