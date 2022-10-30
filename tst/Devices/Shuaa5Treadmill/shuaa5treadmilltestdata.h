#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "shuaa5treadmill.h"

class Shuaa5TreadmillTestData : public BluetoothDeviceTestData {

public:
    Shuaa5TreadmillTestData() {
        this->addDeviceName("ZW-", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::Shuaa5Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<shuaa5treadmill*>(detectedDevice)!=nullptr;
    }
};

