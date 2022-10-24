#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "bhfitnesselliptical.h"

class BHFitnessEllipticalTestData : public BluetoothDeviceTestData {

public:
    BHFitnessEllipticalTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

         result.append("B01_");
         result.append("b01_");
         result.append("b01_1234");
         result.append("B01_exTraChars");

         return result;
    }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
          return dynamic_cast<bhfitnesselliptical*>(detectedDevice)!=nullptr;
    }
};
