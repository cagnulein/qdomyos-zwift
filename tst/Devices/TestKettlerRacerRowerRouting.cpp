#include <gtest/gtest.h>

#include "Tools/testsettings.h"
#include "devices/bluetooth.h"
#include "devices/kettlerracersbike/kettlerracersbike.h"
#include "devices/kettlerracersrower/kettlerracersrower.h"
#include "qzsettings.h"

TEST(KettlerRacerRowerRouting, KeepsBikeDefaultAndSelectsRowerWhenEnabled)
{
    TestSettings testSettings(QStringLiteral("Roberto Viola"), QStringLiteral("QDomyos-Zwift Kettler Racer S Rower Routing"));
    testSettings.activate();
    testSettings.qsettings.clear();
    testSettings.qsettings.setValue(QZSettings::virtual_device_enabled, false);
    testSettings.qsettings.setValue(QZSettings::virtual_device_bluetooth, false);
    testSettings.qsettings.setValue(QZSettings::dircon_yes, false);
    testSettings.qsettings.setValue(QZSettings::kettler_racer_s_rower, false);

    discoveryoptions options;
    options.startDiscovery = false;
    options.logs = false;
    QBluetoothDeviceInfo deviceInfo(QBluetoothUuid(QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002")),
                                    QStringLiteral("RACER S TEST"), 0);

    {
        bluetooth bluetoothDevice(options);
        bluetoothDevice.homeformLoaded = true;
        bluetoothDevice.deviceDiscovered(deviceInfo);
        ASSERT_NE(bluetoothDevice.device(), nullptr);
        EXPECT_NE(dynamic_cast<kettlerracersbike *>(bluetoothDevice.device()), nullptr);
        EXPECT_EQ(dynamic_cast<kettlerracersrower *>(bluetoothDevice.device()), nullptr);
        bluetoothDevice.restart();
        EXPECT_EQ(bluetoothDevice.device(), nullptr);
    }

    testSettings.qsettings.setValue(QZSettings::kettler_racer_s_rower, true);
    {
        bluetooth bluetoothDevice(options);
        bluetoothDevice.homeformLoaded = true;
        bluetoothDevice.deviceDiscovered(deviceInfo);
        ASSERT_NE(bluetoothDevice.device(), nullptr);
        EXPECT_NE(dynamic_cast<kettlerracersrower *>(bluetoothDevice.device()), nullptr);
        EXPECT_EQ(dynamic_cast<kettlerracersbike *>(bluetoothDevice.device()), nullptr);
        bluetoothDevice.restart();
        EXPECT_EQ(bluetoothDevice.device(), nullptr);
    }

    testSettings.deactivate();
}
