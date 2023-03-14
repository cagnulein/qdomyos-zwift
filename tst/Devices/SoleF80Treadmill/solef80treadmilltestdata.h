#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class SoleF80TreadmillTestData : public TreadmillTestData {

public:
    SoleF80TreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

