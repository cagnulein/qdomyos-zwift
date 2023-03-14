#pragma once

#include "Devices/Bike/biketestdata.h"


class HorizonGR7BikeTestData : public BikeTestData {

public:
    HorizonGR7BikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

