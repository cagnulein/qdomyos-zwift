#pragma once

#include "Devices/Bike/biketestdata.h"


class NautilusBikeTestData : public BikeTestData {

public:
    NautilusBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

