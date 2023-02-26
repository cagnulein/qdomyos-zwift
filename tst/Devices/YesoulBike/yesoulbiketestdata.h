#pragma once

#include "Devices/Bike/biketestdata.h"


class YesoulBikeTestData : public BikeTestData {

public:
    YesoulBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

