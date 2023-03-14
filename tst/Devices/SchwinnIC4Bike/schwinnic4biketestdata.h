#pragma once

#include "Devices/Bike/biketestdata.h"


class SchwinnIC4BikeTestData : public BikeTestData {

public:
    SchwinnIC4BikeTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

