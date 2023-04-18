#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "elliptical.h"

class EllipticalTestData : public BluetoothDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info,
                                                         std::vector<LockscreenFunctionsTestData> &configurations) const override {
        DeviceDiscoveryInfo config(info);
        auto none = QZLockscreenFunctions::configurationType::NONE;

        // Disabling
        for(int i=0; i<4; i++) {
            config.ios_peloton_workaround = i&1;
            config.bike_cadence_sensor = i&2;
            configurations.push_back(LockscreenFunctionsTestData(none, false, false, config));
        }
    }
public:
    EllipticalTestData(std::string deviceName) : BluetoothDeviceTestData(deviceName) {}

    deviceType get_expectedDeviceType() const override { return deviceType::None; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<elliptical*>(detectedDevice)!=nullptr;
    }
};

