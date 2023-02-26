#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class KingsmithR1ProTreadmillTestData : public TreadmillTestData {
protected:
    void configureExclusions() override;
public:
    KingsmithR1ProTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

