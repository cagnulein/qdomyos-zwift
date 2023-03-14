#include "faketreadmilltestdata.h" 
#include "faketreadmill.h"

bool FakeTreadmillTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.fakedevice_treadmill = enable;
    return true;
}

FakeTreadmillTestData::FakeTreadmillTestData(): TreadmillTestData("Fake Treadmill") {
    this->addDeviceName("", comparison::StartsWithIgnoreCase);
}

deviceType FakeTreadmillTestData::get_expectedDeviceType() const { return deviceType::FakeTreadmill; }

bool FakeTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<faketreadmill*>(detectedDevice)!=nullptr;
}
