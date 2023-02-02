#pragma once

#include "gtest/gtest.h"
#include "devices.h"
#include "bluetooth.h"

#include "Tools/testsettings.h"

template <typename T>
class BluetoothDeviceTestSuite : public testing::Test {

protected:
    T typeParam;

    /**
     * @brief Configurations that enable, or at least not prevent, the device from be detected.
     */
    std::vector<DeviceDiscoveryInfo> enablingConfigurations;

    /**
     * @brief Configurations that should prevent the device from being detected.
     */
    std::vector<DeviceDiscoveryInfo> disablingConfigurations;

    /**
     * @brief A sample of valid bluetooth names for the device.
     */
    QStringList names;

    /**
     * @brief The default options for dicovery by an instance of the bluetooth class.
     */
    discoveryoptions defaultDiscoveryOptions;

    /**
     * @brief Manages the QSettings used during the tests, separate from QSettings stored in the system generally.
     */
    TestSettings testSettings;

    /**
     * @brief Call bt.deviceDiscovered on the deviceInfo to try to detect and create the bluetoothdevice object for it.
     * If an exception is thrown, the test is failed with a call to FAIL().
     * Bascially replaces EXPECT_NO_THROW, for ease of breakpoint placement.
     * @param bt
     * @param deviceInfo
     */
    void tryDetectDevice(bluetooth &bt, const QBluetoothDeviceInfo &deviceInfo) const;

    /**
     * @brief Tests device detection.
     * @param testData The test data object for the device to be detected (or not).
     * @param bt The object that will do the detecting.
     * @param deviceInfo The device info for the device for which detection will be attempted.
     * @param expectMatch Indicates if the device is expected to be detected (true) or not (false).
     * @param restart Indicates if the bluetooth (bt) object should be restarted.
     * @param failMessage The failure message if the device is not detected when expected to be, or detected when not expected to be.
     */
    void testDeviceDetection(BluetoothDeviceTestData * testData, bluetooth& bt, const QBluetoothDeviceInfo& deviceInfo, bool expectMatch, bool restart, const QString& failMessage) const;

    /**
     * @brief Tests device detection.
     * @param testData The test data object for the device to be detected (or not).
     * @param bt The object that will do the detecting.
     * @param deviceInfo The device info for the device for which detection will be attempted.
     * @param expectMatch Indicates if the device is expected to be detected (true) or not (false).
     * @param restart Indicates if the bluetooth (bt) object should be restarted.
     * @param failMessage The failure message if the device is not detected when expected to be, or detected when not expected to be.
     */
    void testDeviceDetection(BluetoothDeviceTestData * testData, bluetooth& bt, const QBluetoothDeviceInfo& deviceInfo, bool expectMatch, bool restart, const std::string& failMessage) const;

public:
    BluetoothDeviceTestSuite() : testSettings("Roberto Viola", "QDomyos-Zwift Testing") {}

    // Sets up the test fixture.
    void SetUp() override;

    // Tears down the test fixture.
    // virtual void TearDown();


    /**
     * @brief Tests that a device is not detected if its exluding devices have already been detected.
     */
    void test_deviceDetection_exclusions();

    /**
     * @brief Test that if a device is enabled in the settings, and no excluding devices have already been detected,
     * the device under test will be created if a valud bluetooth name is provided.
     */
    void test_deviceDetection_validNames_enabled();

    /**
     * @brief Test that if a device is disabled in the settings, and no excluding devices have already been detected,
     * the device under test will NOT be created if a valid bluetooth name is provided.
     */
    void test_deviceDetection_validNames_disabled();

    /**
     * @brief Test that for devices whose detected depends on valid bluetooth device info data,
     * invalid bluetooth device info prevents detection.
     */
    void test_deviceDetection_validNames_invalidBluetoothDeviceInfo();

    /**
     * @brief Test that if a device is enabled in the settings, and no excluding devices have already been detected,
     * the device under test will NOT be created if an invald name is provided.
     * e.g.starts with correct text, but not the right length and/or wrong case.
     */
    void test_deviceDetection_invalidNames_enabled();

};


TYPED_TEST_SUITE(BluetoothDeviceTestSuite, BluetoothDeviceTestDataTypes);

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceNotDetectedDueToExclusions) {
    this->test_deviceDetection_exclusions();
}

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceDetectedValidNamesSettingsEnabled) {
    this->test_deviceDetection_validNames_enabled();
}

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceNotDetectedValidNamesSettingsDisabled) {
    this->test_deviceDetection_validNames_disabled();
}

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceNotDetectedInvalidNamesSettingsEnabled) {
    this->test_deviceDetection_invalidNames_enabled();
}

TYPED_TEST(BluetoothDeviceTestSuite, TestDeviceNotDetectedValidNamesInvalidBluetoothDeviceInfo) {
    this->test_deviceDetection_validNames_invalidBluetoothDeviceInfo();
}
