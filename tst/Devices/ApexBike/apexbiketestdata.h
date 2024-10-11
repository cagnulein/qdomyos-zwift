#pragma once

#include "Devices/Bike/biketestdata.h".h"
#include "devices/apexbike/apexbike.h"

class ApexBikeTestData : public BikeTestData {

public:
    ApexBikeTestData() : BikeTestData("Apex Bike") {
        this->addDeviceName("WLT8266BM", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ApexBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<apexbike*>(detectedDevice)!=nullptr;
    }
};

