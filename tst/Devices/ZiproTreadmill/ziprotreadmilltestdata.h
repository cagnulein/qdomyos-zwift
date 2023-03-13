#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"

class ZiproTreadmillTestData : public TreadmillTestData {

public:
    ZiproTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

