#pragma once

#include "Devices/Bike/biketestdata.h"

#include "devices/snodebike/snodebike.h"

class SnodeBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new snodebike(options.noResistance, options.noHeartService);
    }
    SnodeBikeTestData(std::string testName) : BikeTestData(testName) { }

    void configureExclusions() override;
public:

    deviceType get_expectedDeviceType() const override { return deviceType::SnodeBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<snodebike*>(detectedDevice)!=nullptr;
    }
};

class SnodeBike1TestData : public SnodeBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.snode_bike = enable;
        return true;
    }
public:
    SnodeBike1TestData() : SnodeBikeTestData("Snode Bike") {
        this->addDeviceName("FS-", comparison::StartsWith);
    }
};

class SnodeBike2TestData : public SnodeBikeTestData {

    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            config.horizon_treadmill_force_ftms = false;
            configurations.push_back(config);
        } else {
            // Basic case where the device is disabled in the settings
            config.horizon_treadmill_force_ftms = true;
            configurations.push_back(config);
        }
    }

public:
    SnodeBike2TestData() : SnodeBikeTestData("Snode Bike TF") {
        this->addDeviceName("TF-", comparison::StartsWithIgnoreCase);
    }

};
