## Project Overview

QDomyos-Zwift is a Qt-based application that bridges fitness equipment (treadmills, bikes, ellipticals, rowers) with virtual training platforms like Zwift. It acts as a Bluetooth intermediary, connecting physical equipment to fitness apps while providing enhanced features like Peloton integration, power zone training, and workout programs.

## Rules
1. Ask, dont assume. If something's unclear, ask before writing a line and no silent guesses about intent, architecture, or requirements.
2. Simplest solution first and implement the minimum thing that works. No abstractions you didn't request.
3. Dont touch unrelated code and if a file isnt part of the current task, leave it.
4. Flag uncertainty explicitly or if you're not confident, say so before proceeding as confidence without certainty causes more damage than admitting a gap.

## Architecture Overview

### Device Architecture
The application follows a hierarchical device architecture:

1. **Base Class**: `bluetoothdevice` - Abstract base for all fitness devices
   - Manages Bluetooth connectivity via Qt's QLowEnergyController
   - Defines common metrics (speed, cadence, heart rate, power, distance)
   - Integrates with virtual devices for app connectivity

2. **Device Type Classes**: Inherit from `bluetoothdevice`
   - `bike` - Bike-specific features (resistance, gears, power zones)
   - `treadmill` - Treadmill features (speed control, inclination, pace)
   - `elliptical` - Combined bike/treadmill features
   - `rower` - Rowing metrics (stroke count, 500m pace)
   - `stairclimber` - Step counting and climbing metrics
   - `jumprope` - Jump sequence tracking

3. **Concrete Implementations**: Inherit from device type classes
   - Located in `src/devices/[devicename]/` folders
   - Examples: `domyosbike`, `pelotonbike`, `ftmsbike`

### Virtual Device System
- `virtualdevice` - Abstract base for virtual representations
- `virtualbike`, `virtualtreadmill`, etc. - Advertise to external apps
- Enables bidirectional communication between physical and virtual devices

### Bluetooth Management
- `bluetooth` class acts as device factory and connection manager
- `discoveryoptions` configures device discovery process
- Supports multiple connection types (Bluetooth LE, TCP, UDP)

## Key Development Areas

### Adding New Device Support
1. Create device folder in `src/devices/[devicename]/`
2. Implement device class inheriting from appropriate base type
3. Add device detection logic to `bluetooth.cpp`
4. Update `qdomyos-zwift.pri` with new source files
5. Add tests in `tst/Devices/` following existing patterns

### Adding Device Detection to bluetooth.cpp

**CRITICAL: Always verify device pattern conflicts before adding to bluetooth.cpp**

When adding a new device pattern to `src/devices/bluetooth.cpp`, you **MUST** follow these verification steps:

1. **Search for Similar Patterns**: Use grep/search to find all existing device patterns that might conflict
   - Search for device name prefixes (e.g., if adding "KS-NG-", search for all "KS-" patterns)
   - Check patterns in all device type cases (bikes, treadmills, ellipticals, rowers, etc.)

2. **Analyze Pattern Specificity**: Understand the pattern hierarchy
   - More specific patterns should be checked BEFORE less specific ones
   - Example: "KS-NGCH-" is more specific than "KS-NG-"
   - The order matters: devices are matched by the FIRST matching pattern in the if-else chain

3. **Check Case Order**: Verify the order of device type cases in bluetooth.cpp
   - Earlier cases take precedence over later cases
   - Ensure more specific patterns in earlier cases won't prevent your pattern from matching
   - Ensure your pattern won't incorrectly match devices intended for other cases

4. **Document Conflicts**: When conflicts exist, verify they are intentional
   - More specific patterns earlier in the chain should catch specific devices
   - Your pattern should only catch devices not matched by more specific patterns
   - Example: "KS-NGCH-X21C" (kingsmithR2Treadmill) should match before "KS-NG-" (horizontreadmill)

5. **Test Pattern Matching**: Consider these scenarios
   - Will your pattern match the intended device? (e.g., "KS-NG-X218")
   - Will it incorrectly match other devices? (e.g., "KS-NGCH-X21C")
   - Are there existing patterns that would match your device first?

