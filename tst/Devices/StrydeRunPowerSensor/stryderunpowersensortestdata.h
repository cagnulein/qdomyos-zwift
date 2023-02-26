#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class StrydeRunPowerSensorTestData : public TreadmillTestData {
private:
    QString powerSensorName;
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;
public:
    StrydeRunPowerSensorTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class ZwiftRunpodTestData : public TreadmillTestData {
private:
    QString powerSensorName;
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;
public:
    ZwiftRunpodTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

