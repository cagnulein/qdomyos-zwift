#pragma once

#include "Devices/Bike/biketestdata.h"
#include "solebike.h"

class SoleBikeTestData : public BikeTestData {

public:
    SoleBikeTestData() : BikeTestData("Sole Bike") {
        this->addDeviceName("LCB", comparison::StartsWithIgnoreCase);
        this->addDeviceName("R92", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<solebike*>(detectedDevice)!=nullptr;
    }
};

