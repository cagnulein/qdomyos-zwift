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

    /**
     * @brief Gets the friendly names of all registered test data object.
     * @return
     */
    static const std::vector<QString> Names();


    /**
     * @brief Gets all the registered test data objects.
     * @return
     */
    static const std::vector<const BluetoothDeviceTestData*> TestData();

    /**
     * @brief Gets the test data for the specified friendly name.
     * @return
     */
    static const BluetoothDeviceTestData * GetTestData(const QString& name);

    /**
     * @brief Gets a type id from this class' type id generator, for the specified type.
     * @return
     */
    template <class T>
    static DeviceTypeId GetTypeId() { return TypeIdGenerator::GetTypeId<T>(); }

    /**
     * @brief Returns the test data objects that expect bluetoothdevice objects of types with the specified ids.
     * @param typeIds A list of type ids.
     * @return
     */
    static QMultiMap<DeviceTypeId, const BluetoothDeviceTestData*> WhereExpects(const std::unordered_set<DeviceTypeId> &typeIds);

    /**
     * @brief Initialize the index - sets up the test data.
     */
    static void Initialize();
};
