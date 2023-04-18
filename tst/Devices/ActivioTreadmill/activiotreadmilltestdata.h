#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "activiotreadmill.h"

class ActivioTreadmillTestData : public TreadmillTestData {

public:
    ActivioTreadmillTestData() : TreadmillTestData("Activio Treadmill") {
        this->addDeviceName("RUNNERT", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ActivioTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<activiotreadmill*>(detectedDevice)!=nullptr;
    }
};

