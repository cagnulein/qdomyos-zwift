#pragma once

#include "Devices/Rower/rowertestdata.h"


class SmartRowRowerTestData : public RowerTestData {
public:
    SmartRowRowerTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

