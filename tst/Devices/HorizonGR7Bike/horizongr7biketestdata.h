#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/horizongr7bike/horizongr7bike.h"


class HorizonGR7BikeTestData : public BikeTestData {

public:
    HorizonGR7BikeTestData() : BikeTestData("Horizon GR7 Bike") {
        this->addDeviceName("JFIC", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonGr7Bike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<horizongr7bike*>(detectedDevice)!=nullptr;
    }
};

