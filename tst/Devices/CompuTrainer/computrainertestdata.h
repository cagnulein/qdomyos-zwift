#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/computrainerbike/computrainerbike.h"

class CompuTrainerTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new computrainerbike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }

    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.computrainer_serial_port = enable ? "X":QString();
        return true;
    }
public:
    CompuTrainerTestData() : BikeTestData("CompuTrainer Bike") {
        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::CompuTrainerBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<computrainerbike*>(detectedDevice)!=nullptr;
    }
};

