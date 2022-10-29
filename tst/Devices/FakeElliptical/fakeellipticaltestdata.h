#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fakeelliptical.h"

class FakeEllipticalTestData : public BluetoothDeviceTestData {

public:
    FakeEllipticalTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fakeelliptical*>(detectedDevice)!=nullptr;
    }
};

