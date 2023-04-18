#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "truetreadmill.h"

class TrueTreadmillTestData : public TreadmillTestData {

public:
    TrueTreadmillTestData() : TreadmillTestData("True Treadmill") {
        this->addDeviceName("TRUE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TREADMILL", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrueTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<truetreadmill*>(detectedDevice)!=nullptr;
    }
};

