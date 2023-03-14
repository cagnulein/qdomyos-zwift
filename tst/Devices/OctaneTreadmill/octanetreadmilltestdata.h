#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class OctaneTreadmillTestData : public TreadmillTestData {

public:
    OctaneTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

