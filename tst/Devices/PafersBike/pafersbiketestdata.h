#pragma once


#include "Devices/Bike/biketestdata.h"
#include "devices/pafersbike/pafersbike.h"


class PafersBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new pafersbike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        // the treadmill is given priority
        info.pafers_treadmill = !enable;
        return true;
    }
public:
    PafersBikeTestData() : BikeTestData("Pafers Bike") {
        this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<pafersbike*>(detectedDevice)!=nullptr;
    }
};

