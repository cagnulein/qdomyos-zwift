#pragma once

#include "gtest/gtest.h"
#include "bluetoothdevicetestdatafactory.h"
#include "bluetoothdevicetestsuite.h"

template <typename T>
class TypedBluetoothDeviceTestSuite : public testing::Test {
protected:
    T typeParam;
public:
    TypedBluetoothDeviceTestSuite() {}

};


TYPED_TEST_SUITE(TypedBluetoothDeviceTestSuite, BluetoothDeviceTestDataFactory::BluetoothDeviceTestDataTypes);

TYPED_TEST(TypedBluetoothDeviceTestSuite, TestDeviceDetected) {
    BluetoothDeviceTestSuite::test_deviceDetection(this->typeParam);
}
