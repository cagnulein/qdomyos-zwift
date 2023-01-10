#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "trxappgateusbtreadmill.h"

class TrxAppGateUSBTreadmillTestData : public BluetoothDeviceTestData {

protected:
    void configureExclusions() override;

    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo> configurations) const override {

        if(enable) {
            DeviceDiscoveryInfo info1(info);
            info1.toorx_bike = false;
            info1.toorx_ftms_treadmill = false;
            configurations.push_back(info1);
        } else {
            DeviceDiscoveryInfo info1(info);
            info1.toorx_bike = true;
            info1.toorx_ftms_treadmill = true;
            configurations.push_back(info1);

            DeviceDiscoveryInfo info2(info);
            info2.toorx_bike = false;
            info2.toorx_ftms_treadmill = true;
            configurations.push_back(info2);

            DeviceDiscoveryInfo info3(info);
            info3.toorx_bike = false;
            info3.toorx_ftms_treadmill = false;
            configurations.push_back(info3);

            DeviceDiscoveryInfo info4(info);
            info4.toorx_bike = true;
            info4.toorx_ftms_treadmill = false;
            configurations.push_back(info4);
        }
    }
public:
    TrxAppGateUSBTreadmillTestData();

    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbtreadmill*>(detectedDevice)!=nullptr;
    }
};