**Example Verification Process:**

```bash
# Search for similar patterns
grep -n "KS-" src/devices/bluetooth.cpp

# Review each match for conflicts
# - kingsmithR2Treadmill has "KS-NGCH-X21C" (line 1323)
# - horizontreadmill has "KS-MC" (line 1562)
# - Adding "KS-NG-" to horizontreadmill is safe because:
#   1. "KS-NGCH-" patterns are more specific
#   2. kingsmithR2Treadmill case comes first (line 1312 vs 1560)
#   3. "KS-NG-X218" won't match "KS-NGCH-" patterns
```

**Common Pitfalls:**
- Adding a pattern without checking existing patterns
- Not considering pattern order in the if-else chain
- Adding overly broad patterns that match unintended devices
- Not testing with actual device names

### Characteristics & Protocols
- Bluetooth characteristics handlers in `src/characteristics/`
- FTMS (Fitness Machine Service) protocol support
- ANT+ integration for sensors
- Custom protocol implementations for specific brands

### UI & QML
- QML-based UI with Qt Quick Controls 2
- Main QML files in `src/` (main.qml, settings.qml, etc.)
- Platform-specific UI adaptations (iOS, Android, desktop)

### Integration Features
- Peloton workout/resistance integration (`peloton.cpp`)
- Zwift workout parsing (`zwiftworkout.cpp`)
- GPX file support for route following (`gpx.cpp`)
- Training program support (ZWO, XML formats)

## Platform-Specific Notes

### iOS
- Swift bridge files in `src/ios/`
- Apple Watch integration via `WatchKitConnection.swift`
- HealthKit integration for fitness data
- ConnectIQ SDK for Garmin devices

### Android
- Java bridge files in `src/android/src/`
- ANT+ integration via Android ANT SDK
- Foreground service for background operation
- USB serial support for wired connections

#### Building a debug APK for on-device testing (Waydroid / real device)

Use `tools/build_android_debug_aab.sh` as the reference recipe (qmake → make →
`make install` → `androiddeployqt --aux-mode --no-build` → gradle). Known
Qt/NDK toolchain on this machine: `QT_ANDROID_DIR=/home/cagnulein/qt5.15.0-android-page16/usr/local/Qt-5.15.0`,
`ANDROID_SDK_ROOT=/home/cagnulein/Android/Sdk`, NDK `ndk/21.3.6528147`.

**Gotchas that will silently produce a broken/crashing APK if skipped** (each one cost 15-30+ min to
diagnose the hard way — check these BEFORE spending time in logcat):

1. **`ANDROID_ABIS=<one-abi>` is ignored by the top-level `make`/`all` target** — qmake's generated
   Makefile always aggregates `armeabi-v7a-all arm64-v8a-all x86-all x86_64-all` regardless of the
   override. To build only the ABI you need, invoke the specific submake target directly, e.g.
   `make x86_64-all` (`make INSTALL_ROOT=... install` will *also* silently rebuild all 4 ABIs — no
   known workaround short of patching the generated Makefile). Check the device's actual ABI first:
   `adb shell getprop ro.product.cpu.abi`.
2. **`src/translations/*.qm` are untracked build artifacts**, not present in a fresh `git worktree`
   checkout — `qrc_translations.cpp` will fail with `No rule to make target '...qdomyos-zwift_vi.qm'`.
   Copy them from an existing checkout (`cp .../src/translations/*.qm <worktree>/src/translations/`)
   before running qmake/make.
