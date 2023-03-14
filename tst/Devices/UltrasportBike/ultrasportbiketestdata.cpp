#include "ultrasportbiketestdata.h" 
#include "ultrasportbike.h"

UltrasportBikeTestData::UltrasportBikeTestData() : BikeTestData("Ultrasport Bike") {
    this->addDeviceName("X-BIKE", comparison::StartsWithIgnoreCase);
}

deviceType UltrasportBikeTestData::get_expectedDeviceType() const { return deviceType::UltraSportBike; }

bool UltrasportBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<ultrasportbike*>(detectedDevice)!=nullptr;
}
