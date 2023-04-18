#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "bike.h"

class BikeTestData : public BluetoothDeviceTestData {
protected:

    void configureLockscreenSettings(const DeviceDiscoveryInfo &info,
                                                   std::vector<LockscreenFunctionsTestData> &configurations) const override {
        DeviceDiscoveryInfo config(info);
        auto bike = QZLockscreenFunctions::configurationType::BIKE;
        bool expectedZwiftMode = this-get_expectedLockscreenZwiftMode();

        // Enabling
        config.ios_peloton_workaround = true;
        config.bike_cadence_sensor = true;
        configurations.push_back(LockscreenFunctionsTestData(bike, true, expectedZwiftMode, config));


        // Disabling
        for(int i=0; i<3; i++) {
            config.ios_peloton_workaround = i&1;
            config.bike_cadence_sensor = i&2;
            configurations.push_back(LockscreenFunctionsTestData(bike, false, false, config));
        }
    }
public:
    BikeTestData(std::string deviceName) : BluetoothDeviceTestData(deviceName) {}

    deviceType get_expectedDeviceType() const override {
        return deviceType::None; // abstract
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bike*>(detectedDevice)!=nullptr;
    }
};

