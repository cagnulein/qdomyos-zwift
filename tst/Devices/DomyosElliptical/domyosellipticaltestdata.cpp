#include "domyosellipticaltestdata.h" 
#include "domyoselliptical.h"

DomyosEllipticalTestData::DomyosEllipticalTestData() : EllipticalTestData("Domyos Elliptical") {
    this->addDeviceName("Domyos-EL", comparison::StartsWith);
    this->addInvalidDeviceName("DomyosBridge",comparison::StartsWith);
}

deviceType DomyosEllipticalTestData::get_expectedDeviceType() const { return deviceType::DomyosElliptical; }

bool DomyosEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<domyoselliptical*>(detectedDevice)!=nullptr;
}
