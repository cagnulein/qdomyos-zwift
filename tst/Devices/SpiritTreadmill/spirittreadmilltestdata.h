#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "spirittreadmill.h"

class SpiritTreadmillTestData : public BluetoothDeviceTestData {

public:
    SpiritTreadmillTestData(): BluetoothDeviceTestData("Spirit Treadmill") {

        this->addDeviceName("XT385", comparison::StartsWithIgnoreCase);
        this->addDeviceName("XT485", comparison::StartsWithIgnoreCase);
        this->addDeviceName("XT800", comparison::StartsWithIgnoreCase);
        this->addDeviceName("XT900", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SpiritTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<spirittreadmill*>(detectedDevice)!=nullptr;
    }
};

