#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class Shuaa5TreadmillTestData : public TreadmillTestData {

public:
    Shuaa5TreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

