#include "stagesbiketestdata.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

#include "stagesbike.h"

StagesBikeTestData::StagesBikeTestData(std::string testName): BikeTestData(testName) {
}

void StagesBikeTestData::configureExclusions() {
    this->exclude(new FTMSBike1TestData());
    this->exclude(new FTMSBike2TestData());
}

deviceType StagesBikeTestData::get_expectedDeviceType() const { return deviceType::StagesBike; }

bool StagesBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<stagesbike*>(detectedDevice)!=nullptr;
}

StagesBike1TestData::StagesBike1TestData() : StagesBikeTestData("Stages Bike") {
    this->addDeviceName("STAGES ", comparison::StartsWithIgnoreCase);
}

bool StagesBike2TestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.powerSensorName = enable ? "Disabled":"Roberto";
    return true;
}

StagesBike2TestData::StagesBike2TestData() : StagesBikeTestData("Stages Bike (Assioma / Power Sensor disabled") {

    this->addDeviceName("ASSIOMA", comparison::StartsWithIgnoreCase);
}
