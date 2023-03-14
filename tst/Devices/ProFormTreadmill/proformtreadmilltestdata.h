#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class ProFormTreadmillTestData : public TreadmillTestData {

public:
    ProFormTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

