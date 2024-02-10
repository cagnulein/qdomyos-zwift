#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/inspirebike/inspirebike.h"

class InspireBikeTestData : public BluetoothDeviceTestData {

public:
    InspireBikeTestData() : BluetoothDeviceTestData("Inspire Bike") {
        this->addDeviceName("IC", comparison::StartsWithIgnoreCase, 8);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::InspireBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<inspirebike*>(detectedDevice)!=nullptr;
    }
};

