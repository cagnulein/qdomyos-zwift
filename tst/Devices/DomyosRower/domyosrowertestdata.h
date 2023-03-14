#pragma once

#include "Devices/Rower/rowertestdata.h"

class DomyosRowerTestData : public RowerTestData {

public:
    DomyosRowerTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

