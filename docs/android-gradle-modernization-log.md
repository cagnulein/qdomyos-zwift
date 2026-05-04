# Android Gradle Modernization Log

Date: 2026-05-03
Repo: `/home/cagnulein/qdomyos-zwift`

## Goal

Modernize the Android Gradle setup as far as possible without breaking compatibility, and keep a written record of steps, failures, and decisions.

## Initial Findings

- Main Android packaging for the Qt app lives in `src/android/`.
- Secondary Android project `zwiftplay/` already uses a newer Gradle structure with version catalogs.
- Current main Qt Android toolchain state:
  - Gradle wrapper: `5.6.4`
  - Android Gradle Plugin: `3.6.0`
  - Repositories include `jcenter()`
  - Java level: `1.8`
- Local environment at start:
  - `qmake`: Qt `5.15.3`
  - `java`/`javac`: OpenJDK `17.0.16`
  - Android SDK/NDK env vars: not set

## Compatibility Working Assumption

- `src/android/` is generated from a Qt 5.15-era template and is more fragile than `zwiftplay/`.
- Because the request explicitly prioritizes compatibility, the main Qt Android project should not be pushed directly to AGP 8.x without a validated Qt-side migration path.
- A safer modernization target for `src/android/` is the last broadly compatible pre-AGP-8 lane:
  - AGP `7.4.x`
  - Gradle `7.6.x`
  - Java `11`+ runtime, while keeping source/target bytecode conservative.

## Steps Log

### 2026-05-03 1. Baseline inspection

Status: done

- Inspected:
  - `src/android/build.gradle`
  - `src/android/gradle/wrapper/gradle-wrapper.properties`
  - `src/android/gradle.properties`
  - `zwiftplay/*`
- Confirmed the main upgrade risk is the Qt app packaging template, not `zwiftplay`.

### 2026-05-03 2. External compatibility check

Status: done

- Checked official Android and Qt docs.
- Key references:
  - AGP `7.4` requires at least Gradle `7.5` and JDK `11`.
  - Newer Qt Gradle plugin docs target Qt 6.8+, not this project's Qt 5.15 setup.

### 2026-05-03 3. Conservative Qt Android Gradle upgrade

Status: done

- Updated `src/android/build.gradle`:
  - AGP `3.6.0` -> `7.4.2`
  - protobuf Gradle plugin `0.9.4` -> `0.9.5`
  - removed `jcenter()`
  - removed legacy `com.android.support:*` deps
  - added explicit `namespace`
  - removed stale `buildToolsVersion '29.0.2'`
  - removed duplicate `compileSdkVersion` from `defaultConfig`
- Updated `src/android/gradle/wrapper/gradle-wrapper.properties`:
  - Gradle `5.6.4` -> `7.6.6`
- Updated `src/android/gradle.properties`:
  - added Gradle JVM args
  - enabled full warning mode
  - added fallback `androidCompileSdkVersion=34`

### 2026-05-03 4. Android SDK bootstrap on local machine

Status: done

- Installed Android command-line tools under:
  - `/home/cagnulein/Android/Sdk/cmdline-tools/latest`
- Installed SDK packages:
  - `platform-tools`
  - `platforms;android-33`
  - `platforms;android-34`
  - `build-tools;33.0.1`
  - `build-tools;33.0.2`
  - `build-tools;34.0.0`
  - `build-tools;35.0.0` (pulled in by modern AGP during validation)
  - `ndk;21.4.7075529`
- Added machine-local SDK pointers:
  - `zwiftplay/local.properties`
  - `src/android/local.properties`

### 2026-05-03 5. `zwiftplay` build validation and modernization

Status: done

- First validation failure:
  - missing `local.properties` / `sdk.dir`
- After setting SDK path, `./gradlew :app:assembleDebug` succeeded.
- Modernized `zwiftplay`:
  - Gradle `8.0` -> `8.13`
  - AGP `8.1.2` -> `8.13.2`
  - Kotlin `1.9.10` -> `2.2.21`
- Removed Kotlin DSL deprecation by migrating from `kotlinOptions.jvmTarget` to `kotlin.compilerOptions`.
- Removed Groovy DSL property-assignment deprecations in Android module build files.
- Revalidated with:
  - `./gradlew :app:assembleDebug`
  - `./gradlew :app:assembleDebug --warning-mode all`
- Result:
  - build successful
  - no remaining build-script deprecation warnings surfaced in the validated path

