#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class ProFormEllipticalTestData : public EllipticalTestData {

public:
    ProFormEllipticalTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

