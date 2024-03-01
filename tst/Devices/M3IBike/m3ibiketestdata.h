#pragma once


#include "Devices/Bike/biketestdata.h"


#include "devices/m3ibike/m3ibike.h"


class M3IBikeTestData : public BikeTestData {
protected:
    void configureBluetoothDeviceInfos(const QBluetoothDeviceInfo& info,  bool enable, std::vector<QBluetoothDeviceInfo>& bluetoothDeviceInfos) const override;

    bike* doCreateInstance(const BikeOptions& options) override {
        return new m3ibike(options.noResistance, options.noHeartService);
    }
public:
    M3IBikeTestData() : BikeTestData("M3I Bike") {
        this->addDeviceName("M3", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::M3IBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<m3ibike*>(detectedDevice)!=nullptr;
    }

};

