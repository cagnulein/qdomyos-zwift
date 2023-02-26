#pragma once

#include "Devices/Bike/biketestdata.h"


class MepanelBikeTestData : public BikeTestData {

public:
    MepanelBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

