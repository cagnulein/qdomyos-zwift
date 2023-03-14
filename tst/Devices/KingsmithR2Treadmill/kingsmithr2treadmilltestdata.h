#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class KingsmithR2TreadmillTestData : public TreadmillTestData {

public:
    KingsmithR2TreadmillTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

