#pragma once


#include "Devices/Rower/rowertestdata.h"

#include "devices/smartrowrower/smartrowrower.h"


class SmartRowRowerTestData : public RowerTestData {


public:

    SmartRowRowerTestData() : RowerTestData("Smart Row Rower") {
        this->addDeviceName("SMARTROW", comparison::StartsWithIgnoreCase);

    }

    deviceType get_expectedDeviceType() const override { return deviceType::SmartRowRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<smartrowrower*>(detectedDevice)!=nullptr;
    }
};

