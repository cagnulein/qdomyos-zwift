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

deviceType FTMSRowerTestData::get_expectedDeviceType() const { return deviceType::FTMSRower; }

bool FTMSRowerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<ftmsrower*>(detectedDevice)!=nullptr;
}
