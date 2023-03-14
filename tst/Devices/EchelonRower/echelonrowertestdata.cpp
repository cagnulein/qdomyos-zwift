#include "echelonrowertestdata.h" 
#include "echelonrower.h"

EchelonRowerTestData::EchelonRowerTestData() : RowerTestData("Echelon Rower") {
    this->addDeviceName("ECH-ROW", comparison::StartsWith);
    this->addDeviceName("ROWSPORT-", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ROW-S", comparison::StartsWith);
}

deviceType EchelonRowerTestData::get_expectedDeviceType() const { return deviceType::EchelonRower; }

bool EchelonRowerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<echelonrower*>(detectedDevice)!=nullptr;
}
