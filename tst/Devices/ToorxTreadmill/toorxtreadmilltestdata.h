#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class ToorxTreadmillTestData : public TreadmillTestData {

public:
    ToorxTreadmillTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

