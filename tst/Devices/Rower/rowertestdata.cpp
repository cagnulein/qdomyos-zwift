#include "rowertestdata.h"


RowerTestData::RowerTestData(std::string testName) : BluetoothDeviceTestData(testName) {}

void RowerTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);
    auto none = QZLockscreenFunctions::configurationType::NONE;
    auto bike = QZLockscreenFunctions::configurationType::BIKE;
    auto rower = QZLockscreenFunctions::configurationType::ROWER;
    auto expectedZwiftMode = this->get_expectedLockscreenZwiftMode();

    for(int i=0; i<8; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        config.virtual_device_rower = i&4;
        bool enabled = config.ios_peloton_workaround && config.bike_cadence_sensor && !config.virtual_device_rower;
        configurations.push_back(LockscreenFunctionsTestData(enabled ? rower:none, enabled ? bike:none, enabled, enabled && expectedZwiftMode, config));
    }
}
