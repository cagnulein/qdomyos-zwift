#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class DomyosTreadmillTestData : public TreadmillTestData {
protected:
    void configureBluetoothDeviceInfos(const QBluetoothDeviceInfo& info,  bool enable, std::vector<QBluetoothDeviceInfo>& bluetoothDeviceInfos) const override;
public:
    DomyosTreadmillTestData();

    void configureExclusions() override;

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;

};

