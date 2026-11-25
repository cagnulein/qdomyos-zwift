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

## The downloaded Android APK matches, but QZ still behaves like an older build

### Provide first

- Exact test artifact link or ID and the SHA-256 of the downloaded APK, if support supplied a reference checksum.
- Installed QZ version/build and a fresh log captured after installing the APK.
- State whether the machine is an embedded Android console, and whether installation was performed through ADB or the normal package installer.

### If still unclear

- With ADB access and support guidance, use `adb shell pm path org.cagnulen.qdomyoszwift` to locate the installed package APK.
- Pull the installed `base.apk` to a computer and calculate its SHA-256; compare with the intended artifact, accounting for split APK packaging where applicable.
- If the installed file is verified, collect a new log from a single controlled action and identify the expected versus actual command behavior. Avoid repeatedly reinstalling the same verified APK.

### Why it matters

- Verifying the downloaded artifact alone does not establish which package Android is running.
- An installed-package checksum can separate a stale installation from a runtime command-path problem, preventing unnecessary repeated builds or downloads.

## An embedded Peloton bike starts QZ but cannot initialize or read the bike

### Provide first

- Confirm whether the hardware is the original Peloton Bike or Bike+, and whether QZ is installed **on the bike's console** rather than on a separate tablet or phone.
- Provide the exact installed APK filename, release date or artifact link, and running QZ build number. Confirm whether the APK matches the hardware variant.
- State whether the failure occurs at launch/initialization or only after starting a workout, and whether any metrics appear.

### If still unclear

- Compare the installed build with the current machine-specific nightly artifact for that hardware variant. For the original Bike, the workflow publishes `android-debug-peloton-bike.apk`; Bike+ uses a separate build. Do not substitute the generic Android APK.
- After installing the correct current build, reproduce the initial startup once and provide a **fresh** QZ log only if the failure persists.
- Record whether the console runs OpenPelo and which startup step remains incomplete.

### Why it matters

- The CI workflow produces separate Peloton Bike and Bike+ APKs. A correctly installed but older or wrong-variant APK can appear to be a bike initialization problem.
- In a confirmed startup case, switching from an older original-Bike APK to the current original-Bike nightly resolved the issue immediately, without further configuration changes.
