#pragma once

#include "Devices/Rower/rowertestdata.h"
#include "devices/concept2skierg/concept2skierg.h"

class Concept2SkiErgTestData : public RowerTestData {
public:
    Concept2SkiErgTestData() : RowerTestData("Concept2 Ski Erg") {
        this->addDeviceName("PM5", "SKI", comparison::IgnoreCase);
        this->addDeviceName("PM5", comparison::IgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::Concept2SkiErg; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<concept2skierg*>(detectedDevice)!=nullptr;
    }
};

