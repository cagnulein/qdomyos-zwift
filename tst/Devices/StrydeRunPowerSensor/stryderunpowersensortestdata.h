#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "strydrunpowersensor.h"

class StrydeRunPowerSensorTestData : public BluetoothDeviceTestData {
    QString powerSensorName;
public:
    StrydeRunPowerSensorTestData() {
        this->hasSettings = true;
        this->powerSensorName = "WattsItCalled-";
        this->addDeviceName(this->powerSensorName, comparison::StartsWith);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.power_as_treadmill = enable;
        info.powerSensorName = enable ? this->powerSensorName:"Disabled";
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PowerTreadmill_StrydrunPowerSensor; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<strydrunpowersensor*>(detectedDevice)!=nullptr;
    }
};

