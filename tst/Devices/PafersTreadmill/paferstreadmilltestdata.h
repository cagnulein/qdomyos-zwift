#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "paferstreadmill.h"

class PafersTreadmillTestData : public BluetoothDeviceTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            for(int x = 1; x<=3; x++) {
                config.pafers_treadmill = x & 1;
                config.pafers_treadmill_bh_iboxster_plus = x & 2;
                configurations.push_back(config);
            }
        } else {
            config.pafers_treadmill = false;
            config.pafers_treadmill_bh_iboxster_plus = false;
            configurations.push_back(config);
        }

    }


public:
    PafersTreadmillTestData() : BluetoothDeviceTestData("Pafers Treadmill") {
        this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<paferstreadmill*>(detectedDevice)!=nullptr;
    }
};

