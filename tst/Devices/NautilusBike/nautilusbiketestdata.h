#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/nautilusbike/nautilusbike.h"

class NautilusBikeTestData : public BluetoothDeviceTestData {

public:
    NautilusBikeTestData(): BluetoothDeviceTestData("Nautilus Bike") {
        this->addDeviceName("NAUTILUS B", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NautilusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nautilusbike*>(detectedDevice)!=nullptr;
    }
};

