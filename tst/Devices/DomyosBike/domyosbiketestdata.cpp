#include "domyosbiketestdata.h" 
#include "domyosbike.h"

DomyosBikeTestData::DomyosBikeTestData() : BikeTestData("Domyos Bike") {

    this->addDeviceName("Domyos-Bike", comparison::StartsWith);
    this->addInvalidDeviceName("DomyosBridge", comparison::StartsWith);

}

deviceType DomyosBikeTestData::get_expectedDeviceType() const { return deviceType::DomyosBike; }

bool DomyosBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<domyosbike*>(detectedDevice)!=nullptr;
}
