#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "mcfbike.h"

class MCFBikeTestData : public BluetoothDeviceTestData {

public:
    MCFBikeTestData() {
        this->addDeviceName("MCF-", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::MCFBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<mcfbike*>(detectedDevice)!=nullptr;
    }
};

