#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "flywheelbike.h"

class FlywheelBikeTestData : public BluetoothDeviceTestData {

public:
    FlywheelBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FlywheelBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<flywheelbike*>(detectedDevice)!=nullptr;
    }
};

