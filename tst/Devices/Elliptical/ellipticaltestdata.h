#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "elliptical.h"

class EllipticalTestData : public BluetoothDeviceTestData {

public:
    EllipticalTestData() : BluetoothDeviceTestData("Elliptical (Abstract)") {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::None; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<elliptical*>(detectedDevice)!=nullptr;
    }
};

