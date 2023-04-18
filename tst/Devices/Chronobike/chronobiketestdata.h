#pragma once

#include "Devices/Bike/biketestdata.h"
#include "chronobike.h"

class ChronobikeTestData : public BikeTestData {

public:
    ChronobikeTestData() : BikeTestData("Chronobike") {
        this->addDeviceName("CHRONO ", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ChronoBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<chronobike*>(detectedDevice)!=nullptr;
    }
};

