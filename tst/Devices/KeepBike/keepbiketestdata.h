#pragma once

#include "Devices/Bike/biketestdata.h"
#include "keepbike.h"

class KeepBikeTestData : public BikeTestData {

public:
    KeepBikeTestData() : BikeTestData("Keep Bike") {
        this->addDeviceName("KEEP_BIKE_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::KeepBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<keepbike*>(detectedDevice)!=nullptr;
    }
};

