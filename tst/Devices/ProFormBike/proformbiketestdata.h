#pragma once

#include "Devices/Bike/biketestdata.h"


class ProFormBikeTestData : public BikeTestData {

public:
    ProFormBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

