#include "virtualbiketestdata.h"
#include "virtualbike.h"

void VirtualBikeTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);

    QZLockscreenFunctions::configurationType bike = QZLockscreenFunctions::configurationType::BIKE;
    QZLockscreenFunctions::configurationType none = QZLockscreenFunctions::configurationType::NONE;

    for(int i=0; i<64; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        config.bike_power_sensor = i&4;
        config.virtual_device_onlyheart = i&8;
        config.virtual_device_echelon = i&16;
        config.virtual_device_ifit = i&32;
        bool isPelotonWorkaroundActive = i==1;
                ;
        configurations.push_back(LockscreenFunctionsTestData(isPelotonWorkaroundActive ? bike:none,
                                                             isPelotonWorkaroundActive,
                                                             isPelotonWorkaroundActive, config));
    }
}

VirtualBikeTestData::VirtualBikeTestData() : VirtualDeviceTestData("Virtual Bike", true)
{
}

virtualdevice *VirtualBikeTestData::createDevice(bluetoothdevice * device) {
    return new virtualbike(device);
}
