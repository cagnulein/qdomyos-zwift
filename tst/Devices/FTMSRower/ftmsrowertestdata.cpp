#include "ftmsrowertestdata.h" 
#include "ftmsrower.h"

FTMSRowerTestData::FTMSRowerTestData() : RowerTestData("FTMS Rower") {

    this->addDeviceName("CR 00", comparison::StartsWithIgnoreCase);
    this->addDeviceName("KAYAKPRO", comparison::StartsWithIgnoreCase);
    this->addDeviceName("WHIPR", comparison::StartsWithIgnoreCase);
    this->addDeviceName("KS-WLT", comparison::StartsWithIgnoreCase);
    this->addDeviceName("I-ROWER", comparison::StartsWithIgnoreCase);
    this->addDeviceName("PM5ROW", comparison::IgnoreCase);
    this->addDeviceName("PM5XROW", comparison::IgnoreCase);
    this->addDeviceName("PM5XROWX", comparison::IgnoreCase);
    this->addDeviceName("PM5ROWX", comparison::IgnoreCase);
    this->addDeviceName("SF-RW", comparison::IgnoreCase);
}

void FTMSRowerTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
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

deviceType FTMSRowerTestData::get_expectedDeviceType() const { return deviceType::FTMSRower; }

bool FTMSRowerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<ftmsrower*>(detectedDevice)!=nullptr;
}
