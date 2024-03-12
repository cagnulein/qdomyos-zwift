#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/elliptical.h"

class EllipticalTestData : public BluetoothDeviceTestData {

public:
    EllipticalTestData(std::string deviceName) : BluetoothDeviceTestData(deviceName) {}

    deviceType get_expectedDeviceType() const override { return deviceType::None; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<elliptical*>(detectedDevice)!=nullptr;
    }
};

