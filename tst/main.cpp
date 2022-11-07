
#include <gtest/gtest.h>
#include "Devices/bluetoothdevicetestdatafactory.h"

int main(int argc, char *argv[])
{
    BluetoothDeviceTestDataFactory::registerTestData();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
