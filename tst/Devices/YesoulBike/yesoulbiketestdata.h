#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/yesoulbike/yesoulbike.h"


class YesoulBikeTestData : public BikeTestData {

public:
    YesoulBikeTestData() : BikeTestData("Yesoul Bike") {
        this->addDeviceName(yesoulbike::bluetoothName, comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::YesoulBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<yesoulbike*>(detectedDevice)!=nullptr;
    }
};

