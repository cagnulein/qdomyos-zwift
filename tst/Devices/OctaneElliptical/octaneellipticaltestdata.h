#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class OctaneEllipticalTestData : public EllipticalTestData {

public:
    OctaneEllipticalTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

