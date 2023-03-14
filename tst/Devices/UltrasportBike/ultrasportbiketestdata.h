#pragma once

#include "Devices/Bike/biketestdata.h"


class UltrasportBikeTestData : public BikeTestData {

public:
    UltrasportBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

