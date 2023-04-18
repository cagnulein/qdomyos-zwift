#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "rower.h"

class RowerTestData : public BluetoothDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info,
                                     std::vector<LockscreenFunctionsTestData> &configurations) const override {


        DeviceDiscoveryInfo config(info);
        auto none = QZLockscreenFunctions::configurationType::NONE;
        auto bike = QZLockscreenFunctions::configurationType::BIKE;
        auto rower = QZLockscreenFunctions::configurationType::ROWER;
        auto expectedZwiftMode = this->get_expectedLockscreenZwiftMode();

        for(int i=0; i<8; i++) {
            config.ios_peloton_workaround = i&1;
            config.bike_cadence_sensor = i&2;
            config.virtual_device_rower = i&4;

            // generally rowers have no enabling configuations for the peloton workaround
            bool enabled = false; //config.ios_peloton_workaround && config.bike_cadence_sensor && !config.virtual_device_rower;

            configurations.push_back(LockscreenFunctionsTestData(none, enabled ? bike:none, enabled, enabled && expectedZwiftMode, config));
        }
    }
public:
    RowerTestData(std::string deviceName) : BluetoothDeviceTestData(deviceName) {}

    deviceType get_expectedDeviceType() const override {
        return deviceType::None; // abstract
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<rower*>(detectedDevice)!=nullptr;
    }
};

