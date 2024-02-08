#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/solebike/solebike.h"

class SoleBikeTestData : public BluetoothDeviceTestData {

public:
    SoleBikeTestData() : BluetoothDeviceTestData("Sole Bike") {
        this->addDeviceName("LCB", comparison::StartsWithIgnoreCase);
        this->addDeviceName("R92", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<solebike*>(detectedDevice)!=nullptr;
    }
};

