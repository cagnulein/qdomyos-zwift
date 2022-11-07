#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "strydrunpowersensor.h"

class StrydeRunPowerSensorTestData : public BluetoothDeviceTestData {
private:
    QString powerSensorName;
protected:
    void configureSettings(const devicediscoveryinfo& info, bool enable, std::vector<devicediscoveryinfo> configurations) const override {
        if(enable) {
            // enabled and powerSensorName in settings matches device name
            devicediscoveryinfo info1(info);
            info1.power_as_treadmill = true;
            info1.powerSensorName = this->powerSensorName;
            configurations.push_back(info1);
        } else {
            // enabled but powerSensorName in settings does not match device name
            devicediscoveryinfo info1(info);
            info1.power_as_treadmill = true;
            info1.powerSensorName = "NOT " +this->powerSensorName;
            configurations.push_back(info1);

            // disabled with non-matching name
            devicediscoveryinfo info2(info);
            info2.power_as_treadmill = false;
            info2.powerSensorName = "NOT " +this->powerSensorName;
            configurations.push_back(info2);

            // disabled with matching name
            devicediscoveryinfo info3(info);
            info3.power_as_treadmill = false;
            info3.powerSensorName = this->powerSensorName;
            configurations.push_back(info3);
        }
    }
public:
    StrydeRunPowerSensorTestData() : BluetoothDeviceTestData("Stryderun Treadmill / Power Sensor") {
        this->powerSensorName = "WattsItCalled-";
        this->addDeviceName(this->powerSensorName, comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PowerTreadmill_StrydrunPowerSensor; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<strydrunpowersensor*>(detectedDevice)!=nullptr;
    }
};

