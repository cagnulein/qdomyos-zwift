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
                                                         bool pelotonWorkaroundActive,
                                                         const DeviceDiscoveryInfo &settings) {
    this->configType = configType;
    this->settings = settings;
    this->pelotonWorkaroundActive = pelotonWorkaroundActive;
}

QZLockscreenFunctions::configurationType LockscreenFunctionsTestData::get_lockscreenConfigType() const {
    // Once testing of virtualdevices is implemented, this will need to consider the case where zwiftMode is true.
    bool zwiftMode = false;

    auto result = this->get_lockscreenFunctionsConfigType();
    if(this->get_lockscreenFunctionsConfigType()==QZLockscreenFunctions::configurationType::ROWER && !zwiftMode)
        return QZLockscreenFunctions::configurationType::BIKE;
    return result;
}

QZLockscreenFunctions::configurationType LockscreenFunctionsTestData::get_lockscreenFunctionsConfigType() const { return this->configType; }

DeviceDiscoveryInfo LockscreenFunctionsTestData::get_settings() const { return this->settings;}

bool LockscreenFunctionsTestData::get_isPelotonActive() const { return this->pelotonWorkaroundActive; }