3. **`androiddeployqt --aux-mode` on this Qt/NDK combo does NOT reliably bundle the Qt runtime
   `.so`s or the QML plugin RCC bundle**, even though it logs no error. Two distinct silent
   failures, each with a different symptom:
   - Missing `lib/<abi>/libQt5*.so` (Charts, Widgets, Gui, ...) → APK install succeeds but the app
     crashes instantly with `UnsatisfiedLinkError: libQt5Charts_<abi>.so not found`.
   - Missing `assets/android_rcc_bundle.rcc` (androiddeployqt logs `Skipping createRCC`) → the app
     does NOT crash with a native error; instead `QQmlApplicationEngine` fails to resolve *every*
     QML module (`module "QtQuick.Controls" is not installed`, etc.), `engine.load()` produces an
     **empty** root object list, and `homeform`'s constructor immediately dereferences
     `engine->rootObjects().constFirst()` → `Q_ASSERT`/`abort()` with a backtrace that only shows
     `QList::first()` and `homeform::homeform(...)`, giving *zero* hint about the real QML cause.
   - **Fix**: reuse `libs/<abi>/*.so` and `assets/android_rcc_bundle.rcc` from any previously
     successful `android-build` output directory on the same machine (same Qt version → same Qt
     module/plugin binaries, safe to copy) instead of relying on androiddeployqt to regenerate them.
   - **Root-causing tip**: the custom message handler in `src/main.cpp` (`myMessageOutput`) swallows
     ALL `qWarning`/`qDebug` (including QML load errors) unless the `log_debug` QSettings key is
     `true` — so QML errors are invisible in `adb logcat` by default. Temporarily force
     `logdebug = true` there to see them, then revert.
4. **`gradlew assembleDebug` requires Java 17**, not 11, despite CI using
   `temurin 11.0.23` (AGP 8.13 requirement) — `export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64`.
5. **Waydroid containers have no functional Bluetooth adapter**
   (`adb shell dumpsys bluetooth_manager` shows `enabled: false`, un-fixable via
   `adb shell svc bluetooth enable`). `QBluetoothDeviceDiscoveryAgent::start()` is created (a virtual
   adapter object exists) but its scan never emits `finished()`, so `bluetooth::finished()` — which is
   what actually activates `Fake Device`/`Fake Treadmill`/etc — never runs, and the app hangs on
   "Connecting..." forever. Real Android devices/emulators with working BT don't hit this.
6. **`adb shell run-as` does not work on Waydroid** (`setegid(AID_PACKAGE_INFO) failed`), so editing
   `qDomyos-Zwift.conf` directly is unavailable there — drive settings through the UI instead
   (`adb shell input tap/swipe`).
7. **QtWebView `<canvas>` content (chartjs live charts) doesn't visually repaint on Waydroid after
   the first paint**, even though the underlying JS/Chart.js state is provably correct and updating.
   Verified by instrumenting `dotreadmillchartlive.js` with `console.log` of the dataset array,
   `chart.scales.x.getPixelForValue()`, and `canvas.width/height` on every tick: the array keeps
   growing every second with correct x/y values and correct computed pixel positions, `chart.update()`
   (both animated and `'none'` mode) runs with no JS error — yet `adb shell screencap` **and**
   `adb shell screenrecord` (a different capture path, rules out a screencap-specific bug) both show
   the canvas frozen at whatever it looked like a few seconds after the chart first opened. A manual
   touch/swipe on the WebView momentarily forces a real repaint (briefly shows the correct up-to-date
   line) before freezing again. This means: don't trust `screencap`/`screenrecord` snapshots as proof
   a Chart.js-based live chart is broken on this environment — instrument the JS directly
   (`console.log` piped through `adb logcat | grep chromium`) to check the actual data/redraw state,
   and treat visual confirmation as only possible on a real Android device.

### Windows
- ADB integration for Nordic Track iFit devices
- PaddleOCR integration for Zwift workout detection
- Windows-specific networking features

## File Structure Patterns

### Device Files
```
src/devices/[devicename]/
├── [devicename].h        # Header file
├── [devicename].cpp      # Implementation
└── README.md            # Device-specific documentation (optional)
```

### Test Files
```
tst/Devices/
├── DeviceTestData.h     # Test data definitions
├── Test[DeviceName].h   # Device-specific test cases
└── TestBluetooth.cpp    # Main device detection test suite
```

## Testing Framework

- Uses Google Test (gtest) with Google Mock
- Comprehensive device detection testing
- Configuration-based test scenarios
- XML output support for CI/CD integration
- Tests must be built after main application (links against libqdomyos-zwift.a)

## Configuration & Settings

