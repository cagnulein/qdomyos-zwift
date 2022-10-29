#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "elliptical.h"

class EllipticalTestData : public BluetoothDeviceTestData {

public:
    EllipticalTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::Elliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<elliptical*>(detectedDevice)!=nullptr;
    }
};

