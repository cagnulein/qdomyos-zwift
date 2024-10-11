#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/rower.h"

class RowerTestData : public BluetoothDeviceTestData {

public:
    RowerTestData(std::string deviceName) : BluetoothDeviceTestData(deviceName) {}

    deviceType get_expectedDeviceType() const override {
        return deviceType::None; // abstract
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<rower*>(detectedDevice)!=nullptr;
    }
};

