#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "treadmill.h"

class TreadmillTestData : public BluetoothDeviceTestData {

public:
    TreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override {
        return deviceType::None; // abstract
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<treadmill*>(detectedDevice)!=nullptr;
    }
};

