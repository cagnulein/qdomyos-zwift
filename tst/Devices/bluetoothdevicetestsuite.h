#pragma once

#include "gtest/gtest.h"
#include "devices.h"
#include "bluetooth.h"

#include "Tools/testsettings.h"

template <typename T>
class BluetoothDeviceTestSuite : public testing::Test {
protected:
    T typeParam;

    std::vector<devicediscoveryinfo> enablingConfigurations;
    std::vector<devicediscoveryinfo> disablingConfigurations;
    QStringList names;
    discoveryoptions defaultDiscoveryOptions;
    TestSettings testSettings;
public:
    BluetoothDeviceTestSuite() : testSettings("Roberto Viola", "QDomyos-Zwift Testing") {}

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

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceNotDetectedValidNamesSettingsDisabled) {
    this->test_deviceDetection_validNames_disabled();
}

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceNotDetectedValidNamesSettingsDisabledInvalidBluetoothDeviceInfo) {
    this->test_deviceDetection_validNames_invalidBluetoothDeviceInfo_disabled();
}

