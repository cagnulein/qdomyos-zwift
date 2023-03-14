#include "biketestdata.h"

void BikeTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info,
                                                      std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);
    auto bike = QZLockscreenFunctions::configurationType::BIKE;

    // Enabling
    config.ios_peloton_workaround = true;
    config.bike_cadence_sensor = true;
    configurations.push_back(LockscreenFunctionsTestData(bike, true, false, config));

    // Disabling
    for(int i=0; i<3; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        configurations.push_back(LockscreenFunctionsTestData(bike, false, false, config));
    }
}


