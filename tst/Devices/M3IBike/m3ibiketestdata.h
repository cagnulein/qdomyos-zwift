#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "m3ibike.h"

class M3IBikeTestData : public BluetoothDeviceTestData {

public:
    M3IBikeTestData() : BluetoothDeviceTestData("M3I Bike") {
        this->testInvalidBluetoothDeviceInfo = true;

        this->addDeviceName("M3", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::M3IBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<m3ibike*>(detectedDevice)!=nullptr;
    }

    QBluetoothDeviceInfo get_bluetoothDeviceInfo(const QBluetoothUuid& uuid, const QString& name, bool valid=true) override;
};

