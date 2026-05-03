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
  - sandbox-local Gradle wrapper failures caused by read-only `~/.gradle` and
    blocked network access during wrapper download; those were environmental,
    not project issues

## Pending

- Optional cleanup:
  - move `android:extractNativeLibs` out of `AndroidManifest.xml` into modern build configuration if desired