### 2026-05-03 6. Main Qt Android project validation

Status: done

- Installed Qt for Android with `aqtinstall`:
  - location: `/home/cagnulein/Qt/5.15.2/android`
  - includes `androiddeployqt` and the Android Java/resource template tree
- Validated:
  - `QT_ANDROID_DIR=/home/cagnulein/Qt/5.15.2/android ./gradlew tasks`
  - `QT_ANDROID_DIR=/home/cagnulein/Qt/5.15.2/android ./gradlew assembleDebug`
- Modernized the main Qt Android Gradle project further:
  - AGP `7.4.2` -> `8.13.2`
  - Gradle `7.6.6` -> `8.13`
- Compatibility fixes applied:
  - wired Qt Android jar dependencies explicitly from `$QT_ANDROID_DIR/jar`
  - corrected Qt Android Java/AIDL/resource source set paths for the actual Qt 5.15 layout
  - added generated AIDL Java output to debug/release source sets
  - replaced `BuildConfig.DEBUG` usage in `ChannelService.java` with a runtime debuggable flag check
  - removed deprecated manifest `package=` usage by moving identity to `applicationId`
  - cleaned Gradle Groovy DSL assignments that produced warnings under Gradle 8.13
- Result:
  - `assembleDebug` successful
  - `minSdk` left unchanged at `21`

### 2026-05-03 7. GitHub Actions Android workflow alignment

Status: done

- Updated `.github/workflows/main.yml` Android jobs to match the validated local toolchain:
  - Qt Android `5.15.0` -> `5.15.2`
  - Java `11` -> `17`
  - `androiddeployqt --android-platform android-31` -> `android-34`
  - added explicit SDK package install for:
    - `ndk;21.4.7075529`
    - `platforms;android-34`
    - `build-tools;35.0.0`
  - aligned Qt patch destination paths to `Qt/5.15.2/android/jar/`
  - added `qtpurchasing` to Qt module installation in Android CI jobs
  - changed Android packaging to:
    - `androiddeployqt --gradle --aab --no-build`
    - remove `android.bundle.enableUncompressedNativeLibs=false` from generated `gradle.properties`
    - run `./gradlew assembleDebug bundleDebug` explicitly
- Kept `minSdk` unchanged.

### 2026-05-03 8. End-to-end local Android bundle validation

Status: done

- Verified the standard Android packaging flow end-to-end against the generated
  Gradle project:
  - `make INSTALL_ROOT=... install`
  - `androiddeployqt --gradle --aab --no-build`
  - remove `android.bundle.enableUncompressedNativeLibs=false` from generated
    `gradle.properties`
  - `./gradlew assembleDebug bundleDebug`
- Produced artifacts successfully:
  - `output/android-standard-local/build/outputs/apk/debug/android-standard-local-debug.apk`
  - `output/android-standard-local/build/outputs/bundle/debug/android-standard-local-debug.aab`
- Kept `minSdk` unchanged at `21`.

### 2026-05-03 9. Qt Android 5.15.0 rollback and revalidation

Status: done

- Reverted Android packaging target from Qt Android `5.15.2` back to `5.15.0`,
  as requested.
- Rebuilt and installed local `qthttpserver` against
  `/home/cagnulein/Qt/5.15.0/android`, because the initial Qt `5.15.0` toolchain
  was missing the installed `QtHttpServer` / `QtSslServer` pieces required by the
  current project configuration.
- Re-ran the standard Android packaging flow on Qt Android `5.15.0`:
  - `qmake -spec android-clang 'ANDROID_ABIS=armeabi-v7a arm64-v8a x86 x86_64'`
  - `make INSTALL_ROOT=/home/cagnulein/qdomyos-zwift/output/android-standard-local-5150 install`
  - `androiddeployqt --gradle --aab --no-build`
  - fix generated `local.properties` to the real SDK path
  - remove `android.bundle.enableUncompressedNativeLibs=false`
  - `./gradlew assembleDebug bundleDebug`
- Produced artifacts successfully:
  - `output/android-standard-local-5150/build/outputs/apk/debug/android-standard-local-5150-debug.apk`
  - `output/android-standard-local-5150/build/outputs/bundle/debug/android-standard-local-5150-debug.aab`
- Kept `minSdk` unchanged at `21`.

### 2026-05-03 10. Garmin Android SDK refresh

Status: done

