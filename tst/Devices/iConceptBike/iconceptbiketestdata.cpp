#include "iconceptbiketestdata.h" 
#include "iconceptbike.h"

iConceptBikeTestData::iConceptBikeTestData() : BikeTestData("iConcept Bike") {
    this->addDeviceName("BH DUALKIT", comparison::StartsWithIgnoreCase);
}

deviceType iConceptBikeTestData::get_expectedDeviceType() const { return deviceType::IConceptBike; }

bool iConceptBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<iconceptbike*>(detectedDevice)!=nullptr;
}
