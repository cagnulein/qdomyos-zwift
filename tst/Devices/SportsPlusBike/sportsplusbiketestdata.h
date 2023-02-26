#pragma once

#include "Devices/Bike/biketestdata.h"


class SportsPlusBikeTestData : public BikeTestData {

public:
    SportsPlusBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

