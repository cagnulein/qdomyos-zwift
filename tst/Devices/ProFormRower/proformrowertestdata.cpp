#include "proformrowertestdata.h" 
#include "proformrower.h"

ProFormRowerTestData::ProFormRowerTestData() : RowerTestData("ProForm Rower") {
    this->addDeviceName("I_RW", comparison::StartsWithIgnoreCase);
}

deviceType ProFormRowerTestData::get_expectedDeviceType() const { return deviceType::ProformRower; }

bool ProFormRowerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<proformrower*>(detectedDevice)!=nullptr;
}
