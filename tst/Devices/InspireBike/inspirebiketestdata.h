#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "inspirebike.h"

class InspireBikeTestData : public BluetoothDeviceTestData {

public:
    InspireBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::InspireBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<inspirebike*>(detectedDevice)!=nullptr;
    }
};

