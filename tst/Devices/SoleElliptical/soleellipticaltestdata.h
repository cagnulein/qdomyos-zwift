#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "soleelliptical.h"

class SoleEllipticalTestData : public BluetoothDeviceTestData {

public:
    SoleEllipticalTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<soleelliptical*>(detectedDevice)!=nullptr;
    }
};

