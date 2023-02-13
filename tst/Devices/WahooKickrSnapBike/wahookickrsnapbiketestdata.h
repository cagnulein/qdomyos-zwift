#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "wahookickrsnapbike.h"

class WahooKickrSnapBikeTestData : public BluetoothDeviceTestData {

public:
    WahooKickrSnapBikeTestData() : BluetoothDeviceTestData("Wahoo Kickr Snap Bike") {
        this->addDeviceName("KICKR SNAP", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KICKR BIKE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KICKR ROLLR", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WAHOO KICKR", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::WahooKickrSnapBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<wahookickrsnapbike*>(detectedDevice)!=nullptr;
    }
};

