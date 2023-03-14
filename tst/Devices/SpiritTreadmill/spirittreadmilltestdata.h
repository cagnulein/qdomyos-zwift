#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class SpiritTreadmillTestData : public TreadmillTestData {

public:
    SpiritTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

