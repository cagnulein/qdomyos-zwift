#ifndef MOCKBLUETOOTH_H
#define MOCKBLUETOOTH_H

#include "bluetooth.h"
#include "mockbluetoothdevice.h"

// Minimal mock bluetooth for testing trainprogram
class MockBluetooth : public bluetooth {
    Q_OBJECT

public:
    MockBluetooth() : bluetooth(false, false) {
        // Create a mock treadmill device
        mockDevice = new MockTreadmill();
    }

    ~MockBluetooth() {
        delete mockDevice;
    }

    // Return our mock device
    bluetoothdevice* device() override { return mockDevice; }

private:
    MockTreadmill* mockDevice;
};

#endif // MOCKBLUETOOTH_H
