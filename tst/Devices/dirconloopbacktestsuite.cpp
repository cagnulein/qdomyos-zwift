#include "dirconloopbacktestsuite.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QSettings>
#include <functional>

#include "Tools/testsettings.h"
#include "devices/dircon/dirconmanager.h"
#include "devices/dircon/wahoodirconbike.h"
#include "devices/dircon/wahoodircontreadmill.h"
#include "devices/fakebike/fakebike.h"
#include "devices/faketreadmill/faketreadmill.h"
#include "qzsettings.h"

namespace {

bool waitForCondition(const std::function<bool()> &predicate, int timeoutMs) {
    const QDateTime endTime = QDateTime::currentDateTime().addMSecs(timeoutMs);
    while (QDateTime::currentDateTime() < endTime) {
        if (predicate()) {
            return true;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }
    return predicate();
}

void configureDirconSettings(TestSettings &testSettings, int serverBasePort, int dirconId) {
    testSettings.qsettings.setValue(QZSettings::dircon_yes, true);
    testSettings.qsettings.setValue(QZSettings::dircon_server_base_port, serverBasePort);
    testSettings.qsettings.setValue(QZSettings::dircon_id, dirconId);
    testSettings.qsettings.setValue(QZSettings::virtual_device_enabled, false);
    testSettings.qsettings.setValue(QZSettings::zwift_play_emulator, false);
    testSettings.qsettings.setValue(QZSettings::rouvy_compatibility, false);
}

DirconDeviceInfo makeDirconDeviceInfo(const QString &name, quint16 port, bool isTreadmill) {
    DirconDeviceInfo deviceInfo;
    deviceInfo.name = name;
    deviceInfo.displayName = name + QStringLiteral(" (DirCon)");
    deviceInfo.address = QStringLiteral("127.0.0.1");
    deviceInfo.port = port;
    deviceInfo.isTreadmill = isTreadmill;
    return deviceInfo;
}

} // namespace

void DirconLoopbackTestSuite::test_fakebike_dircon_server_accepts_wahoodirconbike_client() {
    TestSettings testSettings(QStringLiteral("qz-test-org"), QStringLiteral("qz-test-dircon-loopback"));
    testSettings.activate();

    constexpr int kServerBasePort = 46000;
    configureDirconSettings(testSettings, kServerBasePort, 1);

    fakebike serverBike(true, true, true);
    DirconManager dirconServer(&serverBike, 4, 1.0);

    const DirconDeviceInfo deviceInfo = makeDirconDeviceInfo(QStringLiteral("Wahoo KICKR 0001"),
                                                             static_cast<quint16>(kServerBasePort), false);

    wahoodirconbike clientBike(deviceInfo, true, true, 4, 1.0);

    const bool connected = waitForCondition([&clientBike]() { return clientBike.connected(); }, 5000);
    EXPECT_TRUE(connected) << "DirCon client failed to complete handshake against local fakebike DirCon server";

    // Drive the fakebike with target power to force metric updates on the DirCon server side,
    // then verify the client receives non-zero telemetry through notifications.
    serverBike.changePower(180);

    const bool receivedTelemetry = waitForCondition(
        [&clientBike]() {
            return clientBike.wattsMetric().value() > 0 &&
                   clientBike.currentCadence().value() > 0 &&
                   clientBike.currentSpeed().value() > 0;
        },
        8000);
    EXPECT_TRUE(receivedTelemetry)
        << "DirCon client connected but did not receive expected bike telemetry (watt/cadence/speed)";

    testSettings.deactivate();
}

void DirconLoopbackTestSuite::test_faketreadmill_dircon_server_accepts_wahoodircontreadmill_client() {
    TestSettings testSettings(QStringLiteral("qz-test-org"), QStringLiteral("qz-test-dircon-loopback-treadmill"));
    testSettings.activate();

    constexpr int kServerBasePort = 46100;
    constexpr quint16 kTreadmillPortOffset = 3;
    configureDirconSettings(testSettings, kServerBasePort, 7);

    faketreadmill serverTreadmill(true, true, true);
    DirconManager dirconServer(&serverTreadmill, 4, 1.0);

    const DirconDeviceInfo deviceInfo =
        makeDirconDeviceInfo(QStringLiteral("Wahoo TREAD 0007"),
                             static_cast<quint16>(kServerBasePort + kTreadmillPortOffset), true);

    wahoodircontreadmill clientTreadmill(deviceInfo, true, true);

    const bool connected = waitForCondition([&clientTreadmill]() { return clientTreadmill.connected(); }, 5000);
    EXPECT_TRUE(connected) << "DirCon treadmill client failed to complete handshake against local fake treadmill";

    serverTreadmill.changeSpeed(8.5);
    serverTreadmill.changeInclination(4.0, 4.0);

    const bool receivedTelemetry = waitForCondition(
        [&clientTreadmill]() {
            return clientTreadmill.currentSpeed().value() > 0.0 &&
                   clientTreadmill.currentInclination().value() > 0.0;
        },
        8000);
    EXPECT_TRUE(receivedTelemetry)
        << "DirCon treadmill client connected but did not receive expected speed/inclination telemetry";

    testSettings.deactivate();
}

void DirconLoopbackTestSuite::test_two_fakebike_dircon_servers_accept_two_wahoodirconbike_clients() {
    TestSettings testSettings(QStringLiteral("qz-test-org"), QStringLiteral("qz-test-dircon-two-instances"));
    testSettings.activate();

    constexpr int kFirstServerBasePort = 46200;
    constexpr int kSecondServerBasePort = 46210;

    configureDirconSettings(testSettings, kFirstServerBasePort, 11);
    fakebike firstServerBike(true, true, true);
    DirconManager firstDirconServer(&firstServerBike, 4, 1.0);

    configureDirconSettings(testSettings, kSecondServerBasePort, 12);
    fakebike secondServerBike(true, true, true);
    DirconManager secondDirconServer(&secondServerBike, 4, 1.0);

    const DirconDeviceInfo firstDeviceInfo =
        makeDirconDeviceInfo(QStringLiteral("Wahoo KICKR 0011"), static_cast<quint16>(kFirstServerBasePort), false);
    const DirconDeviceInfo secondDeviceInfo =
        makeDirconDeviceInfo(QStringLiteral("Wahoo KICKR 0012"), static_cast<quint16>(kSecondServerBasePort), false);

    wahoodirconbike firstClient(firstDeviceInfo, true, true, 4, 1.0);
    wahoodirconbike secondClient(secondDeviceInfo, true, true, 4, 1.0);

    EXPECT_TRUE(waitForCondition([&firstClient]() { return firstClient.connected(); }, 5000))
        << "First DirCon bike client failed to connect";
    EXPECT_TRUE(waitForCondition([&secondClient]() { return secondClient.connected(); }, 5000))
        << "Second DirCon bike client failed to connect";

    firstServerBike.changePower(170);
    secondServerBike.changePower(220);

    EXPECT_TRUE(waitForCondition([&firstClient]() { return firstClient.wattsMetric().value() > 0; }, 8000))
        << "First DirCon bike client did not receive telemetry";
    EXPECT_TRUE(waitForCondition([&secondClient]() { return secondClient.wattsMetric().value() > 0; }, 8000))
        << "Second DirCon bike client did not receive telemetry";

    testSettings.deactivate();
}
