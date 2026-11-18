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

// Exercise the real driver state transitions without a Bluetooth adapter.
class JkFitness10800HandshakeTestPeer {
  public:
    static void defaultsAndDiscovery() {
        jkfitness10800treadmill driver;
        driver.updateReadyState();
        EXPECT_FALSE(driver.initDone);
        driver.processFrame(QByteArray::fromHex("551e0101"));
        driver.processFrame(QByteArray::fromHex("55c0020ac1"));
        EXPECT_FALSE(driver.initDone);
        driver.requestStart = 1;
        driver.discoverySettled = true;
        driver.updateReadyState();
        EXPECT_TRUE(driver.initDone);
        EXPECT_FALSE(driver.deviceTypeKnown);
        EXPECT_FALSE(driver.unitsKnown);
        EXPECT_EQ(driver.protocolDeviceType, 2);
        EXPECT_EQ(driver.units, 0);
        EXPECT_EQ(driver.requestStart, -1);
        driver.processFrame(QByteArray::fromHex("550d0a00000000000008320600"));
        EXPECT_DOUBLE_EQ(driver.currentSpeed().value(), 8.5);
        driver.processFrame(QByteArray::fromHex("550c0101"));
        EXPECT_TRUE(driver.unitsKnown);
        EXPECT_EQ(driver.units, 1);
        driver.resetConnectionState();
        EXPECT_FALSE(driver.initDone);
        EXPECT_FALSE(driver.discoverySettled);
        EXPECT_FALSE(driver.nativeStartupComplete);
        EXPECT_EQ(driver.units, 0);
    }
    static void unsupportedTypeAndTimeout() {
        jkfitness10800treadmill driver;
        driver.discoverySettled = true;
        driver.updateReadyState();
        driver.startSequenceQueue.enqueue(QByteArray::fromHex("550a0101"));
        driver.processFrame(QByteArray::fromHex("55bb0101"));
        EXPECT_FALSE(driver.initDone);
        EXPECT_TRUE(driver.startSequenceQueue.isEmpty());
        driver.resetConnectionState();
        driver.startWaitingForRunning = true;
        driver.requestSpeed = 4;
        driver.startSequenceQueue.enqueue(QByteArray::fromHex("550a0101"));
        driver.sendNextStartFrame();
        EXPECT_FALSE(driver.startWaitingForRunning);
        EXPECT_TRUE(driver.startSequenceQueue.isEmpty());
        EXPECT_EQ(driver.requestSpeed, -1);
    }
};

TEST(JkFitness10800Handshake, UsesApkDefaultsOnlyAfterDiscoverySettles) {
    JkFitness10800HandshakeTestPeer::defaultsAndDiscovery();
}

TEST(JkFitness10800Handshake, RejectsReportedNonTreadmillAndCancelsTimedOutStart) {
    JkFitness10800HandshakeTestPeer::unsupportedTypeAndTimeout();
}
