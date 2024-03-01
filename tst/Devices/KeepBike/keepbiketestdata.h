#pragma once


#include "Devices/Bike/biketestdata.h"


#include "devices/keepbike/keepbike.h"


class KeepBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new keepbike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
public:
    KeepBikeTestData() : BikeTestData("Keep Bike") {
        this->addDeviceName("KEEP_BIKE_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::KeepBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<keepbike*>(detectedDevice)!=nullptr;
    }
};

