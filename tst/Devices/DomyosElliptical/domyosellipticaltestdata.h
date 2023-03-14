#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class DomyosEllipticalTestData : public EllipticalTestData {

public:
    DomyosEllipticalTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

