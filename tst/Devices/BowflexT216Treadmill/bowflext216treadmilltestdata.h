#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "bowflext216treadmill.h"

class BowflexT216TreadmillTestData : public BluetoothDeviceTestData {

public:
    BowflexT216TreadmillTestData() : BluetoothDeviceTestData("Bowflex T216 Treadmill") {
        this->addDeviceName("BOWFLEX T", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::BowflexT216Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bowflext216treadmill*>(detectedDevice)!=nullptr;
    }
};

