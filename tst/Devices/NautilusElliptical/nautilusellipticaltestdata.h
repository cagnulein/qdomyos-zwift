#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class NautilusEllipticalTestData : public EllipticalTestData {

public:
    NautilusEllipticalTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

