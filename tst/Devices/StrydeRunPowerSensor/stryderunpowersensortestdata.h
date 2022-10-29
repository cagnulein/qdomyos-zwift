#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "stryderunpowersensor.h"

class StrydeRunPowerSensorTestData : public BluetoothDeviceTestData {

public:
    StrydeRunPowerSensorTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::StrydeRunPowerSensor; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<stryderunpowersensor*>(detectedDevice)!=nullptr;
    }
};

