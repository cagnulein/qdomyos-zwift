#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "activiotreadmill.h"

class ActivioTreadmillTestData : public BluetoothDeviceTestData {

public:
    ActivioTreadmillTestData() {
        this->addDeviceName("RUNNERT", true, true);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ActivioTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<activiotreadmill*>(detectedDevice)!=nullptr;
    }
};

