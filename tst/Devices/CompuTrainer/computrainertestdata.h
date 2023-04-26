#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "computrainerbike.h"

class CompuTrainerTestData : public BluetoothDeviceTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.computrainer_serial_port = enable ? "X":QString();
        return true;
    }
public:
    CompuTrainerTestData() : BluetoothDeviceTestData("CompuTrainer Bike") {
        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::CompuTrainerBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<computrainerbike*>(detectedDevice)!=nullptr;
    }
};

