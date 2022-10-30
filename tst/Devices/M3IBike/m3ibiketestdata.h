#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "m3ibike.h"

class M3IBikeTestData : public BluetoothDeviceTestData {

public:
    M3IBikeTestData() {
        this->testInvalidBluetoothDeviceInfo = true;

        this->addDeviceName("M3", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::M3IBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<m3ibike*>(detectedDevice)!=nullptr;
    }

    QBluetoothDeviceInfo get_bluetoothDeviceInfo(const QBluetoothUuid& uuid, const QString& name, bool valid=true) override {
        // The M3I bike detector looks into the manufacturer data.

        QBluetoothDeviceInfo result = BluetoothDeviceTestData::get_bluetoothDeviceInfo(uuid, name, true);

        if(!valid) {
            result.setManufacturerData(1, QByteArray("Invalid manufacturer data."));
            return result;
        }

        // TODO: set some valid manufactuer data here.
        result.setManufacturerData(1, QByteArray(10, ' '));

        return result;
    }
};

