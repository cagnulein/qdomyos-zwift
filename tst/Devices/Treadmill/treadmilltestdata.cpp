#include "treadmilltestdata.h"


void TreadmillTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);
    auto none = QZLockscreenFunctions::configurationType::NONE;

    // Disabling
    for(int i=0; i<4; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        configurations.push_back(LockscreenFunctionsTestData(none, false, false, config));
    }
}

TreadmillTestData::TreadmillTestData(std::string testName): BluetoothDeviceTestData(testName) {}

