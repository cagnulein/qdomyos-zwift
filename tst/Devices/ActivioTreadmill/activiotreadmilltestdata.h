#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"

class ActivioTreadmillTestData : public TreadmillTestData {

public:
    ActivioTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

