#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "bowflextreadmill.h"

class BowflexTreadmillTestData : public BluetoothDeviceTestData {

public:
    BowflexTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::BowflexTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bowflextreadmill*>(detectedDevice)!=nullptr;
    }
};

