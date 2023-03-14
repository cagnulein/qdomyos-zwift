#include "virtualtreadmilltestdata.h"
#include "virtualtreadmill.h"

void VirtualTreadmillTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);

    QZLockscreenFunctions::configurationType treadmill = QZLockscreenFunctions::configurationType::TREADMILL;
    QZLockscreenFunctions::configurationType none = QZLockscreenFunctions::configurationType::NONE;

    for(int i=0; i<2; i++) {
        config.ios_peloton_workaround = i&1;
        bool enabled = config.ios_peloton_workaround;
        configurations.push_back(LockscreenFunctionsTestData(enabled?treadmill:none,
                                                             enabled, enabled, config));
    }


}

VirtualTreadmillTestData::VirtualTreadmillTestData() : VirtualDeviceTestData("Virtual Treadmill", true)
{

}

virtualdevice *VirtualTreadmillTestData::createDevice(bluetoothdevice *device) {
    return new virtualtreadmill(device, false);
}
