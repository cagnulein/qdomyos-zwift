#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class BowflexT216TreadmillTestData : public TreadmillTestData {

public:
    BowflexT216TreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

