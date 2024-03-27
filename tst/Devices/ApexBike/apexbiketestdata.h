#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/apexbike/apexbike.h"

class ApexBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new apexbike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
public:
    ApexBikeTestData() : BikeTestData("Apex Bike") {
        this->addDeviceName("WLT8266BM", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ApexBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<apexbike*>(detectedDevice)!=nullptr;
    }
};

