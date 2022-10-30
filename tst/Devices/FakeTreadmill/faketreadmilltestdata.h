#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "faketreadmill.h"

class FakeTreadmillTestData : public BluetoothDeviceTestData {

public:
    FakeTreadmillTestData() {
        this->hasSettings = true;
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.fakedevice_treadmill = enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<faketreadmill*>(detectedDevice)!=nullptr;
    }
};

