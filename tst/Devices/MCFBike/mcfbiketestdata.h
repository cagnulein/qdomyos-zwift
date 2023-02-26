#pragma once

#include "Devices/Bike/biketestdata.h"


class MCFBikeTestData : public BikeTestData {

public:
    MCFBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

