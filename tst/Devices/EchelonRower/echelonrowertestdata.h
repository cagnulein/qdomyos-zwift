#pragma once

#include "Devices/Rower/rowertestdata.h"

class EchelonRowerTestData : public RowerTestData {
public:
    EchelonRowerTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

