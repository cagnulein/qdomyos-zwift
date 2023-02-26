#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class ProFormEllipticalTrainerTestData : public EllipticalTestData {

public:
    ProFormEllipticalTrainerTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

