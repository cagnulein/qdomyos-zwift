#pragma once

#include "Devices/Bike/biketestdata.h"


class EchelonConnectSportBikeTestData : public BikeTestData {

public:
    EchelonConnectSportBikeTestData();

    void configureExclusions() override;

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

