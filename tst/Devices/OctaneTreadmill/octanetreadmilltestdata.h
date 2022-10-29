#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "octanetreadmill.h"

class OctaneTreadmillTestData : public BluetoothDeviceTestData {

public:
    OctaneTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::OctaneTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<octanetreadmill*>(detectedDevice)!=nullptr;
    }
};

