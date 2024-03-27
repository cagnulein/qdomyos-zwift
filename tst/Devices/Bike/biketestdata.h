#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/bike.h"

class BikeOptions {
public:
   double resistanceGain=1.0, resistanceOffset=0.0;
   bool noResistance=false, noHeartService = false, noVirtualDevice = false;

};

class BikeTestData : public BluetoothDeviceTestData {
protected:
    /**
     * @brief Template function called from createInstance to create an instance of the device class.
     * @param options
     * @return
     */
    virtual bike* doCreateInstance(const BikeOptions& options) =0;

public:
    BikeTestData(std::string deviceName) : BluetoothDeviceTestData(deviceName) {}

    deviceType get_expectedDeviceType() const override {
        return deviceType::None; // abstract
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bike*>(detectedDevice)!=nullptr;
    }

    /**
     * @brief (Potentially amongst other things) Calls a protected function to create an instance of the device class.
     * @param options
     * @return
     */
    bike* createInstance(const BikeOptions& options) {
        // potentially validate or alter the options
        auto result = this->doCreateInstance(options);

        // potentially adjust the device object (e.g. connect signals and slots)
        return result;
    }
};

