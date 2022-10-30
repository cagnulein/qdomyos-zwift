#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "strydrunpowersensor.h"

class StrydeRunPowerSensorTestData : public BluetoothDeviceTestData {

public:
    StrydeRunPowerSensorTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PowerTreadmill_StrydrunPowerSensor; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<strydrunpowersensor*>(detectedDevice)!=nullptr;
    }
};

