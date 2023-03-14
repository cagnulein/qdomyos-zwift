#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class NordicTrackEllipticalTestData : public EllipticalTestData {

public:
    NordicTrackEllipticalTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

