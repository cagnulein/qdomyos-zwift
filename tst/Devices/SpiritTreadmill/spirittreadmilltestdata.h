#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "spirittreadmill.h"

class SpiritTreadmillTestData : public BluetoothDeviceTestData {

public:
    SpiritTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SpiritTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<spirittreadmill*>(detectedDevice)!=nullptr;
    }
};

