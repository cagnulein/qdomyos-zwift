#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/elliptical.h"

class EllipticalTestData : public BluetoothDeviceTestData {

public:
    EllipticalTestData() : BluetoothDeviceTestData("Elliptical (Abstract)") {}

    bool get_isAbstract() const override { return true; }

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::None; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<elliptical*>(detectedDevice)!=nullptr;
    }
};

