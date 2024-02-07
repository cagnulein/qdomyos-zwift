#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/activiotreadmill/activiotreadmill.h"

class ActivioTreadmillTestData : public BluetoothDeviceTestData {

public:
    ActivioTreadmillTestData() : BluetoothDeviceTestData("Activio Treadmill") {
        this->addDeviceName("RUNNERT", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ActivioTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<activiotreadmill*>(detectedDevice)!=nullptr;
    }
};

