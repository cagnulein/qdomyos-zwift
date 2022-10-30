#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fakebike.h"

class FakeBikeTestData : public BluetoothDeviceTestData {

public:
    FakeBikeTestData() {
        this->hasSettings = true;
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.fake_bike = enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fakebike*>(detectedDevice)!=nullptr;
    }
};

