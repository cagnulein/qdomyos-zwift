#include "elitesterzosmarttestdata.h" 
#include "elitesterzosmart.h"

EliteSterzoSmartTestData::EliteSterzoSmartTestData() : BikeTestData("Elite Sterzo Smart") {
    this->addDeviceName("", comparison::StartsWith);
}


deviceType EliteSterzoSmartTestData::get_expectedDeviceType() const { return deviceType::EliteSterzoSmart; }

bool EliteSterzoSmartTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<elitesterzosmart*>(detectedDevice)!=nullptr;
}
