#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "solef80treadmill.h"

class SoleF80TreadmillTestData : public BluetoothDeviceTestData {

public:
    SoleF80TreadmillTestData() : BluetoothDeviceTestData("Sole F80") {
        this->addDeviceName("F65", comparison::StartsWithIgnoreCase);
        this->addDeviceName("S77", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TT8", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F63", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ST90", comparison::StartsWithIgnoreCase);        
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleF80Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<solef80treadmill*>(detectedDevice)!=nullptr;
    }
};

class SoleF85TreadmillTestData : public BluetoothDeviceTestData {
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            config.sole_treadmill_inclination = true;
            configurations.push_back(config);
        } else {
            // Basic case where the device is disabled in the settings
            config.sole_treadmill_inclination = false;
            configurations.push_back(config);
        }
    }

  public:
    SoleF85TreadmillTestData() : BluetoothDeviceTestData("Sole F85 Treadmill") {
        this->addDeviceName("F85", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F80", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleF80Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<solef80treadmill *>(detectedDevice) != nullptr;
    }
};