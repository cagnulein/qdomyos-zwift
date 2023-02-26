#pragma once
#include <lockscreen/qzlockscreenfunctions.h>
#include "devicediscoveryinfo.h"

class LockscreenFunctionsTestData
{
    QZLockscreenFunctions::configurationType configType;
    DeviceDiscoveryInfo settings;
    bool pelotonWorkaroundActive = false;
public:
    LockscreenFunctionsTestData(QZLockscreenFunctions::configurationType configType, bool pelotonWorkaroundActive, const DeviceDiscoveryInfo& settings);

    QZLockscreenFunctions::configurationType get_configType() const;
    DeviceDiscoveryInfo get_settings() const;
    bool get_isPelotonActive() const;   
};

