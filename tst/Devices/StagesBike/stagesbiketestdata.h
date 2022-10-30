#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

#include "stagesbike.h"

class StagesBikeTestData : public BluetoothDeviceTestData {
protected:
    StagesBikeTestData() {        
    }

    void configureExclusions() override;
public:


    deviceType get_expectedDeviceType() const override { return deviceType::StagesBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<stagesbike*>(detectedDevice)!=nullptr;
    }
};

class StagesBike1TestData : public StagesBikeTestData {

public:
    StagesBike1TestData() {
        this->addDeviceName("STAGES ", comparison::StartsWithIgnoreCase);
    }

};


class StagesBike2TestData : public StagesBikeTestData {

public:
    StagesBike2TestData() {
        this->hasSettings = true;

        this->addDeviceName("ASSIOMA", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.powerSensorName = enable ? "Disabled":"Roberto";
    }

};
