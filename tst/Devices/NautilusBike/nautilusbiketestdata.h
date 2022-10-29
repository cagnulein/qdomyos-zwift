#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "nautilusbike.h"

class NautilusBikeTestData : public BluetoothDeviceTestData {

public:
    NautilusBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NautilusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nautilusbike*>(detectedDevice)!=nullptr;
    }
};

