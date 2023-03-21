#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "solef80treadmill.h"

class SoleF80TreadmillTestData : public BluetoothDeviceTestData {

public:
    SoleF80TreadmillTestData() : BluetoothDeviceTestData("Sole F80") {
        this->addDeviceName("F80", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F65", comparison::StartsWithIgnoreCase);
        this->addDeviceName("S77", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TT8", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F63", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ST90", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F85", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleF80Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<solef80treadmill*>(detectedDevice)!=nullptr;
    }
};

