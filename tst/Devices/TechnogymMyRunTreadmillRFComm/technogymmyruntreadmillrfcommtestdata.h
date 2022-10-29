#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "technogymmyruntreadmillrfcomm.h"

class TechnogymMyRunTreadmillRFCommTestData : public BluetoothDeviceTestData {

public:
    TechnogymMyRunTreadmillRFCommTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TechnogymMyRunTreadmillRFComm; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<technogymmyruntreadmillrfcomm*>(detectedDevice)!=nullptr;
    }
};

