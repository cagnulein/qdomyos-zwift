#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class BHFitnessEllipticalTestData : public EllipticalTestData {

public:
    BHFitnessEllipticalTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};
