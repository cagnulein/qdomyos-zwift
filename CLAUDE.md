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

## Development Tips

- Use Qt Creator for development with proper project file support
- The project uses Qt's signal/slot mechanism extensively
- Device implementations should follow existing patterns for consistency
- Add comprehensive logging using the project's logging framework
- Test device detection thoroughly using the existing test infrastructure
- Consider platform differences when adding new features