- Updated the Android Garmin dependency in `src/android/build.gradle` from
  `com.garmin.connectiq:ciq-companion-app-sdk:2.2.0@aar` to `2.3.0@aar`.
- Validation completed successfully on Qt Android `5.15.0`:
  - `QT_ANDROID_DIR=/home/cagnulein/Qt/5.15.0/android ./gradlew assembleDebug`
  - `QT_ANDROID_DIR=/home/cagnulein/Qt/5.15.0/android ./gradlew bundleDebug`

### 2026-05-04 11. Disk pressure cleanup and workflow hardening

Status: done

- Removed the no-longer-needed local Qt Android `5.15.2` installation:
  - `/home/cagnulein/Qt/5.15.2`
- Removed stale generated Android outputs to recover disk space before the next
  validation round.
- Confirmed the workspace returned to usable free space after cleanup.
- Hardened Android CI packaging steps in `.github/workflows/main.yml`:
  - aligned remaining Android jobs from Qt `5.15.2` to `5.15.0`
  - changed the generated `gradle.properties` cleanup from an exact-match `sed`
    to a pattern that removes any
    `android.bundle.enableUncompressedNativeLibs=...` assignment with optional
    whitespace
- Reason for the hardening:
  - a GitHub Actions run still failed in `Build Android package` with
    `The option 'android.bundle.enableUncompressedNativeLibs' is deprecated`
    even though a cleanup step was present; the exact-match pattern was too
    brittle

### 2026-05-04 12. GitHub Actions runner Gradle isolation

Status: done

- A subsequent GitHub Actions Android run still failed in `Build Android package`
  after the generated project cleanup had already switched to a broader `sed`
  pattern.
- The decisive clue was the wrapper download location in the CI log:
  - `/home/runner/.gradle/wrapper/...`
- That means the job was still using the runner-global Gradle user home rather
  than an isolated workspace-local one, so the deprecated property could still
  be injected from runner state outside the generated Android project.
- Updated Android CI packaging to:
  - set `GRADLE_USER_HOME=${{ github.workspace }}/.gradle-android`
  - cache `${{ github.workspace }}/.gradle-android/{caches,wrapper}`
  - remove any `${GRADLE_USER_HOME}/gradle.properties` before invoking Gradle
  - keep the generated-project `gradle.properties` cleanup in place
  - run `./gradlew --no-daemon assembleDebug bundleDebug`

### 2026-05-04 13. GitHub Actions wrapper override hardening

Status: done

- A later GitHub Actions Android run showed that the wrapper still downloaded to
  `/home/runner/.gradle/wrapper/...` despite the exported
  `GRADLE_USER_HOME=${{ github.workspace }}/.gradle-android`.
- That means the environment export alone was not enough to force the wrapper
  off the runner-global location during the workflow execution.
- Updated Android CI packaging again to:
  - invoke Gradle as `./gradlew -g "$GRADLE_USER_HOME" --no-daemon ...`
  - sanitize every generated `gradle.properties` under
    `${{ github.workspace }}/output/android`, not just the root one
- Reason for the change:
  - the CI failure still happens at plugin application time in generated
    `output/android/build.gradle`, so the safe fix is to force the Gradle user
    home explicitly on the command line and scrub the whole generated tree

### 2026-05-04 14. GitHub Actions wrapper JVM override and diagnostics

Status: done

- Another GitHub Actions Android rerun still downloaded the wrapper into
  `/home/runner/.gradle/wrapper/...` even after adding `./gradlew -g ...`.
- Updated the Android packaging jobs once more to hard-force the wrapper JVM
  bootstrap path and expose the generated state in logs:
  - set `GRADLE_OPTS="-Dgradle.user.home=$GRADLE_USER_HOME"`
  - remove `$HOME/.gradle/gradle.properties` before Gradle runs
  - print every generated `gradle.properties` file under `output/android`
    after sanitization
- Reason for the change:
  - the remaining failure is still at AGP plugin application time, so the next
    useful signal is whether the deprecated property is still present anywhere
    in the generated tree or only injected by the runner-global Gradle state

### 2026-05-04 15. GitHub Actions wrapper bypass

Status: done

- Another Android CI rerun still bootstrapped Gradle from
  `/home/runner/.gradle/wrapper/...` even after:
  - `GRADLE_USER_HOME=...`
  - `GRADLE_OPTS=-Dgradle.user.home=...`
  - `./gradlew -g ...`
