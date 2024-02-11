#pragma once


#include "Devices/Bike/biketestdata.h"


#include "devices/m3ibike/m3ibike.h"


class M3IBikeTestData : public BikeTestData {

public:
    M3IBikeTestData() : BikeTestData("M3I Bike") {
        this->testInvalidBluetoothDeviceInfo = true;

        this->addDeviceName("M3", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::M3IBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<m3ibike*>(detectedDevice)!=nullptr;
    }

    QBluetoothDeviceInfo get_bluetoothDeviceInfo(const QBluetoothUuid& uuid, const QString& name, bool valid=true) override;
};

