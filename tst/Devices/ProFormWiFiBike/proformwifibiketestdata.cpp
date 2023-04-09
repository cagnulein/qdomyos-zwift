#include "proformwifibiketestdata.h" 
#include "proformwifibike.h"

bool ProFormWiFiBikeTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.proformtdf4ip = enable ? this->get_testIP():QString();
    return true;
}

ProFormWiFiBikeTestData::ProFormWiFiBikeTestData(): BikeTestData("ProForm WiFi Bike") {
    // any name
    this->addDeviceName("", comparison::StartsWithIgnoreCase);
}

deviceType ProFormWiFiBikeTestData::get_expectedDeviceType() const { return deviceType::ProformWifiBike; }

bool ProFormWiFiBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<proformwifibike*>(detectedDevice)!=nullptr;
}

bool ProFormWiFiBikeTestData::get_expectedLockscreenZwiftMode() const {
    // special case because the virtual device is set up in the bluetoothdevice subclass' constructor
    return true;
}

