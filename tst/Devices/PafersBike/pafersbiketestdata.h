#pragma once

#include "Devices/Bike/biketestdata.h"
#include "pafersbike.h"

class PafersBikeTestData : public BluetoothDeviceTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        // the treadmill is given priority
        info.pafers_treadmill = !enable;
        return true;
    }
public:
    PafersBikeTestData() : BluetoothDeviceTestData("Pafers Bike") {
        this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<pafersbike*>(detectedDevice)!=nullptr;
    }
};

