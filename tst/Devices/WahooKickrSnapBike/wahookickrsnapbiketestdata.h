#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "wahookickrsnapbike.h"

class WahooKickrSnapBikeTestData : public BluetoothDeviceTestData {

public:
    WahooKickrSnapBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::WahooKickrSnapBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<wahookickrsnapbike*>(detectedDevice)!=nullptr;
    }
};

