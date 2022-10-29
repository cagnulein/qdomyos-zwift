#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "chronobike.h"

class ChronobikeTestData : public BluetoothDeviceTestData {

public:
    ChronobikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::Chronobike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<chronobike*>(detectedDevice)!=nullptr;
    }
};

