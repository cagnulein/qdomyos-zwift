#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "cscbike.h"

class CSCBikeTestData : public BluetoothDeviceTestData {
private:
    QString cscBikeName;
public:
    CSCBikeTestData() {
        this->cscBikeName = "CyclingSpeedCadenceBike-";
        this->hasSettings = true;

        this->addDeviceName(this->cscBikeName, comparison::StartsWith);
        this->addInvalidDeviceName("X"+this->cscBikeName, comparison::Exact);

    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
       info.cscName = enable ? this->cscBikeName : "Disabled";
       info.csc_as_bike = enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::CSCBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<cscbike*>(detectedDevice)!=nullptr;
    }
};

