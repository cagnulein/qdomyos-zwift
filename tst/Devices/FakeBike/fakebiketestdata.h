#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fakebike.h"

class FakeBikeTestData : public BluetoothDeviceTestData {

public:
    FakeBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fakebike*>(detectedDevice)!=nullptr;
    }
};

