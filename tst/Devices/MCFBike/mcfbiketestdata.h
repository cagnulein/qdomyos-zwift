#pragma once

#include "Devices/Bike/biketestdata.h"
#include "mcfbike.h"

class MCFBikeTestData : public BikeTestData {

public:
    MCFBikeTestData() : BikeTestData("MCF Bike") {
        this->addDeviceName("MCF-", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::MCFBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<mcfbike*>(detectedDevice)!=nullptr;
    }
};

