#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/ultrasportbike/ultrasportbike.h"


class UltrasportBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new ultrasportbike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
public:
    UltrasportBikeTestData() : BikeTestData("Ultrasport Bike") {
        this->addDeviceName("X-BIKE", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::UltraSportBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ultrasportbike*>(detectedDevice)!=nullptr;
    }
};

