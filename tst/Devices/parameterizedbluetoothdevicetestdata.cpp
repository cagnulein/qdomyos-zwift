#include "parameterizedbluetoothdevicetestdata.h"
#include "bluetooth.h"
#include "bluetoothdevicetestsuite.h"

void ParameterizedBluetoothDeviceTestSuite::test_deviceDetection(BluetoothDeviceTestData_ptr testData)
{
    const QString testUUID = QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002");
    QBluetoothUuid uuid{testUUID};
    QStringList names = testData.get()->get_deviceNames();

    EXPECT_GT(names.length(), 0) << "No bluetooth names configured for test";

    devicediscoveryinfo defaultDiscoveryInfo;

    // Test that the device is identified when enabled in the settings
    auto enablingConfigurations = testData.get()->get_configurations(defaultDiscoveryInfo, true);
    auto disablingConfigurations = testData.get()->get_configurations(defaultDiscoveryInfo, false);

    // Assuming these settings don't matter for this device.
    if(enablingConfigurations.size()==0)
        enablingConfigurations.push_back(defaultDiscoveryInfo);

    // set up options suitable for testing device creation
    discoveryoptions options;
    options.startDiscovery = false;
    options.createTemplateManagers = false;

    bluetooth bt(options);

    EXPECT_GT(enablingConfigurations.size(), 0) << "Test broken - there should be at least 1 enabling configuration";

    for(devicediscoveryinfo discoveryInfo : enablingConfigurations) {
        for(QString deviceName : names)
        {
            QBluetoothDeviceInfo deviceInfo = testData.get()->get_bluetoothDeviceInfo(uuid, deviceName);
            auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
            EXPECT_EQ(discovered.type, testData.get()->get_expectedDeviceType()) << "Expected device type not detected for name: " << deviceName.toStdString();

            // try to create the device
            std::shared_ptr<bluetoothdevice> device;

            EXPECT_NO_THROW(device = std::shared_ptr<bluetoothdevice>(bt.createDevice(discovered))) << "Failed to create device object";

            EXPECT_TRUE(testData.get()->get_isExpectedDevice(device.get()));
        }
    }

    // Test that the device is not identified when disabled in the settings
    for(devicediscoveryinfo discoveryInfo : disablingConfigurations) {
        for(QString deviceName : names)
        {
            QBluetoothDeviceInfo deviceInfo = testData.get()->get_bluetoothDeviceInfo(uuid, deviceName);
            auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
            EXPECT_NE(discovered.type, testData.get()->get_expectedDeviceType()) << "Expected device type detected when disabled";
        }
    }

    if(testData.get()->get_testInvalidBluetoothDeviceInfo()) {
        // Test that the device is not identified when disabled in the settings
        for(devicediscoveryinfo discoveryInfo : disablingConfigurations) {
            for(QString deviceName : names)
            {
                QBluetoothDeviceInfo deviceInfo = testData.get()->get_bluetoothDeviceInfo(uuid, deviceName, false);
                auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
                EXPECT_NE(discovered.type, testData.get()->get_expectedDeviceType()) << "Expected device type detected from invalid bluetooth device info";
            }
        }
    }

    // Test that it doesn't detect this device if its higher priority "namesakes" are already detected.
    auto exclusions = testData.get()->get_exclusions();
    for(auto exclusion : exclusions) {
        for(devicediscoveryinfo enablingDiscoveryInfo : enablingConfigurations) {
            devicediscoveryinfo discoveryInfo(enablingDiscoveryInfo);

            // indicate that the excludes are "already discovered"
            discoveryInfo.exclude(exclusion->get_expectedDeviceType());

            for(QString deviceName : names)
            {
                QBluetoothDeviceInfo deviceInfo = testData.get()->get_bluetoothDeviceInfo(uuid, deviceName);
                auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
                EXPECT_NE(discovered.type, testData.get()->get_expectedDeviceType()) << "Detected device in spite of exclusion";
            }
        }
    }

    // Test that it doesn't detect this device for the "wrong" names
    for(devicediscoveryinfo enablingDiscoveryInfo : enablingConfigurations) {

        devicediscoveryinfo discoveryInfo(enablingDiscoveryInfo);
        names = testData.get()->get_failingDeviceNames();

        for(QString deviceName : names)
        {
            QBluetoothDeviceInfo deviceInfo = testData.get()->get_bluetoothDeviceInfo(uuid, deviceName);
            auto discovered = bluetooth::discoverDevice(discoveryInfo, deviceInfo);
            EXPECT_NE(discovered.type, testData.get()->get_expectedDeviceType()) << "Detected device from invalid name: " << deviceName.toStdString();
        }
    }
}