- Settings managed via `qzsettings.cpp` (QSettings wrapper)
- Platform-specific configuration paths
- Profile system for multiple users/devices
- Extensive customization options for device behavior
- `src/settings-catalog.json` is the manually maintained cross-platform catalog of every persistent setting declared in `src/settings.qml` and its settings child QML files. When adding, removing, or renaming any settings QML property, update `src/settings-catalog.json` by hand in the same change. Keep `settingCount` synchronized and add new settings at the end of the catalog.

## External Dependencies

- Qt 5.15.2+ (Bluetooth, WebSockets, Charts, Quick, etc.)
- Google Test (submodule for testing)
- Platform SDKs (Android ANT+, iOS HealthKit, Windows ADB)
- Protocol Buffers for Zwift API integration
- MQTT client for IoT integration
- Various fitness platform APIs (Strava, Garmin Connect, etc.)

## Adding New ProForm Treadmill Models

This section provides a complete guide for adding new ProForm treadmill models to the codebase, based on the ProForm 995i implementation.

### Prerequisites

1. **Bluetooth Frame Capture File**: A file containing raw Bluetooth frames from the target treadmill
2. **Frame Analysis**: Understanding of which frames are initialization vs. sendPoll frames
3. **BLE Header Knowledge**: Each frame has an 11-byte BLE header that must be removed

### Step-by-Step Implementation Process

#### 1. Process Bluetooth Frames

First, process the raw Bluetooth frames by removing the first 11 bytes (BLE header) from each frame:

```bash
# Example: if you have "proform_model.c" with raw frames
# Process each frame by removing first 11 bytes
# Separate initialization frames from sendPoll frames
```

**Key Requirements:**
- Remove exactly 11 bytes from each frame (BLE header)
- Identify the boundary between initialization and sendPoll frames
- Initialization frames come first, sendPoll frames follow
- Document which packet number starts the sendPoll sequence

#### 2. Add Boolean Flag to Header File

Add the new model flag to `src/devices/proformtreadmill/proformtreadmill.h`:

```cpp
// Add before #ifdef Q_OS_IOS section
bool proform_treadmill_newmodel = false;
```

#### 3. Add Settings Support

Update the following files for settings integration:

**In `src/qzsettings.h`:**
```cpp
static const QString proform_treadmill_newmodel;
static constexpr bool default_proform_treadmill_newmodel = false;
```

**In `src/qzsettings.cpp`:**
```cpp
const QString QZSettings::proform_treadmill_newmodel = QStringLiteral("proform_treadmill_newmodel");
```

* Update the `allSettingsCount` in `qzsettings.cpp`

#### 4. Update QML Settings UI

**In `src/settings.qml`:**

1. Add property at the END of properties list:
```qml
property bool proform_treadmill_newmodel: false
```
 
2. Update ComboBox model array:
```qml
model: ["Disabled", "Proform New Model", ...]
```

3. Add case selection logic (find next available case number):
```qml
currentIndex: settings.proform_treadmill_newmodel ? XX : 0;
```

4. Add reset logic:
```qml
settings.proform_treadmill_newmodel = false;
```

5. Add switch case:
```qml
case XX: settings.proform_treadmill_newmodel = true; break;
```

#### 5. Implement Device Logic

**In `src/devices/proformtreadmill/proformtreadmill.cpp`:**

1. **Load Settings** (in constructor):
```cpp
proform_treadmill_newmodel = settings.value(QZSettings::proform_treadmill_newmodel, QZSettings::default_proform_treadmill_newmodel).toBool();
```

2. **Add Initialization Case** (in `btinit()` method):
```cpp
} else if (proform_treadmill_newmodel) {
    // ALL initialization frames go here
    uint8_t initData1[] = {0x00, 0xfe, 0x02, 0x08, 0x02};
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    // ... continue with ALL init frames from capture file
    // Use frames from beginning until sendPoll boundary
}
```

3. **Add SendPoll Case** (in `sendPoll()` method):
```cpp
} else if (proform_treadmill_newmodel) {
    switch (counterPoll) {
        case 0:
            // First sendPoll frame
            break;
        case 1:
            // Second sendPoll frame  
            break;
        // ... continue with pattern from sendPoll frames
        default:
            // Reset counter and cycle
            counterPoll = -1;
            break;
    }
}
```

