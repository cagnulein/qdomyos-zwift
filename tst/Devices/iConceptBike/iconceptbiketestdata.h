#pragma once

#include "Devices/Bike/biketestdata.h"


class iConceptBikeTestData : public BikeTestData {

public:
    iConceptBikeTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

