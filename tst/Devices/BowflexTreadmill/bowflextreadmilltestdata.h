#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "bowflextreadmill.h"

class BowflexTreadmillTestData : public TreadmillTestData {

public:
    BowflexTreadmillTestData() : TreadmillTestData("Bowflex Treadmill"){}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override {
        // device not supported
        return deviceType::None;
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bowflextreadmill*>(detectedDevice)!=nullptr;
    }
};

