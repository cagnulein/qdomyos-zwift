#include "dirconloopbacktestsuite.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QSettings>
#include <functional>

#include "Tools/testsettings.h"
#include "devices/dircon/dirconmanager.h"
#include "devices/dircon/wahoodirconbike.h"
#include "devices/fakebike/fakebike.h"
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

} // namespace

void DirconLoopbackTestSuite::test_fakebike_dircon_server_accepts_wahoodirconbike_client() {
    TestSettings testSettings(QStringLiteral("qz-test-org"), QStringLiteral("qz-test-dircon-loopback"));
    testSettings.activate();

    constexpr int kServerBasePort = 46000;
    testSettings.qsettings.setValue(QZSettings::dircon_yes, true);
    testSettings.qsettings.setValue(QZSettings::dircon_server_base_port, kServerBasePort);
    testSettings.qsettings.setValue(QZSettings::dircon_id, 1);
    testSettings.qsettings.setValue(QZSettings::virtual_device_enabled, false);
    testSettings.qsettings.setValue(QZSettings::zwift_play_emulator, false);
    testSettings.qsettings.setValue(QZSettings::rouvy_compatibility, false);

    fakebike serverBike(true, true, true);
    DirconManager dirconServer(&serverBike, 4, 1.0);

    DirconDeviceInfo deviceInfo;
    deviceInfo.name = QStringLiteral("Wahoo KICKR 0001");
    deviceInfo.displayName = QStringLiteral("Wahoo KICKR 0001 (DirCon)");
    deviceInfo.address = QStringLiteral("127.0.0.1");
    deviceInfo.port = static_cast<quint16>(kServerBasePort);

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
