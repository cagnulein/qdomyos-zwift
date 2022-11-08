#pragma once

#include "gtest/gtest.h"
#include "Devices/bluetoothdevicetestdata.h"

class BluetoothDeviceTestSuite : public testing::Test
{
public:
    BluetoothDeviceTestSuite()  {}

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }


    template<class T>
    static void test_deviceDetection();
    static void test_deviceDetection(BluetoothDeviceTestData& testData);

};

