#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "truetreadmill.h"

class TrueTreadmillTestData : public BluetoothDeviceTestData {

public:
    TrueTreadmillTestData() {
        this->addDeviceName("F80", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F65", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TT8", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F63", comparison::StartsWithIgnoreCase);
        this->addDeviceName("F85", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrueTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<truetreadmill*>(detectedDevice)!=nullptr;
    }
};

