#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/chronobike/chronobike.h"

class ChronobikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new chronobike(options.noResistance, options.noHeartService);
    }
public:
    ChronobikeTestData() : BikeTestData("Chronobike") {
        this->addDeviceName("CHRONO ", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ChronoBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<chronobike*>(detectedDevice)!=nullptr;
    }
};

