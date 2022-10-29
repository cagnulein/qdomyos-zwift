#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "bowflext216treadmill.h"

class BowflexT216TreadmillTestData : public BluetoothDeviceTestData {

public:
    BowflexT216TreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::BowflexT216Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bowflext216treadmill*>(detectedDevice)!=nullptr;
    }
};

