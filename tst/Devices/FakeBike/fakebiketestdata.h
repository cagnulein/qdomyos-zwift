#pragma once


#include "Devices/Bike/biketestdata.h"


#include "devices/fakebike/fakebike.h"


class FakeBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new fakebike(options.noResistance, options.noHeartService, options.noVirtualDevice);
    }

    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.fake_bike = enable;
        return true;
    }
public:
    FakeBikeTestData() : BikeTestData("Fake Bike"){
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fakebike*>(detectedDevice)!=nullptr;
    }
};

