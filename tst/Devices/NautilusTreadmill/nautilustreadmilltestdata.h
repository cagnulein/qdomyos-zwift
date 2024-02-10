#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/nautilustreadmill/nautilustreadmill.h"

class NautilusTreadmillTestData : public BluetoothDeviceTestData {

public:
    NautilusTreadmillTestData() : BluetoothDeviceTestData("Nautilus Treadmill") {
        this->addDeviceName("NAUTILUS T", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::NautilusTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nautilustreadmill*>(detectedDevice)!=nullptr;
    }
};

