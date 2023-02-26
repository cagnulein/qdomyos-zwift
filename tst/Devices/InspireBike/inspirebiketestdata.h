#pragma once

#include "Devices/Bike/biketestdata.h"


class InspireBikeTestData : public BikeTestData {

public:
    InspireBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

