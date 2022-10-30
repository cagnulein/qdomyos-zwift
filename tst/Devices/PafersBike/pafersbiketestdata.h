#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "pafersbike.h"

class PafersBikeTestData : public BluetoothDeviceTestData {

public:
    PafersBikeTestData() {
        this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        // the treadmill is given priority
        info.pafers_treadmill = !enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<pafersbike*>(detectedDevice)!=nullptr;
    }
};

