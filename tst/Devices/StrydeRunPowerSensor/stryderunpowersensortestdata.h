#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "strydrunpowersensor.h"

class StrydeRunPowerSensorTestData : public BluetoothDeviceTestData {
private:
    QString powerSensorName;
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        if(enable) {
            // power_as_treadmill enabled and powerSensorName in settings matches device name
            DeviceDiscoveryInfo config(info);
            config.power_as_treadmill = true;
            config.powerSensorName = this->powerSensorName;
            configurations.push_back(config);
        } else {
            // enabled but powerSensorName in settings does not match device name
            DeviceDiscoveryInfo config(info);
            config.power_as_treadmill = true;
            config.powerSensorName = "NOT " +this->powerSensorName;
            configurations.push_back(config);

            // disabled with non-matching name
            config.power_as_treadmill = false;
            config.powerSensorName = "NOT " +this->powerSensorName;
            configurations.push_back(config);

            // disabled with matching name
            config.power_as_treadmill = false;
            config.powerSensorName = this->powerSensorName;
            configurations.push_back(config);
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

class ZwiftRunpodTestData : public BluetoothDeviceTestData {
private:
    QString powerSensorName;
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        DeviceDiscoveryInfo config(info);

        if(enable) {
            /* Avoid the config that enables the StrydeRunPowerSensorTestData device
            // power_as_treadmill enabled and powerSensorName in settings matches device name
            config.power_as_treadmill = true;
            config.powerSensorName = this->powerSensorName;
            configurations.push_back(config);
            */

            /*
             * In order for the search to occur, the power sensor name must start with "Disabled", or
             * power_as_bike or power_as_treadmill must be true.
            */

            config.power_as_treadmill = true;
            config.powerSensorName = "NOT " + this->powerSensorName;
            configurations.push_back(config);

            config.power_as_treadmill = false;
            config.powerSensorName = "Disabled";
            configurations.push_back(config);

        } else {
            // disable the search
            config.power_as_treadmill = false;
            config.powerSensorName = this->powerSensorName;
            config.power_as_bike = false;
            configurations.push_back(config);
        }
    }
public:
    ZwiftRunpodTestData() : BluetoothDeviceTestData("Zwift Runpod") {
        this->powerSensorName = "WattsItCalled-";
        this->addDeviceName("ZWIFT RUNPOD", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PowerTreadmill_StrydrunPowerSensor; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<strydrunpowersensor*>(detectedDevice)!=nullptr;
    }
};

