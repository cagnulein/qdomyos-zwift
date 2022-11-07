#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "sportstechbike.h"

class SportsTechBikeTestData : public BluetoothDeviceTestData {

public:
    SportsTechBikeTestData() : BluetoothDeviceTestData("Sport Tech Bike") {
        this->addDeviceName("EW-BK", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SportsTechBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<sportstechbike*>(detectedDevice)!=nullptr;
    }
};

