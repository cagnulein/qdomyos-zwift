#include "smartrowrowertestdata.h" 
#include "smartrowrower.h"

SmartRowRowerTestData::SmartRowRowerTestData() : RowerTestData("Smart Row Rower") {
    this->addDeviceName("SMARTROW", comparison::StartsWith);
}

deviceType SmartRowRowerTestData::get_expectedDeviceType() const { return deviceType::SmartRowRower; }

bool SmartRowRowerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<smartrowrower*>(detectedDevice)!=nullptr;
}
