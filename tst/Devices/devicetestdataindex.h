#pragma once

#include <vector>
#include <qstringlist.h>

#include "bluetoothdevicetestdata.h"
#include "Tools/typeidgenerator.h"

class DeviceTestDataIndex {
private:
    static QMap<QString,const BluetoothDeviceTestData*> testData;
    static bool isInitialized;

    static class BluetoothDeviceTestDataBuilder *  RegisterNewDeviceTestData(const QString& name);
public:

    static const std::vector<QString> Names();
    static const std::vector<const BluetoothDeviceTestData*> TestData();    
    static const BluetoothDeviceTestData * GetTestData(const QString& name);

    template <class T>
    static DeviceTypeId GetTypeId() { return TypeIdGenerator::GetTypeId<T>(); }

    static QMultiMap<DeviceTypeId, const BluetoothDeviceTestData*> WhereExpects(const std::unordered_set<DeviceTypeId> &typeIds);

    static void Initialize();
};
