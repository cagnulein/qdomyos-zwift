#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/treadmill.h"

class TreadmillTestData : public BluetoothDeviceTestData {

public:
    TreadmillTestData(std::string deviceName): BluetoothDeviceTestData(deviceName) {}


    deviceType get_expectedDeviceType() const override {
        return deviceType::None; // abstract
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<treadmill*>(detectedDevice)!=nullptr;
    }
};

