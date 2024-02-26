#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/skandikawiribike/skandikawiribike.h"


class SkandikaWiryBikeTestData : public BikeTestData {

public:
    SkandikaWiryBikeTestData() : BikeTestData("Skandika Wiry Bike") {
        this->addDeviceName("BFCP", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SkandikawiriBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<skandikawiribike*>(detectedDevice)!=nullptr;
    }
};

