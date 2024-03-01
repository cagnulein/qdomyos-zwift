#pragma once


#include "Devices/Bike/biketestdata.h"


#include "devices/mcfbike/mcfbike.h"


class MCFBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new mcfbike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
public:
    MCFBikeTestData() : BikeTestData("MCF Bike") {
        this->addDeviceName("MCF-", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::MCFBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<mcfbike*>(detectedDevice)!=nullptr;
    }
};

