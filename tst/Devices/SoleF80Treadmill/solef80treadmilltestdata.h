#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"
#include "devices/solef80treadmill/solef80treadmill.h"


class SoleF80TreadmillTestData : public TreadmillTestData {
public:
    SoleF80TreadmillTestData() : TreadmillTestData("Sole F80") {
        
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

class SoleF85TreadmillTestData : public TreadmillTestData {
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
    SoleF85TreadmillTestData() : TreadmillTestData("Sole F85 Treadmill") {
        this->addDeviceName("F85", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F89", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F80", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleF80Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<solef80treadmill *>(detectedDevice) != nullptr;
    }
};
