#include "virtualrowertestdata.h"
#include "virtualrower.h"

void VirtualRowerTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);

    QZLockscreenFunctions::configurationType rower = QZLockscreenFunctions::configurationType::ROWER;
    QZLockscreenFunctions::configurationType none = QZLockscreenFunctions::configurationType::NONE;

    for(int i=0; i<4; i++) {
        config.ios_peloton_workaround = i&1;
        config.virtual_device_onlyheart = i&2;
        bool isPelotonWorkaroundActive = config.ios_peloton_workaround && !config.virtual_device_onlyheart;
        configurations.push_back(LockscreenFunctionsTestData(isPelotonWorkaroundActive ? rower:none,
                                                             isPelotonWorkaroundActive,
                                                             false, // rower doesn't have Zwift mode
                                                             config));
    }
}

VirtualRowerTestData::VirtualRowerTestData(): VirtualDeviceTestData("Virtual Rower", false)
{

}

virtualdevice *VirtualRowerTestData::createDevice(bluetoothdevice *device) {
    return new virtualrower(device);
}
