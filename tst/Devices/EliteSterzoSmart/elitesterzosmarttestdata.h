#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/elitesterzosmart/elitesterzosmart.h"

class EliteSterzoSmartTestData : public BikeTestData {

public:
    EliteSterzoSmartTestData() : BikeTestData("Elite Sterzo Smart") {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EliteSterzoSmart; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<elitesterzosmart*>(detectedDevice)!=nullptr;
    }
};

