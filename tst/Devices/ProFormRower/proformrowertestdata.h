#pragma once

#include "Devices/Rower/rowertestdata.h"


class ProFormRowerTestData : public RowerTestData {

public:
    ProFormRowerTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

