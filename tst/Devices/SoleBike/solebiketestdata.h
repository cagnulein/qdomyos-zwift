#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "solebike.h"

class SoleBikeTestData : public BluetoothDeviceTestData {

public:
    SoleBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<solebike*>(detectedDevice)!=nullptr;
    }
};

