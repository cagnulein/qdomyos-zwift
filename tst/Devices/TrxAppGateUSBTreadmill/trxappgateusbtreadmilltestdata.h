#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "trxappgateusbtreadmill.h"

class TrxAppGateUSBTreadmillTestData : public BluetoothDeviceTestData {

protected:
    void configureExclusions() override;

    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        DeviceDiscoveryInfo config(info);

        if(enable) {
            config.toorx_bike = false;
            config.toorx_ftms_treadmill = false;
            configurations.push_back(config);
        } else {
            for(int i=1; i<4; i++) {
                config.toorx_bike = i&1;
                config.toorx_ftms_treadmill = i&2;
                configurations.push_back(config);
            }
        }
    }
public:
    TrxAppGateUSBTreadmillTestData();

    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbtreadmill*>(detectedDevice)!=nullptr;
    }
};

