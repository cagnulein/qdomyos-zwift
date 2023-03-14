#include "fakeellipticaltestdata.h" 
#include "fakeelliptical.h"

bool FakeEllipticalTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.fakedevice_elliptical = enable;
    return true;
}

void FakeEllipticalTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);
    auto virtualDevice = QZLockscreenFunctions::configurationType::BIKE;

    for(int i=0; i<8; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        config.virtual_device_rower = i&4;
        bool pelotonEnabled = config.ios_peloton_workaround && config.bike_cadence_sensor; // virtual_device_rower not consdered by this rower
        configurations.push_back(LockscreenFunctionsTestData(virtualDevice, pelotonEnabled, false, config));
    }
}

FakeEllipticalTestData::FakeEllipticalTestData() : EllipticalTestData("Fake Elliptical") {
    this->addDeviceName("", comparison::StartsWithIgnoreCase);
}

deviceType FakeEllipticalTestData::get_expectedDeviceType() const { return deviceType::FakeElliptical; }

bool FakeEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<fakeelliptical*>(detectedDevice)!=nullptr;
}
