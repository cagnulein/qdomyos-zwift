#include "virtualdevicetestdata.h"


void VirtualDeviceTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {}

std::string VirtualDeviceTestData::get_testName() const { return this->testName; }

std::vector<LockscreenFunctionsTestData> VirtualDeviceTestData::get_lockscreenConfigurations(const DeviceDiscoveryInfo &info) {
    std::vector<LockscreenFunctionsTestData> result;

    DeviceDiscoveryInfo newInfo(info);

    this->configureLockscreenSettings(info, result);

    return result;
}
