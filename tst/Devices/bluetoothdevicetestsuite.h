#pragma once

#include "gtest/gtest.h"
#include "devices.h"
#include "bluetooth.h"

template <typename T>
class BluetoothDeviceTestSuite : public testing::Test {
protected:
    T typeParam;

    std::vector<devicediscoveryinfo> enablingConfigurations;
    std::vector<devicediscoveryinfo> disablingConfigurations;
    QStringList names;
    discoveryoptions defaultDiscoveryOptions;
public:
    BluetoothDeviceTestSuite() {}

    // Sets up the test fixture.
    void SetUp() override;

    // Tears down the test fixture.
    // virtual void TearDown();


    void test_deviceDetection_exclusions();
    void test_deviceDetection_validNames_enabled();
    void test_deviceDetection_validNames_disabled();
    void test_deviceDetection_validNames_invalidBluetoothDeviceInfo_disabled();
    void test_deviceDetection_invalidNames_enabled();

};


TYPED_TEST_SUITE(BluetoothDeviceTestSuite, BluetoothDeviceTestDataTypes);

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceDetectedExclusions) {
    this->test_deviceDetection_exclusions();
}

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceDetectedValidNamesSettingsEnabled) {
    this->test_deviceDetection_validNames_enabled();
}

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceDetectedValidNamesSettingsDisabled) {
    this->test_deviceDetection_validNames_disabled();
}

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceDetectedValidNamesSettingsDisabledInvalidBluetoothDeviceInfo) {
    this->test_deviceDetection_validNames_invalidBluetoothDeviceInfo_disabled();
}
