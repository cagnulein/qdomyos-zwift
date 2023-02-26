#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class SoleEllipticalTestData : public EllipticalTestData {

public:
    SoleEllipticalTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

