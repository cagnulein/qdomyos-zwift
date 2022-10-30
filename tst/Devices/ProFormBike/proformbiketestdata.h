#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformbike.h"

class ProFormBikeTestData : public BluetoothDeviceTestData {

public:
    ProFormBikeTestData() {
        this->addDeviceName("I_EB", comparison::StartsWithIgnoreCase);
        this->addDeviceName("I_SB", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformbike*>(detectedDevice)!=nullptr;
    }
};

