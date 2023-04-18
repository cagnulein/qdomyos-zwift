#pragma once

#include "Devices/Rower/rowertestdata.h"
#include "concept2skierg.h"

class Concept2SkiErgTestData : public RowerTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const override {
        DeviceDiscoveryInfo config(info);
        auto bike = QZLockscreenFunctions::configurationType::BIKE;
        bool expectedZwiftMode = this->get_expectedLockscreenZwiftMode();

        // Enabling
        config.ios_peloton_workaround = true;
        config.bike_cadence_sensor = true;
        configurations.push_back(LockscreenFunctionsTestData(bike, true, expectedZwiftMode,config));

        // Disabling
        for(int i=0; i<3; i++) {
            config.ios_peloton_workaround = i&1;
            config.bike_cadence_sensor = i&2;
            configurations.push_back(LockscreenFunctionsTestData(bike, false, false, config));
        }
    }
public:
    Concept2SkiErgTestData() : RowerTestData("Concept2 Ski Erg") {
        this->addDeviceName("PM5", "SKI", comparison::IgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::Concept2SkiErg; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<concept2skierg*>(detectedDevice)!=nullptr;
    }
};

