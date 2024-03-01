#pragma once


#include "Devices/Bike/biketestdata.h"
#include "devices/schwinnic4bike/schwinnic4bike.h"


class SchwinnIC4BikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new schwinnic4bike(options.noResistance, options.noHeartService);
    }
public:
    SchwinnIC4BikeTestData() : BikeTestData("Schwinn IC4 Bike") {

        this->addDeviceName("IC BIKE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("C7-", comparison::StartsWithIgnoreCase);
        this->addDeviceName("C9/C10", comparison::StartsWithIgnoreCase);

        // 17 characters, beginning with C7-
        this->addInvalidDeviceName("C7-45678901234567", comparison::IgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::SchwinnIC4Bike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<schwinnic4bike*>(detectedDevice)!=nullptr;
    }
};

