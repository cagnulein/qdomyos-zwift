#pragma once

#include "Devices/Bike/biketestdata.h"


class M3IBikeTestData : public BikeTestData {

public:
    M3IBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;

    QBluetoothDeviceInfo get_bluetoothDeviceInfo(const QBluetoothUuid& uuid, const QString& name, bool valid=true) override;
};

