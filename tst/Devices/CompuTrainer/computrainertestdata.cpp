#include "computrainertestdata.h" 
#include "computrainerbike.h"

bool CompuTrainerTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.computrainer_serial_port = enable ? "X":QString();
    return true;
}

CompuTrainerTestData::CompuTrainerTestData() : BikeTestData("CompuTrainer Bike") {
    // any name
    this->addDeviceName("", comparison::StartsWithIgnoreCase);
}

deviceType CompuTrainerTestData::get_expectedDeviceType() const { return deviceType::CompuTrainerBike; }

bool CompuTrainerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<computrainerbike*>(detectedDevice)!=nullptr;
}
