#pragma once

#include "Devices/bluetoothdevicetestdata.h"

#include "snodebike.h"

class SnodeBikeTestData : public BluetoothDeviceTestData {
protected:
    SnodeBikeTestData() { }

    void configureExclusions() override;
public:

    deviceType get_expectedDeviceType() const override { return deviceType::SnodeBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<snodebike*>(detectedDevice)!=nullptr;
    }
};

class SnodeBike1TestData : public SnodeBikeTestData {
protected:
    bool configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.snode_bike = enable;
        return true;
    }
public:
    SnodeBike1TestData() {
        this->addDeviceName("FS-", comparison::StartsWith);
    }
};

class SnodeBike2TestData : public SnodeBikeTestData {

public:
    SnodeBike2TestData() {
        this->addDeviceName("TF-", comparison::StartsWith);
    }

};
