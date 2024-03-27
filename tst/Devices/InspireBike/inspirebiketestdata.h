#pragma once


#include "Devices/Bike/biketestdata.h"
#include "devices/inspirebike/inspirebike.h"


class InspireBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new inspirebike(options.noResistance, options.noHeartService);
    }
public:
    InspireBikeTestData() : BikeTestData("Inspire Bike") {
        this->addDeviceName("IC", comparison::StartsWithIgnoreCase, 8);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::InspireBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<inspirebike*>(detectedDevice)!=nullptr;
    }
};

