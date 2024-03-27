#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/schwinn170bike/schwinn170bike.h"


class Schwinn170BikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new schwinn170bike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
public:
    Schwinn170BikeTestData() : BikeTestData("Schwinn 170 Bike") {

        this->addDeviceName("SCHWINN 170/270", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::Schwinn170Bike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<schwinn170bike*>(detectedDevice)!=nullptr;
    }
};

