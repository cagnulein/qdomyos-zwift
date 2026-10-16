#include <gtest/gtest.h>

#include "Tools/testsettings.h"
#include "devices/bluetooth.h"
#include "devices/pafersbike/pafersbike.h"
#include "devices/pafersrower/pafersrower.h"
#include "qzsettings.h"

TEST(PafersRowerRouting, SelectsBikeByDefaultAndRowerWhenEnabled) {
    TestSettings testSettings(QStringLiteral("Roberto Viola"), QStringLiteral("QDomyos-Zwift Pafers Rower Routing"));
    testSettings.activate();
    testSettings.qsettings.clear();
    testSettings.qsettings.setValue(QZSettings::virtual_device_enabled, false);
    testSettings.qsettings.setValue(QZSettings::virtual_device_bluetooth, false);
    testSettings.qsettings.setValue(QZSettings::dircon_yes, false);
    testSettings.qsettings.setValue(QZSettings::pafers_treadmill, false);
    testSettings.qsettings.setValue(QZSettings::pafers_treadmill_bh_iboxster_plus, false);

    discoveryoptions options;
    options.startDiscovery = false;
    options.logs = false;
    QBluetoothDeviceInfo deviceInfo(QBluetoothUuid(QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002")),
                                    QStringLiteral("PAFERS_16840C"), 0);

    testSettings.qsettings.setValue(QZSettings::pafers_rower, false);
    {
        bluetooth bluetoothDevice(options);
        bluetoothDevice.homeformLoaded = true;
        bluetoothDevice.deviceDiscovered(deviceInfo);
        ASSERT_NE(bluetoothDevice.device(), nullptr);
        EXPECT_NE(dynamic_cast<pafersbike *>(bluetoothDevice.device()), nullptr);
        EXPECT_EQ(dynamic_cast<pafersrower *>(bluetoothDevice.device()), nullptr);
        bluetoothDevice.restart();
    }

    testSettings.qsettings.setValue(QZSettings::pafers_rower, true);
    testSettings.qsettings.setValue(QZSettings::pafers_treadmill, true);
    {
        bluetooth bluetoothDevice(options);
        bluetoothDevice.homeformLoaded = true;
        bluetoothDevice.deviceDiscovered(deviceInfo);
        ASSERT_NE(bluetoothDevice.device(), nullptr);
        EXPECT_NE(dynamic_cast<pafersrower *>(bluetoothDevice.device()), nullptr);
        EXPECT_EQ(dynamic_cast<pafersbike *>(bluetoothDevice.device()), nullptr);
        bluetoothDevice.restart();
    }

    testSettings.deactivate();
}
