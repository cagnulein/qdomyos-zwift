#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/wahookickrsnapbike/wahookickrsnapbike.h"


class WahooKickrSnapBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new wahookickrsnapbike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
public:
    WahooKickrSnapBikeTestData() : BikeTestData("Wahoo Kickr Snap Bike") {
        this->addDeviceName("KICKR SNAP", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KICKR BIKE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KICKR ROLLR", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WAHOO KICKR", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::WahooKickrSnapBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<wahookickrsnapbike*>(detectedDevice)!=nullptr;
    }
};

