#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"

class ESLinkerTreadmillTestData : public TreadmillTestData {

public:
    ESLinkerTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

