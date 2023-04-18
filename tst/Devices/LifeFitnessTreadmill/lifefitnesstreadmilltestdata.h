#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "lifefitnesstreadmill.h"

class LifeFitnessTreadmillTestData : public TreadmillTestData {

public:
    LifeFitnessTreadmillTestData() : TreadmillTestData("Life Fitness Treadmill") {
        this->addDeviceName("LF", comparison::StartsWithIgnoreCase, 18);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::LifeFitnessTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<lifefitnesstreadmill*>(detectedDevice)!=nullptr;
    }
};

