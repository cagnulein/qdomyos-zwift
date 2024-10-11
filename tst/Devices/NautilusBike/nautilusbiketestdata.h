#pragma once


#include "Devices/Bike/biketestdata.h"
#include "devices/nautilusbike/nautilusbike.h"


class NautilusBikeTestData : public BikeTestData {

public:
    NautilusBikeTestData(): BikeTestData("Nautilus Bike") {
        this->addDeviceName("NAUTILUS B", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NautilusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nautilusbike*>(detectedDevice)!=nullptr;
    }
};

