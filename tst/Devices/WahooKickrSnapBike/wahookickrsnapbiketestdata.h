#pragma once

#include "Devices/Bike/biketestdata.h"


class WahooKickrSnapBikeTestData : public BikeTestData {

public:
    WahooKickrSnapBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

