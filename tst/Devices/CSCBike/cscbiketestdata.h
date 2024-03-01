#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/cscbike/cscbike.h"

class CSCBikeTestData : public BikeTestData {
protected:
    QString cscBikeName;


    bike* doCreateInstance(const BikeOptions& options) override {
        return new cscbike(options.noResistance, options.noHeartService, options.noVirtualDevice);
    }
public:
    CSCBikeTestData(std::string testName) : BikeTestData(testName) {
        this->cscBikeName = "CyclingSpeedCadenceBike-";
    }

    deviceType get_expectedDeviceType() const override { return deviceType::CSCBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<cscbike*>(detectedDevice)!=nullptr;
    }
};

class CSCBike1TestData : public CSCBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
       info.cscName = enable ? this->cscBikeName : "Disabled";
       info.csc_as_bike = enable;
       return true;
    }
public:
    CSCBike1TestData() : CSCBikeTestData("CSC Bike (Named)") {

        // Test for cases where success means the csc_as_bike setting has to be enabled.

        this->addDeviceName(this->cscBikeName, comparison::StartsWith);
        this->addInvalidDeviceName("X"+this->cscBikeName, comparison::Exact);
    }
};


class CSCBike2TestData : public CSCBikeTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
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
public:
    CSCBike2TestData() : CSCBikeTestData("CSC Bike (Unnamed)") {
        this->addDeviceName(QStringLiteral("JOROTO-BK-"), comparison::StartsWithIgnoreCase);
    }
};
