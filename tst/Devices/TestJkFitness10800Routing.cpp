#include <gtest/gtest.h>

#include "Tools/testsettings.h"
#include "devices/bluetooth.h"
#include "devices/jkfitness10800treadmill/jkfitness10800treadmill.h"
#include "qzsettings.h"

namespace {

QBluetoothDeviceInfo jkDevice(const QString &name) {
    return QBluetoothDeviceInfo(QBluetoothUuid(QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002")), name, 0);
}

} // namespace

TEST(JkFitness10800Routing, RequiresBhSpadaCompatibilityAndMatchesModelPrefix) {
    TestSettings testSettings(QStringLiteral("QZ JK Fitness Routing"), QStringLiteral("QZ JK Fitness Routing"));
    testSettings.activate();
    testSettings.qsettings.clear();
    testSettings.qsettings.setValue(QZSettings::virtual_device_enabled, false);
    testSettings.qsettings.setValue(QZSettings::virtual_device_bluetooth, false);
    testSettings.qsettings.setValue(QZSettings::dircon_yes, false);

    discoveryoptions options;
    options.startDiscovery = false;
    options.logs = false;

    testSettings.qsettings.setValue(QZSettings::bh_spada_2, false);
    {
        bluetooth bluetoothDevice(options);
        bluetoothDevice.homeformLoaded = true;
        bluetoothDevice.deviceDiscovered(jkDevice(QStringLiteral("JK-10800-076")));
        EXPECT_EQ(bluetoothDevice.device(), nullptr);
        bluetoothDevice.restart();
    }

    testSettings.qsettings.setValue(QZSettings::bh_spada_2, true);
    {
        bluetooth bluetoothDevice(options);
        bluetoothDevice.homeformLoaded = true;
        bluetoothDevice.deviceDiscovered(jkDevice(QStringLiteral("JK-9000-001")));
        EXPECT_EQ(bluetoothDevice.device(), nullptr);
        bluetoothDevice.restart();
    }
    {
        bluetooth bluetoothDevice(options);
        bluetoothDevice.homeformLoaded = true;
        bluetoothDevice.deviceDiscovered(jkDevice(QStringLiteral("jk-10800-076")));
        ASSERT_NE(bluetoothDevice.device(), nullptr);
        EXPECT_NE(dynamic_cast<jkfitness10800treadmill *>(bluetoothDevice.device()), nullptr);
        bluetoothDevice.restart();
    }

    testSettings.deactivate();
}
