#include "iconceptbiketestdata.h" 
#include "iconceptbike.h"

void iConceptBikeTestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if (enable) {
        config.iconcept_elliptical = false;
        configurations.push_back(config);
    } else {
        config.iconcept_elliptical = true;
        configurations.push_back(config);
    }
}

iConceptBikeTestData::iConceptBikeTestData() : BikeTestData("iConcept Bike") {
    this->addDeviceName("BH DUALKIT", comparison::StartsWithIgnoreCase);
}

deviceType iConceptBikeTestData::get_expectedDeviceType() const { return deviceType::IConceptBike; }

bool iConceptBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<iconceptbike*>(detectedDevice)!=nullptr;
}
