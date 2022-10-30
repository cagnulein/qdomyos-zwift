#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "paferstreadmill.h"

class PafersTreadmillTestData : public BluetoothDeviceTestData {

public:
    PafersTreadmillTestData() {
        this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.pafers_treadmill = enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<paferstreadmill*>(detectedDevice)!=nullptr;
    }
};

