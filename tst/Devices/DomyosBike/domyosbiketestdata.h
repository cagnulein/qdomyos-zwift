#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/domyosbike/domyosbike.h"

class DomyosBikeTestData : public BikeTestData {

public:
    DomyosBikeTestData() : BikeTestData("Domyos Bike") {

        this->addDeviceName("Domyos-Bike", comparison::StartsWith);
        this->addInvalidDeviceName("DomyosBridge", comparison::StartsWith);

    }


    deviceType get_expectedDeviceType() const override { return deviceType::DomyosBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyosbike*>(detectedDevice)!=nullptr;
    }
};

