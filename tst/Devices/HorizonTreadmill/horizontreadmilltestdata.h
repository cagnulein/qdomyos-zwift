#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class HorizonTreadmillTestData : public TreadmillTestData {

  public:
    HorizonTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override;
};

class HorizonTreadmillToorxTestData : public TreadmillTestData {
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override;

  public:
    HorizonTreadmillToorxTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override;
};

class HorizonTreadmillBodyToneTestData : public BluetoothDeviceTestData {
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            config.horizon_treadmill_force_ftms = true;
            configurations.push_back(config);
        } else {
            // Basic case where the device is disabled in the settings
            config.horizon_treadmill_force_ftms = false;
            configurations.push_back(config);
        }
    }

  public:
    HorizonTreadmillBodyToneTestData() : BluetoothDeviceTestData("Horizon Treadmill (Bodytone)") {
        this->addDeviceName("TF-", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<horizontreadmill *>(detectedDevice) != nullptr;
    }
};