- At that point the wrapper itself became the unstable piece of the chain.
- Updated the Android packaging jobs to bypass the generated wrapper entirely:
  - download `gradle-8.13-bin.zip` into the job workspace if missing
  - run `${{ github.workspace }}/gradle-8.13/bin/gradle`
  - keep `-g "$GRADLE_USER_HOME"` on the real Gradle invocation
  - keep the generated `gradle.properties` sanitization and diagnostics
- Reason for the change:
  - this removes the last dependency on runner-global wrapper bootstrap behavior
    and makes the Android CI Gradle entrypoint deterministic

### 2026-05-04 16. GitHub Actions residual-property guard

Status: done

- Even after bypassing the wrapper, Android CI still failed with the same AGP 8
  deprecation at plugin application time.
- That means the deprecated property is still present somewhere in the effective
  Gradle inputs, but previous reduced logging was not enough to prove where.
- Updated Android packaging jobs to:
  - sanitize every generated `*.properties`, `*.gradle`, and `*.gradle.kts`
    file under `output/android`
  - scan `output/android`, `$GRADLE_USER_HOME`, and `$HOME/.gradle` for any
    remaining `enableUncompressedNativeLibs` match
  - fail immediately with the residual match list if anything remains
- Reason for the change:
  - the next rerun will either be green or will expose the exact file still
    feeding the deprecated property into AGP

### 2026-05-04 17. GitHub Actions androiddeployqt early-Gradle bypass

Status: done

- A later debug-enabled Android CI run showed that execution never reached the
  post-generation diagnostic line:
  - `Residual enableUncompressedNativeLibs matches after sanitization:`
- That means the failure was happening inside `androiddeployqt` itself, before
  the workflow had any chance to sanitize the generated Gradle files.
- Updated Android packaging jobs to switch from:
  - `androiddeployqt --gradle --aab --no-build`
  to:
  - `androiddeployqt --gradle --aux-mode`
- The `.aab` is still produced by the explicit Gradle step that follows:
  - `assembleDebug bundleDebug`
- Reason for the change:
  - Qt `5.15.0` + modern AGP is tripping on the generated deprecated property
    during `androiddeployqt`'s internal Gradle path, so the safe route is to
    use `androiddeployqt` only for project generation and let the workflow own
    the Gradle build stage entirely

### 2026-05-04 18. GitHub Actions aux-mode libs.xml preseed

Status: done

- The next Android CI run moved past the old AGP 8 deprecation blocker and hit a
  new aux-mode-specific failure:
  - `Cannot find .../output/android/res/values/libs.xml in prepared packaged`
- `src/android/res/values/libs.xml` already exists in the project and is the Qt
  resource template that `androiddeployqt` expects to find in the prepared
  package tree.
- Updated Android packaging jobs to pre-create:
  - `${{ github.workspace }}/output/android/res/values/libs.xml`
  by copying it from:
  - `src/android/res/values/libs.xml`
- Reason for the change:
  - `--aux-mode` avoids the broken internal Gradle path, but it also assumes
    some prepared-package files already exist in the output tree

## Failures / Dead Ends

- One `zwiftplay` build attempt was interrupted manually before completion.
- Intermediate failures during Qt Android validation were resolved in sequence:
  - missing `qt5AndroidDir`
  - Qt AIDL import resolution
  - duplicate Qt resource arrays from `libs.xml`
  - missing `QtNative` class on Java classpath
  - old D8 crash on AGP 7.4
  - AIDL generated Java not included in compilation
  - `BuildConfig.DEBUG` reference issue in `ChannelService.java`
  - `androiddeployqt` generated `android.bundle.enableUncompressedNativeLibs=false`, which AGP 8.13 rejects
  - Qt Android `5.15.0` initially missed installed `QtHttpServer` / `QtSslServer`,
    which caused unresolved link symbols until `qthttpserver` was rebuilt and
    installed against the `5.15.0` Android tree
  - one local Gradle verification attempt for Garmin `2.3.0` failed for an
    environmental reason only: the filesystem was full during dependency
    download (`No space left on device`)
  - sandbox-local Gradle wrapper failures caused by read-only `~/.gradle` and
    blocked network access during wrapper download; those were environmental,
    not project issues

## Pending

- Optional cleanup:
  - move `android:extractNativeLibs` out of `AndroidManifest.xml` into modern build configuration if desired
