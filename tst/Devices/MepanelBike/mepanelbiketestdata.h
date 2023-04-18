#pragma once

#include "Devices/Bike/biketestdata.h"
#include "mepanelbike.h"

class MepanelBikeTestData : public BikeTestData {

public:
    MepanelBikeTestData() : BikeTestData("Mepanel Bike") {
        this->addDeviceName("MEPANEL", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::MepanelBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<mepanelbike*>(detectedDevice)!=nullptr;
    }
};

