#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "cscbike.h"

class CSCBikeTestData : public BluetoothDeviceTestData {
private:
    QString cscBikeName;
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
       info.cscName = enable ? this->cscBikeName : "Disabled";
       info.csc_as_bike = enable;
       return true;
    }
public:
    CSCBikeTestData() : BluetoothDeviceTestData("CSC Bike") {
        this->cscBikeName = "CyclingSpeedCadenceBike-";

        this->addDeviceName(QStringLiteral("JOROTO-BK-"), comparison::StartsWith);
        this->addDeviceName(this->cscBikeName, comparison::StartsWith);
        this->addInvalidDeviceName("X"+this->cscBikeName, comparison::Exact);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::CSCBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<cscbike*>(detectedDevice)!=nullptr;
    }
};

