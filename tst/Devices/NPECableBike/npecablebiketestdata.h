#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "npecablebike.h"

class NPECableBikeTestData : public BluetoothDeviceTestData {

public:
    NPECableBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NPECableBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<npecablebike*>(detectedDevice)!=nullptr;
    }
};

