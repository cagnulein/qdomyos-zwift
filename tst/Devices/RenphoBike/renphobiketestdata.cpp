#include "renphobiketestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/SnodeBike/snodebiketestdata.h"

#include "renphobike.h"

RenphoBikeTestData::RenphoBikeTestData(std::string testName) : BikeTestData(testName) {
}

void RenphoBikeTestData::configureExclusions() {
    this->exclude(new FitPlusBikeFSTestData());
    this->exclude(new FitPlusBikeMRKTestData());
    this->exclude(new SnodeBike1TestData());
    this->exclude(new SnodeBike2TestData());
}

deviceType RenphoBikeTestData::get_expectedDeviceType() const { return deviceType::RenphoBike; }

bool RenphoBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<renphobike*>(detectedDevice)!=nullptr;
}

RenphoBike1TestData::RenphoBike1TestData() : RenphoBikeTestData("Renpho Bike") {
    this->addDeviceName("RQ", comparison::StartsWithIgnoreCase, 5);
    this->addDeviceName("SCH130", comparison::StartsWithIgnoreCase);
}

void RenphoBike2TestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    config.toorx_ftms = enable;
    config.toorx_bike = true;
    configurations.push_back(config);
    config.toorx_ftms = enable;
    config.toorx_bike = false;
    configurations.push_back(config);
}

RenphoBike2TestData::RenphoBike2TestData() : RenphoBikeTestData("Renpho Bike (Toorx FTMS)"){
    this->addDeviceName("TOORX", comparison::StartsWith);
}
