#pragma once

#include "Devices/Bike/biketestdata.h"


class SportsTechBikeTestData : public BikeTestData {

public:
    SportsTechBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

