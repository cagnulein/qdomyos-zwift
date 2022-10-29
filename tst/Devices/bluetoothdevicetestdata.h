#pragma once

#include <bluetoothdevice.h>
#include <QStringList>
#include "discovereddevice.h"

class BluetoothDeviceTestData  {

public:
    BluetoothDeviceTestData() {
        // You can do set-up work for each test here.
    }
    virtual ~BluetoothDeviceTestData() {}


    virtual QStringList get_deviceNames() const =0;

    virtual deviceType get_expectedDeviceType() const =0;
    virtual bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const =0;

};


