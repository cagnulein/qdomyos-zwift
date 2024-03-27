#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/tacxneo2/tacxneo2.h"


class TacxNeo2TestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new tacxneo2(options.noResistance, options.noHeartService);
    }
public:
    TacxNeo2TestData() : BikeTestData("Tacx Neo 2 Bike") {
        this->addDeviceName("TACX NEO", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TACX FLOW", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TACX SMART BIKE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("THINK X", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TacxNeo2; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<tacxneo2*>(detectedDevice)!=nullptr;
    }
};

