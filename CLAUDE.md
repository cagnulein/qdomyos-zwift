# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

QDomyos-Zwift is a Qt-based application that bridges fitness equipment (treadmills, bikes, ellipticals, rowers) with virtual training platforms like Zwift. It acts as a Bluetooth intermediary, connecting physical equipment to fitness apps while providing enhanced features like Peloton integration, power zone training, and workout programs.

## Build System & Commands

### Build Commands
```bash
# Build entire project (use subdirs TEMPLATE)
qmake
make

# Build specific configurations
qmake -r  # Recursive build
make debug    # Debug build
make release  # Release build

# Clean build
make clean
make distclean
```

### Platform-Specific Builds
```bash
# Android
qmake -spec android-clang
make

# iOS 
qmake -spec macx-ios-clang
make

# Windows (MinGW)
qmake -spec win32-g++
make
```

### Testing
```bash
# Build and run tests (requires main app built first)
cd tst
qmake
make
./qdomyos-zwift-tests

# Run with XML output for CI
GTEST_OUTPUT=xml:test-results/ GTEST_COLOR=1 ./qdomyos-zwift-tests
```

### No-GUI Mode
```bash
# Run application without GUI
sudo ./qdomyos-zwift -no-gui
```

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

## Adding ProForm Bike Models

This section documents the process for adding new ProForm bike models to the `proformbike` class.

### Protocol Overview

ProForm bikes use a proprietary Bluetooth protocol similar to NordicTrack iFit devices. The protocol consists of:
- **Initialization sequence**: Device-specific frames sent during connection
- **Polling cycle**: Repeating sequence of frames sent periodically
- **Resistance commands**: Frames to change bike resistance (each model has unique frames)

### Key Implementation Points

1. **Frame extraction**: Use `tshark` to extract frames from `btsnoop_hci.log`
2. **BLE header removal**: Remove exactly 11 bytes from each frame before use
3. **Resistance mapping**: QZ internal levels (e.g., 1-10) map to physical resistance values
4. **Unique resistance packets**: Each ProForm bike model has different resistance packets

### Example: ProForm 325 CSX PFEX439210INT.0

**Resistance Mapping** (10 QZ levels):
- QZ Level 1 → Physical Resistance 4
- QZ Level 2 → Physical Resistance 6
- ...
- QZ Level 10 → Physical Resistance 22

The iFit app changes resistance from 4 to 22 in increments of 2, mapping to 10 QZ internal levels.

## Adding ESLinker/Treadmill Linker Devices

The `eslinkertreadmill` class supports multiple treadmill variants using a type enum system.

### Supported Protocol Types

```cpp
typedef enum TYPE {
    RHYTHM_FUN = 0,
    CADENZA_FITNESS_T45 = 1,
    YPOO_MINI_CHANGE = 2,
    COSTAWAY = 3,
    ESANGLINKER = 4,
    TREADMILL_LINKER = 5,    // Similar to ESANGLINKER but uses km/h instead of mph
} TYPE;
```

### Key Protocol Differences

| Feature | ESANGLINKER | TREADMILL_LINKER |
|---------|-------------|------------------|
| Speed units | mph (multiply by 0.621371) | km/h (no conversion) |
| minStepSpeed | 0.160934 (0.1 mi) | 0.1 (km/h) |
| Initialization | 13 frames with handshake/pairing | 6 frames with pairing |

### Adding a New Variant

1. **Add enum type** to `eslinkertreadmill.h`
2. **Add device name detection** in `deviceDiscovered()` and `bluetooth.cpp`
3. **Implement btinit()** with device-specific initialization frames
4. **Implement forceSpeed()** - verify if device uses mph or km/h
5. **Update characteristicChanged()** parser for telemetry data
6. **Update helper functions** (`minStepSpeed()`, `updateDisplay()`)

### Common Frame Patterns

Frames starting with `0xa9`:
- `0xa9 0xf2`: Initialization/wakeup
- `0xa9 0x08`: Pairing/handshake
- `0xa9 0x01`: Speed command
- `0xa9 0xa0`: Polling/keep-alive
- `0xa9 0xa3`: Start/stop command
- `0xa9 0xe0`: Telemetry data

**Checksum**: Most commands use XOR checksum as last byte

## Additional Memories

- When adding a new setting in QML (setting-tiles.qml), you must:
  * Add the property at the END of the properties list  