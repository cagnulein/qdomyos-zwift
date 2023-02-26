#pragma once

#include "Devices/Bike/biketestdata.h"


class SkandikaWiryBikeTestData : public BikeTestData {

public:
    SkandikaWiryBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

