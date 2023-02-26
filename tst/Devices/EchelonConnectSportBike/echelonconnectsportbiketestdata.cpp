#include "echelonconnectsportbiketestdata.h" 
#include "Devices/EchelonRower/echelonrowertestdata.h"
#include "Devices/EchelonStrideTreadmill/echelonstridetreadmilltestdata.h"
#include "echelonconnectsport.h"

EchelonConnectSportBikeTestData::EchelonConnectSportBikeTestData() : BikeTestData("Echelon Connect Sport Bike") {
    this->addDeviceName("ECH", comparison::StartsWith);
}

void EchelonConnectSportBikeTestData::configureExclusions() {
    this->exclude(new EchelonRowerTestData());
    this->exclude(new EchelonStrideTreadmillTestData());
}

deviceType EchelonConnectSportBikeTestData::get_expectedDeviceType() const { return deviceType::EchelonConnectSport; }

bool EchelonConnectSportBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<echelonconnectsport*>(detectedDevice)!=nullptr;
}
