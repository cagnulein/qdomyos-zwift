#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "ftmsrower.h"

class FTMSRowerTestData : public BluetoothDeviceTestData {

public:
    FTMSRowerTestData() : BluetoothDeviceTestData("FTMS Rower") {

        this->addDeviceName("CR 00", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KAYAKPRO", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WHIPR", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-WLT", comparison::StartsWithIgnoreCase);
        this->addDeviceName("I-ROWER", comparison::StartsWithIgnoreCase);
        this->addDeviceName("PM5ROW", comparison::IgnoreCase);
        this->addDeviceName("PM5XROW", comparison::IgnoreCase);
        this->addDeviceName("PM5XROWX", comparison::IgnoreCase);
        this->addDeviceName("PM5ROWX", comparison::IgnoreCase);
        this->addDeviceName("PM5", comparison::IgnoreCase);
        this->addDeviceName("SF-RW", comparison::IgnoreCase);
        this->addDeviceName("S4 COMMS", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FTMSRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ftmsrower*>(detectedDevice)!=nullptr;
    }
};

