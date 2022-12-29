#include "bluetoothdevicetestsuite.h"
#include "discoveryoptions.h"
#include "bluetooth.h"
#include "qzsettings.h"

const QString testUUID = QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002");
QBluetoothUuid uuid{testUUID};

template<typename T>
void BluetoothDeviceTestSuite<T>::SetUp() {
    devicediscoveryinfo defaultDiscoveryInfo;

    // Test that the device is identified when enabled in the settings
    this->enablingConfigurations = this->typeParam.get_configurations(defaultDiscoveryInfo, true);
    this->disablingConfigurations = this->typeParam.get_configurations(defaultDiscoveryInfo, false);

    // Assuming these settings don't matter for this device.
    if(enablingConfigurations.size()==0)
        enablingConfigurations.push_back(defaultDiscoveryInfo);

    this->defaultDiscoveryOptions = discoveryoptions{};
    this->defaultDiscoveryOptions.startDiscovery = false;
    this->defaultDiscoveryOptions.createTemplateManagers = false;

    this->names = this->typeParam.get_deviceNames();

    EXPECT_GT(this->names.length(), 0) << "No bluetooth names configured for test";
}

template<typename T>
void BluetoothDeviceTestSuite<T>::test_deviceDetection_validNames_enabled() {
    BluetoothDeviceTestData& testData = this->typeParam;

    bluetooth bt(this->defaultDiscoveryOptions);

    for(devicediscoveryinfo discoveryInfo : enablingConfigurations) {
        for(QString deviceName : this->names)
        {
            discoveryInfo.setValues(this->settings);

            QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName);

            // try to create the device
            std::shared_ptr<bluetoothdevice> device;

            EXPECT_NO_THROW(bt.deviceConnected(deviceInfo)) << "Failed to create device object";

            EXPECT_TRUE(testData.get_isExpectedDevice(bt.device()));

            // restart the bluetooth manager to clear the device
            bt.restart();
        }
    }
}

template<typename T>
void BluetoothDeviceTestSuite<T>::test_deviceDetection_validNames_disabled() {
    BluetoothDeviceTestData& testData = this->typeParam;   

    bluetooth bt(this->defaultDiscoveryOptions);

    // Test that the device is not identified when disabled in the settings
    for(devicediscoveryinfo discoveryInfo : disablingConfigurations) {
        for(QString deviceName : this->names)
        {
            discoveryInfo.setValues(this->settings);

            QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName);

            // try to create the device
            std::shared_ptr<bluetoothdevice> device;

            EXPECT_NO_THROW(bt.deviceConnected(deviceInfo)) << "Failed to create device object";

            EXPECT_FALSE(testData.get_isExpectedDevice(bt.device())) << "Created the device when expected not to.";

            // restart the bluetooth manager to clear the device
            bt.restart();
        }
    }
}


template<typename T>
void BluetoothDeviceTestSuite<T>::test_deviceDetection_validNames_invalidBluetoothDeviceInfo_disabled()  {
    BluetoothDeviceTestData& testData = this->typeParam;

    bluetooth bt(this->defaultDiscoveryOptions);

    if(testData.get_testInvalidBluetoothDeviceInfo()) {
        // Test that the device is not identified when disabled in the settings AND has an invalid bluetooth device info
        for(devicediscoveryinfo discoveryInfo : disablingConfigurations) {
            for(QString deviceName : this->names)
            {
                discoveryInfo.setValues(this->settings);

                QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName, false);

                // try to create the device
                std::shared_ptr<bluetoothdevice> device;

                EXPECT_NO_THROW(bt.deviceConnected(deviceInfo)) << "Failed to create device object";

                EXPECT_FALSE(testData.get_isExpectedDevice(bt.device())) << "Created the device when disabled in settings.";

                // restart the bluetooth manager to clear the device
                bt.restart();
            }
        }
    }
}

template<typename T>
void BluetoothDeviceTestSuite<T>::test_deviceDetection_exclusions() {
    BluetoothDeviceTestData& testData = this->typeParam;

    bluetooth bt(this->defaultDiscoveryOptions);

    // Test that it doesn't detect this device if its higher priority "namesakes" are already detected.
    auto exclusions = testData.get_exclusions();
    for(auto exclusion : exclusions) {
        for(devicediscoveryinfo enablingDiscoveryInfo : enablingConfigurations) {
            devicediscoveryinfo discoveryInfo(enablingDiscoveryInfo);

            // indicate that the excludes are "already discovered"
            discoveryInfo.exclude(exclusion->get_expectedDeviceType());

            for(QString deviceName : this->names)
            {
                discoveryInfo.setValues(this->settings);

                QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName);

                EXPECT_NO_THROW(bt.deviceConnected(deviceInfo)) << "Failed to create device object";

                EXPECT_FALSE(testData.get_isExpectedDevice(bt.device())) << "Detected device in spite of exclusion";

                bt.restart();
            }
        }
    }
}

template<typename T>
void BluetoothDeviceTestSuite<T>::test_deviceDetection_invalidNames_enabled()
{
    BluetoothDeviceTestData& testData = this->typeParam;

    bluetooth bt(this->defaultDiscoveryOptions);

    // Test that it doesn't detect this device for the "wrong" names
    for(devicediscoveryinfo enablingDiscoveryInfo : enablingConfigurations) {

        devicediscoveryinfo discoveryInfo(enablingDiscoveryInfo);
        names = testData.get_failingDeviceNames();

        for(QString deviceName : this->names)
        {
            discoveryInfo.setValues(this->settings);

            QBluetoothDeviceInfo deviceInfo = testData.get_bluetoothDeviceInfo(uuid, deviceName);

            // try to create the device
            std::shared_ptr<bluetoothdevice> device;

            EXPECT_NO_THROW(bt.deviceConnected(deviceInfo)) << "Failed to create device object";

            EXPECT_FALSE(testData.get_isExpectedDevice(bt.device())) << "Detected device from invalid name: " << deviceName.toStdString();

            // restart the bluetooth manager to clear the device
            bt.restart();
        }
    }
}


