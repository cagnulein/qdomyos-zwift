#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class LifeFitnessTreadmillTestData : public TreadmillTestData {

public:
    LifeFitnessTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

