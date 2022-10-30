#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "technogymmyruntreadmill.h"

class TechnoGymMyRunTreadmillTestData : public BluetoothDeviceTestData {

public:
    TechnoGymMyRunTreadmillTestData() {
        this->hasSettings = true;
        this->addDeviceName("MYRUN ", comparison::StartsWithIgnoreCase);
        this->addDeviceName("MERACH-U3", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.technogym_myrun_treadmill_experimental = !enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TechnoGymMyRunTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<technogymmyruntreadmill*>(detectedDevice)!=nullptr;
    }
};

