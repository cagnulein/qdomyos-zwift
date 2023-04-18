#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "treadmill.h"

class TreadmillTestData : public BluetoothDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const override {
        DeviceDiscoveryInfo config(info);
        auto none = QZLockscreenFunctions::configurationType::NONE;

        // enabling
        bool expectedZwiftMode = this->get_expectedLockscreenZwiftMode();
        // TODO: add enabling configurations


        // Disabling
        for(int i=0; i<4; i++) {
            config.ios_peloton_workaround = i&1;
            config.bike_cadence_sensor = i&2;
            configurations.push_back(LockscreenFunctionsTestData(none, false, false, config));
        }
    }
public:
    TreadmillTestData(std::string deviceName): BluetoothDeviceTestData(deviceName) {}


    deviceType get_expectedDeviceType() const override {
        return deviceType::None; // abstract
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<treadmill*>(detectedDevice)!=nullptr;
    }
};

