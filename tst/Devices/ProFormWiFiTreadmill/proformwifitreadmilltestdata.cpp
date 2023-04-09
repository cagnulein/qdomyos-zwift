#include "proformwifitreadmilltestdata.h" 
#include "proformwifitreadmill.h"

bool ProFormWiFiTreadmillTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.proformtreadmillip = enable ? this->get_testIP():QString();
    return true;
}

ProFormWiFiTreadmillTestData::ProFormWiFiTreadmillTestData() : TreadmillTestData("ProForm WiFi Treadmill") {
    // any name
    this->addDeviceName("", comparison::StartsWithIgnoreCase);
}

deviceType ProFormWiFiTreadmillTestData::get_expectedDeviceType() const { return deviceType::ProformWifiTreadmill; }

bool ProFormWiFiTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<proformwifitreadmill*>(detectedDevice)!=nullptr;
}

bool ProFormWiFiTreadmillTestData::get_expectedLockscreenZwiftMode() const {
    // Special case:the virtual device is set up in the bluetoothdevice subclass' constructor.
    return true;
}
