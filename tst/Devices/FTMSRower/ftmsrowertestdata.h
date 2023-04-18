#pragma once

#include "Devices/Rower/rowertestdata.h"
#include "ftmsrower.h"

class FTMSRowerTestData : public RowerTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const override {
        DeviceDiscoveryInfo config(info);
        auto virtualDevice = QZLockscreenFunctions::configurationType::BIKE;

        for(int i=0; i<8; i++) {
            config.ios_peloton_workaround = i&1;
            config.bike_cadence_sensor = i&2;
            config.virtual_device_rower = i&4;
            bool pelotonEnabled = config.ios_peloton_workaround && config.bike_cadence_sensor; // virtual_device_rower not consdered by this rower
            configurations.push_back(LockscreenFunctionsTestData(virtualDevice, pelotonEnabled, false, config));
        }
    }
public:
    FTMSRowerTestData() : RowerTestData("FTMS Rower") {

        this->addDeviceName("CR 00", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KAYAKPRO", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WHIPR", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-WLT", comparison::StartsWithIgnoreCase);
        this->addDeviceName("I-ROWER", comparison::StartsWithIgnoreCase);
        this->addDeviceName("PM5ROW", comparison::IgnoreCase);
        this->addDeviceName("PM5XROW", comparison::IgnoreCase);
        this->addDeviceName("PM5XROWX", comparison::IgnoreCase);
        this->addDeviceName("PM5ROWX", comparison::IgnoreCase);
        this->addDeviceName("SF-RW", comparison::IgnoreCase);
        this->addDeviceName("S4 COMMS", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FTMSRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ftmsrower*>(detectedDevice)!=nullptr;
    }
};