4. **Update Force Functions** - Add flag to conditional checks in `forceIncline()` and `forceSpeed()`:
```cpp
} else if (proform_treadmill_8_0 || ... || proform_treadmill_newmodel) {
    write[14] = write[11] + write[12] + 0x12;
}
```

### Implementation Requirements

#### Frame Processing Rules
- **Exactly 11 bytes** must be removed from each frame (BLE header)
- **All initialization frames** must be included in the btinit() case
- **All sendPoll frames** must be included in the sendPoll() switch statement
- **Frame order** must be preserved exactly as captured

#### Settings Integration Rules
- **Property placement**: Always add new properties at the END of the properties list in settings.qml
- **Case numbering**: Find the next available case number in the ComboBox switch statement
- **Naming convention**: Use descriptive names following existing patterns

#### Code Organization Rules
- **Initialization**: All init frames go in btinit() method
- **Communication**: All sendPoll frames go in sendPoll() method with switch/case structure
- **Force functions**: Add new model flag to existing conditional chains

### Common Pitfalls and Solutions

#### Incorrect Byte Removal
- **Problem**: Removing wrong number of bytes (12 instead of 11)
- **Solution**: Always remove exactly 11 bytes (BLE header)

#### Wrong SendPoll Boundary  
- **Problem**: Using initialization frames in sendPoll logic
- **Solution**: Identify exact packet number where sendPoll starts

#### Incomplete Initialization
- **Problem**: Missing initialization frames
- **Solution**: Include ALL frames from start until sendPoll boundary

#### Settings Placement
- **Problem**: Adding property in wrong location in settings.qml
- **Solution**: Always add at END of properties list

### Verification Checklist

- [ ] All 11 bytes removed from each frame
- [ ] Initialization frames correctly identified and included
- [ ] SendPoll frames correctly identified and implemented
- [ ] Settings properly integrated in all required files
- [ ] ComboBox updated with new model option
- [ ] Force functions updated with new model flag
- [ ] Property added at END of settings.qml properties list

### Example Reference

The ProForm 995i implementation serves as the reference example:
- 25 initialization frames (pkt4658-pkt4756)
- 33 sendPoll frames (pkt4761-pkt4897) 
- 6-case sendPoll switch statement with cycling logic
- Complete settings integration across all required files

## Development Tips

- Use Qt Creator for development with proper project file support
- The project uses Qt's signal/slot mechanism extensively
- Device implementations should follow existing patterns for consistency
- Add comprehensive logging using the project's logging framework
- Test device detection thoroughly using the existing test infrastructure
- Consider platform differences when adding new features

## Updating Version Numbers

When releasing a new version of QDomyos-Zwift, you must update the version number in **3 files**:

### 1. Android Manifest
**File**: `src/android/AndroidManifest.xml`

Update both `versionName` and `versionCode`:
```xml
<manifest ... android:versionName="X.XX.XX" android:versionCode="XXXX" ...>
```

- `versionName`: The human-readable version (e.g., "2.20.26")
- `versionCode`: Integer build number that must be incremented (e.g., 1274)

### 2. Main QML File
**File**: `src/main.qml`

Update the version text displayed in the UI (around line 938):
```qml
ItemDelegate {
    text: "version X.XX.XX"
    width: parent.width
}
```

### 3. Qt Project Include File
**File**: `src/qdomyos-zwift.pri`

Update the VERSION variable (around line 1011):
```pri
VERSION = X.XX.XX
```

### Version Numbering Convention

- **Major.Minor.Patch** format (e.g., 2.20.26)
- **Build number** must always increment, never reuse
- Update all 3 files together to keep versions synchronized

### iOS Version (Optional)

iOS version is managed through Xcode project variables:
- `MARKETING_VERSION` in project.pbxproj (corresponds to versionName)
- `CURRENT_PROJECT_VERSION` in project.pbxproj (corresponds to versionCode)

These are typically updated via Xcode IDE rather than manually editing files.

## Additional Memories

- When adding a new setting in QML (setting-tiles.qml), you must:
  * Add the property at the END of the properties list
