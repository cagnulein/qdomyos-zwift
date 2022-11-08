#pragma once

#include "gtest/gtest.h"
#include "bluetoothdevicetestsuite.h"
#include "Devices/bluetoothdevicetestdata.h"
#include "bluetoothdevicetestdatafactory.h"

class ParameterizedBluetoothDeviceTestSuite : public testing::TestWithParam<BluetoothDeviceTestData_ptr>
{
public:
    ParameterizedBluetoothDeviceTestSuite() {}
};

TEST_P(ParameterizedBluetoothDeviceTestSuite, TestDeviceDetected) {
    BluetoothDeviceTestSuite::test_deviceDetection(*this->GetParam().get());
}

INSTANTIATE_TEST_SUITE_P(TestDeviceDetection,
                         ParameterizedBluetoothDeviceTestSuite,
                         testing::ValuesIn(BluetoothDeviceTestDataFactory::get_allTestData()),
                         [](const ::testing::TestParamInfo<ParameterizedBluetoothDeviceTestSuite::ParamType>& info) {
                               return info.param.get()->get_testName();
                             });
