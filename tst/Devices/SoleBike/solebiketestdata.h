#pragma once

#include "Devices/Bike/biketestdata.h"


class SoleBikeTestData : public BikeTestData {

public:
    SoleBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

