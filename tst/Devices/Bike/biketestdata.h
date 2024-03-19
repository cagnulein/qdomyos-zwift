#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/bike.h"

class BikeTestData : public BluetoothDeviceTestData {

public:
    BikeTestData(std::string deviceName) : BluetoothDeviceTestData(deviceName) {}

    deviceType get_expectedDeviceType() const override {
        return deviceType::None; // abstract
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bike*>(detectedDevice)!=nullptr;
    }
};

