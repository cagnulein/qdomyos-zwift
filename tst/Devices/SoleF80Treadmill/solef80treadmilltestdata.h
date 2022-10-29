#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "solef80treadmill.h"

class SoleF80TreadmillTestData : public BluetoothDeviceTestData {

public:
    SoleF80TreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleF80Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<solef80treadmill*>(detectedDevice)!=nullptr;
    }
};

