#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/skandikawiribike/skandikawiribike.h"


class SkandikaWiryBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new skandikawiribike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
public:
    SkandikaWiryBikeTestData() : BikeTestData("Skandika Wiry Bike") {
        this->addDeviceName("BFCP", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SkandikawiriBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<skandikawiribike*>(detectedDevice)!=nullptr;
    }
};

