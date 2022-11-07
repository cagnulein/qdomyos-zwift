#pragma once

#include <QMap>
#include <memory>

#include "bluetoothdevicetestdata.h"


class BluetoothDeviceTestDataFactory
{
    static BluetoothDeviceTestDataVector allTestData;

    BluetoothDeviceTestDataFactory();

    static void registerTestData(BluetoothDeviceTestData * testData)
    {
        BluetoothDeviceTestDataFactory::allTestData.push_back(BluetoothDeviceTestData_ptr(testData));
    }

public:
    static BluetoothDeviceTestDataVector get_allTestData() {
        return BluetoothDeviceTestDataVector(allTestData);
    }

    static void registerTestData();

};
