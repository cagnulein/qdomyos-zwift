#pragma once

#include "Devices/Bike/biketestdata.h"


class TacxNeo2TestData : public BikeTestData {

public:
    TacxNeo2TestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

