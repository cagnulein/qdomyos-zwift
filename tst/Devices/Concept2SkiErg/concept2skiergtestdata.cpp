#include "concept2skiergtestdata.h" 
#include "concept2skierg.h"


Concept2SkiErgTestData::Concept2SkiErgTestData() : RowerTestData("Concept2 Ski Erg") {
    this->addDeviceName("PM5", "SKI", comparison::IgnoreCase);
}

deviceType Concept2SkiErgTestData::get_expectedDeviceType() const { return deviceType::Concept2SkiErg; }

bool Concept2SkiErgTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<concept2skierg*>(detectedDevice)!=nullptr;
}
