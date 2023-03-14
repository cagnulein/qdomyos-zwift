#include "cscbiketestdata.h" 
#include "cscbike.h"

CSCBikeTestData::CSCBikeTestData(std::string testName) : BikeTestData(testName) {
    this->cscBikeName = "CyclingSpeedCadenceBike-";
}

deviceType CSCBikeTestData::get_expectedDeviceType() const { return deviceType::CSCBike; }

bool CSCBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<cscbike*>(detectedDevice)!=nullptr;
}

bool CSCBike1TestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.cscName = enable ? this->cscBikeName : "Disabled";
    info.csc_as_bike = enable;
    return true;
}

CSCBike1TestData::CSCBike1TestData() : CSCBikeTestData("CSC Bike (Named)") {

    // Test for cases where success means the csc_as_bike setting has to be enabled.

    this->addDeviceName(this->cscBikeName, comparison::StartsWith);
    this->addInvalidDeviceName("X"+this->cscBikeName, comparison::Exact);
}

void CSCBike2TestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if(enable) {
        // If the Bluetooth name doesn't match the one being tested, but if csc_as_bike is enabled in the settings,
        // and the bluetooth name does match the cscName in the settings, the device will be detected anyway,
        // so prevent this by not including that specific configuration
        //
        // In order for the search to actually happen, the cscName has to be "Disabled" or csc_as_bike must be true.
        /*
            config.csc_as_bike = true;
            config.cscName = this->cscBikeName;
            configurations.push_back(config);
            */

        config.cscName = "Disabled";
        config.csc_as_bike = true;
        configurations.push_back(config);

        config.cscName = "Disabled";
        config.csc_as_bike = false;
        configurations.push_back(config);

        config.csc_as_bike = true;
        config.cscName = "NOT "+this->cscBikeName;
        configurations.push_back(config);
    }
    else  {
        // prevent the search
        config.csc_as_bike = false;
        config.cscName = "NOT "+this->cscBikeName;
        configurations.push_back(config);
    }
}

CSCBike2TestData::CSCBike2TestData() : CSCBikeTestData("CSC Bike (Unnamed)") {
    this->addDeviceName(QStringLiteral("JOROTO-BK-"), comparison::StartsWithIgnoreCase);
}
