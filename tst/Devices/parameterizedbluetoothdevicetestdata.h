#pragma once

#include "gtest/gtest.h"
#include "Devices/bluetoothdevicetestdata.h"
#include "bluetoothdevicetestdatafactory.h"

class ParameterizedBluetoothDeviceTestSuite : public testing::TestWithParam<BluetoothDeviceTestData_ptr>
{
public:
    ParameterizedBluetoothDeviceTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}

    void test_deviceDetection(BluetoothDeviceTestData_ptr testData);
};

TEST_P(ParameterizedBluetoothDeviceTestSuite, TestDeviceDetected) {
  this->test_deviceDetection(this->GetParam());
}

INSTANTIATE_TEST_SUITE_P(TestDeviceDetection,
                         ParameterizedBluetoothDeviceTestSuite,
                         testing::ValuesIn(BluetoothDeviceTestDataFactory::get_allTestData()),
                         [](const ::testing::TestParamInfo<ParameterizedBluetoothDeviceTestSuite::ParamType>& info) {
                               return info.param.get()->get_testName();
                             });
