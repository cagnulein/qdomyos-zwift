#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class TrueTreadmillTestData : public TreadmillTestData {

public:
    TrueTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

