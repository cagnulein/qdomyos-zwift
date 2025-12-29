#ifndef MOCKBLUETOOTHDEVICE_H
#define MOCKBLUETOOTHDEVICE_H

#include "bluetoothdevice.h"
#include "treadmill.h"

// Minimal mock treadmill device for testing trainprogram
class MockTreadmill : public treadmill {
    Q_OBJECT

public:
    MockTreadmill() : treadmill() {
        // Initialize with minimal required state
    }

    // Override odometer to return constant 0.0 (no distance-based logic in tests)
    double odometer() override { return 0.0; }

    // Required pure virtual implementations
    bool connected() override { return true; }
    void* VirtualDevice() override { return nullptr; }

    // These are not used in time-based tests but need to be implemented
    void changeSpeed(double /*speed*/) override {}
    void changeInclination(double /*grade*/, double /*inclination*/) override {}
};

#endif // MOCKBLUETOOTHDEVICE_H
