#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "snodebike.h"

class SnodeBikeTestData : public BluetoothDeviceTestData {

public:
    SnodeBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SnodeBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<snodebike*>(detectedDevice)!=nullptr;
    }
};

