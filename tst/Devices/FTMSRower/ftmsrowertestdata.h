#pragma once

#include "Devices/Rower/rowertestdata.h"


class FTMSRowerTestData : public RowerTestData {
public:
    FTMSRowerTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

