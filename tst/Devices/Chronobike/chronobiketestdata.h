#pragma once

#include "Devices/Bike/biketestdata.h"


class ChronobikeTestData : public BikeTestData {

public:
    ChronobikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

