#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class HorizonTreadmillTestData : public TreadmillTestData {

  public:
    HorizonTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override;
};

class HorizonTreadmillToorxTestData : public TreadmillTestData {
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override;

  public:
    HorizonTreadmillToorxTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override;
};

class HorizonTreadmillBodyToneTestData : public TreadmillTestData {
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override;

  public:
    HorizonTreadmillBodyToneTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override;
};
