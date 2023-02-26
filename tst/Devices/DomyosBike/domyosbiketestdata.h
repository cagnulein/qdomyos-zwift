#pragma once

#include "Devices/Bike/biketestdata.h"

class DomyosBikeTestData : public BikeTestData {

public:
    DomyosBikeTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

