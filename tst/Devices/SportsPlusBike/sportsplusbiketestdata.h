#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "sportsplusbike.h"

class SportsPlusBikeTestData : public BluetoothDeviceTestData {

public:
    SportsPlusBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SportsPlusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<sportsplusbike*>(detectedDevice)!=nullptr;
    }
};

