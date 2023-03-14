#include "domyosrowertestdata.h"
#include "domyosrower.h"

DomyosRowerTestData::DomyosRowerTestData() : RowerTestData("Domyos Rower") {
    this->addDeviceName("DOMYOS-ROW", comparison::StartsWithIgnoreCase);
    this->addInvalidDeviceName("DomyosBridge", comparison::StartsWith);
}

deviceType DomyosRowerTestData::get_expectedDeviceType() const { return deviceType::DomyosRower; }

bool DomyosRowerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<domyosrower*>(detectedDevice)!=nullptr;
}
