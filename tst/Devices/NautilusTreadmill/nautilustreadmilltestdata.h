#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class NautilusTreadmillTestData : public TreadmillTestData {

public:
    NautilusTreadmillTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

