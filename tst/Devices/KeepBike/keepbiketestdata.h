#pragma once

#include "Devices/Bike/biketestdata.h"


class KeepBikeTestData : public BikeTestData {

public:
    KeepBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

