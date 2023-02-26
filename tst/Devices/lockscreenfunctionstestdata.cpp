#include "lockscreenfunctionstestdata.h"


LockscreenFunctionsTestData::LockscreenFunctionsTestData(QZLockscreenFunctions::configurationType configType,
                                                         bool pelotonWorkaroundActive,
                                                         const DeviceDiscoveryInfo &settings) {
    this->configType = configType;
    this->settings = settings;
    this->pelotonWorkaroundActive = pelotonWorkaroundActive;
}

QZLockscreenFunctions::configurationType LockscreenFunctionsTestData::get_configType() const { return this->configType; }

DeviceDiscoveryInfo LockscreenFunctionsTestData::get_settings() const { return this->settings;}

bool LockscreenFunctionsTestData::get_isPelotonActive() const { return this->pelotonWorkaroundActive; }
