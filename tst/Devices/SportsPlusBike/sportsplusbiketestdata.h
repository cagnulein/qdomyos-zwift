#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/sportsplusbike/sportsplusbike.h"


class SportsPlusBikeTestData : public BikeTestData {

public:
    SportsPlusBikeTestData() : BikeTestData("Sports Plus Bike") {
        this->addDeviceName("CARDIOFIT", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SportsPlusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<sportsplusbike*>(detectedDevice)!=nullptr;
    }
};

