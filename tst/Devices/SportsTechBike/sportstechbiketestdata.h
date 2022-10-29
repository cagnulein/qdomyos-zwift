#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "sportstechbike.h"

class SportsTechBikeTestData : public BluetoothDeviceTestData {

public:
    SportsTechBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SportsTechBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<sportstechbike*>(detectedDevice)!=nullptr;
    }
};

