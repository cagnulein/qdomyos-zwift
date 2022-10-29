#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fitmetriafanfit.h"

class FitmetriaFanFitTestData : public BluetoothDeviceTestData {

public:
    FitmetriaFanFitTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FitmetriaFanFit; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitmetriafanfit*>(detectedDevice)!=nullptr;
    }
};

