#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "nautiluselliptical.h"

class NautilusEllipticalTestData : public BluetoothDeviceTestData {

public:
    NautilusEllipticalTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NautilusElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nautiluselliptical*>(detectedDevice)!=nullptr;
    }
};

