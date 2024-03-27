#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/bkoolbike/bkoolbike.h"

class BkoolBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new bkoolbike(options.noResistance, options.noHeartService);
    }
public:
    BkoolBikeTestData() : BikeTestData("Bkool Bike") {
        this->addDeviceName("BKOOLSMARTPRO", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::BkoolBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bkoolbike*>(detectedDevice)!=nullptr;
    }
};

