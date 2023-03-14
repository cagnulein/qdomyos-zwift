#include "lockscreenfunctionstestdata.h"


std::map<QZLockscreenFunctions::configurationType, std::string> LockscreenFunctionsTestData::configTypeNames = {
    {QZLockscreenFunctions::configurationType::NONE , "NONE" },
    {QZLockscreenFunctions::configurationType::BIKE , "BIKE"},
    {QZLockscreenFunctions::configurationType::TREADMILL,  "TREADMILL" },
    {QZLockscreenFunctions::configurationType::ROWER , "ROWER"}
};

std::string LockscreenFunctionsTestData::getConfigurationTypeName(QZLockscreenFunctions::configurationType configType) {
    return configTypeNames.at(configType);
}

LockscreenFunctionsTestData::LockscreenFunctionsTestData(QZLockscreenFunctions::configurationType configType,
                                                         bool pelotonWorkaroundActive, bool zwiftMode,
                                                         const DeviceDiscoveryInfo &settings) {
    this->configType = configType;
    this->settings = settings;
    this->pelotonWorkaroundActive = pelotonWorkaroundActive;
    this->zwiftMode = zwiftMode;
}

QZLockscreenFunctions::configurationType LockscreenFunctionsTestData::get_lockscreenConfigType() const {
    return this->get_lockscreenFunctionsConfigType();
}

bool LockscreenFunctionsTestData::get_lockscreenZwiftMode() const {return this->zwiftMode;}

QZLockscreenFunctions::configurationType LockscreenFunctionsTestData::get_lockscreenFunctionsConfigType() const { return this->configType; }

DeviceDiscoveryInfo LockscreenFunctionsTestData::get_settings() const { return this->settings;}

bool LockscreenFunctionsTestData::get_isPelotonActive() const { return this->pelotonWorkaroundActive; }



