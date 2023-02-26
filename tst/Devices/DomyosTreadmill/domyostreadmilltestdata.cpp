#include "domyostreadmilltestdata.h" 
#include "Devices/DomyosElliptical/domyosellipticaltestdata.h"
#include "Devices/DomyosRower/domyosrowertestdata.h"
#include "Devices/DomyosBike/domyosbiketestdata.h"
#include "domyostreadmill.h"

DomyosTreadmillTestData::DomyosTreadmillTestData() : TreadmillTestData("Domyos Treadmill") {
    this->addDeviceName("Domyos", comparison::StartsWith);

    this->addInvalidDeviceName("DomyosBr", comparison::StartsWith);
}

void DomyosTreadmillTestData::configureExclusions() {
    this->exclude(new DomyosEllipticalTestData());
    this->exclude(new DomyosBikeTestData());
    this->exclude(new DomyosRowerTestData());
}

deviceType DomyosTreadmillTestData::get_expectedDeviceType() const { return deviceType::DomyosTreadmill; }

bool DomyosTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<domyostreadmill*>(detectedDevice)!=nullptr;
}
