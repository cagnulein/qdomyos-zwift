#pragma once

#include "Devices/Bike/biketestdata.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

#include "stagesbike.h"

class StagesBikeTestData : public BikeTestData {
protected:
    StagesBikeTestData(std::string testName): BikeTestData(testName) {
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
    StagesBike1TestData() : StagesBikeTestData("Stages Bike") {
        this->addDeviceName("STAGES ", comparison::StartsWithIgnoreCase);
    }

};


class StagesBike2TestData : public StagesBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.powerSensorName = enable ? "Disabled":"Roberto";
        return true;
    }
public:
    StagesBike2TestData() : StagesBikeTestData("Stages Bike (Assioma / Power Sensor disabled") {

        this->addDeviceName("ASSIOMA", comparison::StartsWithIgnoreCase);
    }
};
