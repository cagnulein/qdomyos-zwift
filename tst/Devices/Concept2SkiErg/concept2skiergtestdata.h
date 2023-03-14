#pragma once

#include "Devices/Rower/rowertestdata.h"

class Concept2SkiErgTestData : public RowerTestData {
public:
    Concept2SkiErgTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

