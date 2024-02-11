#pragma once


#include "Devices/Rower/rowertestdata.h"

#include "devices/ftmsrower/ftmsrower.h"


class FTMSRowerTestData : public RowerTestData {

public:
    FTMSRowerTestData() : RowerTestData("FTMS Rower") {

        this->addDeviceName("CR 00", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KAYAKPRO", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WHIPR", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-WLT", comparison::StartsWithIgnoreCase);
        this->addDeviceName("I-ROWER", comparison::StartsWithIgnoreCase);
        this->addDeviceName("PM5ROW", comparison::IgnoreCase);
        this->addDeviceName("PM5XROW", comparison::IgnoreCase);
        this->addDeviceName("SF-RW", comparison::IgnoreCase);
        this->addDeviceName("S4 COMMS", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FTMSRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ftmsrower*>(detectedDevice)!=nullptr;
    }
};

