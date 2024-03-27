#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/solebike/solebike.h"


class SoleBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new solebike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
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

