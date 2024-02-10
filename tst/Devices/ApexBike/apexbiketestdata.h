#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/apexbike/apexbike.h"

class ApexBikeTestData : public BluetoothDeviceTestData {

public:
    ApexBikeTestData() : BluetoothDeviceTestData("Apex Bike") {
        this->addDeviceName("WLT8266BM", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ApexBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<apexbike*>(detectedDevice)!=nullptr;
    }
};

