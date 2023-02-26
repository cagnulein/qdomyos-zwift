#include "flywheelbiketestdata.h" 
#include "flywheelbike.h"

FlywheelBikeTestData::FlywheelBikeTestData(std::string testName) : BikeTestData(testName) {
}

deviceType FlywheelBikeTestData::get_expectedDeviceType() const { return deviceType::FlywheelBike; }

bool FlywheelBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<flywheelbike*>(detectedDevice)!=nullptr;
}

FlywheelBike1TestData::FlywheelBike1TestData() : FlywheelBikeTestData("Flywheel Bike") {
    this->addDeviceName("Flywheel", comparison::StartsWith);
}

bool FlywheelBike2TestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.flywheel_life_fitness_ic8 = enable;
    return true;
}

FlywheelBike2TestData::FlywheelBike2TestData() : FlywheelBikeTestData("Flywheel Life Fitness IC8") {
    // BIKE 1, BIKE 2, BIKE 3...
    this->addDeviceName("BIKE", comparison::StartsWithIgnoreCase, 6);
}
