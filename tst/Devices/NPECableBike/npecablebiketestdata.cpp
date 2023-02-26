#include "npecablebiketestdata.h" 
#include "npecablebike.h"

NPECableBikeTestData::NPECableBikeTestData(std::string testName) : BikeTestData(testName) {}

deviceType NPECableBikeTestData::get_expectedDeviceType() const { return deviceType::NPECableBike; }

bool NPECableBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<npecablebike*>(detectedDevice)!=nullptr;
}

NPECableBike1TestData::NPECableBike1TestData() : NPECableBikeTestData("NPE Cable Bike") {
    this->addDeviceName(">CABLE", comparison::StartsWithIgnoreCase);
    this->addDeviceName("MD", comparison::StartsWithIgnoreCase, 7);
}

bool NPECableBike2TestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    // don't allow this device if the Flywheel bike is enabled.
    info.flywheel_life_fitness_ic8 = !enable;
    return true;
}

NPECableBike2TestData::NPECableBike2TestData() : NPECableBikeTestData("NPECable (excluding Flywheel Life Fitness IC8)") {
    // BIKE 1, BIKE 2, BIKE 3...
    this->addDeviceName("BIKE", comparison::StartsWithIgnoreCase, 6);

    // put in a name that's too long.
    this->addInvalidDeviceName("BIKE567", comparison::IgnoreCase);
}
