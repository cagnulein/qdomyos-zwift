#pragma once

#include "Devices/Rower/rowertestdata.h"
#include "echelonrower.h"

class EchelonRowerTestData : public RowerTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const override {
        DeviceDiscoveryInfo config(info);
        auto rower = QZLockscreenFunctions::configurationType::ROWER;
        auto bike = QZLockscreenFunctions::configurationType::BIKE;

        for(int i=0; i<8; i++) {
            config.ios_peloton_workaround = i&1;
            config.bike_cadence_sensor = i&2;
            config.virtual_device_rower = i&4;
            bool pelotonActive = config.ios_peloton_workaround && config.bike_cadence_sensor && !config.virtual_device_rower;
            configurations.push_back(LockscreenFunctionsTestData(rower, bike,  pelotonActive, false, config));
        }
    }
public:
    EchelonRowerTestData() : RowerTestData("Echelon Rower") {
        this->addDeviceName("ECH-ROW", comparison::StartsWith);
        this->addDeviceName("ROWSPORT-", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ROW-S", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonrower*>(detectedDevice)!=nullptr;
    }
};

