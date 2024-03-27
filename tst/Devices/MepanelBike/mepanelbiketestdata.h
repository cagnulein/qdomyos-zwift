#pragma once


#include "Devices/Bike/biketestdata.h"



#include "devices/mepanelbike/mepanelbike.h"


class MepanelBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new mepanelbike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
public:
    MepanelBikeTestData() : BikeTestData("Mepanel Bike") {
        this->addDeviceName("MEPANEL", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::MepanelBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<mepanelbike*>(detectedDevice)!=nullptr;
    }
};

