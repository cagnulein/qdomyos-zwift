#include "rowertestdata.h"


RowerTestData::RowerTestData(std::string testName) : BluetoothDeviceTestData(testName) {}

void RowerTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);
    auto none = QZLockscreenFunctions::configurationType::NONE;

    // Disabling
    for(int i=0; i<8; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        config.virtual_device_rower = i&4;
        configurations.push_back(LockscreenFunctionsTestData(none, false, config));
    }
}
