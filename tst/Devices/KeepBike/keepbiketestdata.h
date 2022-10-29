#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "keepbike.h"

class KeepBikeTestData : public BluetoothDeviceTestData {

public:
    KeepBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::KeepBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<keepbike*>(detectedDevice)!=nullptr;
    }
};

