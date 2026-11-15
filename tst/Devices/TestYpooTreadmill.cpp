#include <gtest/gtest.h>
#include <QFile>

#include "Tools/testsettings.h"
#include "devices/ypootreadmill/ypootreadmill.h"
#include "qzsettings.h"
#include "characteristics/characteristicnotifier2acd.h"

class YpooTreadmillTestPeer {
  public:
    static void telemetry(ypootreadmill &device, const QByteArray &packet, qint64 time) {
        device.processTelemetry(packet, time);
    }
    static void model(ypootreadmill &device, bool ypM1552) { device.YP_M1552 = ypM1552; }
    static bool stopPending(const ypootreadmill &device) { return device.requestStop != -1; }
    static YpooM5Control::Action beginCountdown(ypootreadmill &device) {
        device.ypooM5Control.telemetry(0, 0, device.ypooM5Clock.elapsed());
        device.ypooM5Control.requestStart();
        device.ypooM5Control.requestSpeed(1.2);
        device.ypooM5Control.requestIncline(1);
        return device.ypooM5Control.next(device.ypooM5Clock.elapsed()).action;
    }
    static void processRequests(ypootreadmill &device) {
        device.initDone = true;
        device.updateYpooM5Control();
    }
    static qint64 now(const ypootreadmill &device) { return device.ypooM5Clock.elapsed(); }
    static YpooM5Control::Command next(ypootreadmill &device, qint64 time) {
        return device.ypooM5Control.next(time, device.YP_M1552);
    }
    static bool requestIncline(ypootreadmill &device, double target) {
        return device.ypooM5Control.requestIncline(target);
    }
    static void startFromApp(ypootreadmill &device) {
        device.initDone = true;
        device.ftmsCharacteristicChanged(QLowEnergyCharacteristic(), QByteArray::fromHex("07"));
    }
};

namespace {
class StopPolicyTestTreadmill : public treadmill {
  public:
    void stop(bool pause) override {
        ++stopCalls;
        treadmill::stop(pause);
    }
    unsigned stopCalls = 0;
};

QByteArray telemetryPacket(int speed, int incline) {
    const uint16_t rawIncline = static_cast<uint16_t>(incline);
    // FTMS: instantaneous speed, signed incline, ramp angle.
    const uint8_t packet[] = {0x08, 0x00, uint8_t(speed), uint8_t(speed >> 8),
                              uint8_t(rawIncline), uint8_t(rawIncline >> 8), 0x00, 0x00};
    return QByteArray(reinterpret_cast<const char *>(packet), sizeof(packet));
}

QByteArray distancePacket(int speed, uint32_t meters) {
    QByteArray packet = telemetryPacket(speed, 0);
    packet[0] = 0x0c; // Instantaneous speed, total distance, inclination and ramp angle.
    const char counter[] = {char(meters), char(meters >> 8), char(meters >> 16)};
    packet.insert(4, counter, sizeof(counter));
    return packet;
}

class YpooTreadmillTest : public ::testing::Test {
  protected:
    TestSettings settings{QStringLiteral("QZTests"), QStringLiteral("YpooTreadmill")};
    ypootreadmill device{false, false};
    void SetUp() override {
        settings.qsettings.clear();
        settings.activate();
    }
};
} // namespace

TEST_F(YpooTreadmillTest, M5ReportsSpeedAndSignedInclineWithoutM1552Scaling) {
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(840, -60), 0);
    EXPECT_DOUBLE_EQ(device.currentSpeed().value(), 8.4);
    EXPECT_DOUBLE_EQ(device.currentInclination().value(), -6);
    EXPECT_TRUE(YpooTreadmillTestPeer::requestIncline(device, 13));
    EXPECT_EQ(YpooTreadmillTestPeer::next(device, 0).action, YpooM5Control::Action::Incline);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(840, 130), 500);
    EXPECT_DOUBLE_EQ(device.currentInclination().value(), 13);
    EXPECT_EQ(YpooTreadmillTestPeer::next(device, 3000).action, YpooM5Control::Action::None);
}

TEST_F(YpooTreadmillTest, M1552ConvertsLevelTelemetryAndConfirmsItsQuantizedTarget) {
    YpooTreadmillTestPeer::model(device, true);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(120, 0), 0);
    ASSERT_TRUE(YpooTreadmillTestPeer::requestIncline(device, 2));
    ASSERT_EQ(YpooTreadmillTestPeer::next(device, 0).action, YpooM5Control::Action::Incline);
    // Captured M1552 firmware reports level 3 for a requested 2% (level 3.5).
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(120, 30), 500);
    EXPECT_NEAR(device.currentInclination().value(), 12.0 / 7.0, 1e-9);
    EXPECT_EQ(YpooTreadmillTestPeer::next(device, 3000).action, YpooM5Control::Action::None);
}

TEST_F(YpooTreadmillTest, MalformedTelemetryDoesNotChangeMetricsOrAdvanceDistanceClock) {
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, -10), 0);
    const auto valid = telemetryPacket(1800, 150);
    for (int size = 0; size < valid.size(); ++size)
        YpooTreadmillTestPeer::telemetry(device, valid.left(size), 500);
    EXPECT_DOUBLE_EQ(device.currentSpeed().value(), 10);
    EXPECT_DOUBLE_EQ(device.currentInclination().value(), -1);
    EXPECT_DOUBLE_EQ(device.odometer(), 0);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, -10), 1000);
    EXPECT_NEAR(device.odometer(), 10.0 / 3600.0, 1e-9);
}

TEST_F(YpooTreadmillTest, CapturedRunUsesDriverDistancePathWithoutAnyMotorCommands) {
    device.setPaused(false);
    QFile file(QStringLiteral(":/ypoo-m5/console-run.csv"));
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    ASSERT_EQ(file.readLine().trimmed(), QByteArray("elapsed_ms,speed_centi_kmh"));
    unsigned samples = 0;
    while (!file.atEnd()) {
        const auto fields = file.readLine().trimmed().split(',');
        ASSERT_EQ(fields.size(), 2);
        const auto time = fields[0].toLongLong();
        YpooTreadmillTestPeer::telemetry(device, telemetryPacket(fields[1].toInt(), 0), time);
        ASSERT_EQ(YpooTreadmillTestPeer::next(device, time).action, YpooM5Control::Action::None);
        ++samples;
    }
    EXPECT_EQ(samples, 5867u);
    EXPECT_NEAR(device.odometer(), 6.658622083333333, 1e-9);
}

TEST_F(YpooTreadmillTest, PausedDriverDoesNotCountDistanceAcrossPauseOrTelemetryGap) {
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, 0), 0);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, 0), 1000);
    device.setPaused(true);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, 0), 2000);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, 0), 3000);
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, 0), 10000);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, 0), 11000);
    EXPECT_NEAR(device.odometer(), 20.0 / 3600.0, 1e-9);
}

TEST_F(YpooTreadmillTest, GenericTreadmillAdvertisesBroadVirtualFtmsRangesWithoutIdleStop) {
    StopPolicyTestTreadmill otherTreadmill;
    treadmill &base = otherTreadmill;
    base.stopWhenIdle();
    EXPECT_EQ(otherTreadmill.stopCalls, 0u);
    const auto control = base.virtualTreadmillControl();
    EXPECT_TRUE(control.enabled);
    EXPECT_EQ(control.speedRange, QByteArray::fromHex("000088130a00"));
    EXPECT_EQ(control.inclineRange, QByteArray::fromHex("0cfef4010100"));
}

TEST_F(YpooTreadmillTest, BothYpooModelsQueuePhysicalStopBeforeMotionOrRecording) {
    for (bool ypM1552 : {false, true}) {
        ypootreadmill modelDevice{false, false};
        YpooTreadmillTestPeer::model(modelDevice, ypM1552);
        treadmill &base = modelDevice;
        ASSERT_FALSE(YpooTreadmillTestPeer::stopPending(modelDevice));
        base.stopWhenIdle();
        EXPECT_TRUE(YpooTreadmillTestPeer::stopPending(modelDevice));
    }
}

TEST_F(YpooTreadmillTest, IdleStopHookCancelsPhysicalCountdownWithTargetsQueued) {
    ASSERT_EQ(YpooTreadmillTestPeer::beginCountdown(device), YpooM5Control::Action::Start);
    EXPECT_DOUBLE_EQ(device.currentSpeed().value(), 0);
    EXPECT_EQ(device.elapsedTime(), QTime(0, 0, 0));
    device.setPaused(true); // The UI's stopped guard must retain this recording state.
    treadmill &base = device;
    base.stopWhenIdle();
    YpooTreadmillTestPeer::processRequests(device);
    EXPECT_EQ(YpooTreadmillTestPeer::next(device, YpooTreadmillTestPeer::now(device) + 1100).action,
              YpooM5Control::Action::Stop);
    EXPECT_TRUE(device.isPaused());
    EXPECT_DOUBLE_EQ(device.odometer(), 0);
}

TEST_F(YpooTreadmillTest, AdvertisesModelRangesThroughGenericDeviceInterface) {
    bluetoothdevice &base = device;
    for (bool ypM1552 : {false, true}) {
        YpooTreadmillTestPeer::model(device, ypM1552);
        const auto control = base.virtualTreadmillControl();
        EXPECT_TRUE(control.enabled);
        EXPECT_EQ(control.speedRange, QByteArray::fromHex("640058070a00"));
        EXPECT_EQ(control.inclineRange, QByteArray::fromHex(ypM1552 ? "000078000a00" : "c4ff96000a00"));
    }
}

TEST_F(YpooTreadmillTest, CounterResetAfterConsoleRunPreservesCumulativeDistanceAndNewSessionOffset) {
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance, true);
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance_increment, true);
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 0), 0);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 20), 15000);
    EXPECT_NEAR(device.odometer(), 0.02, 1e-12);
    // The console stops and clears its counter before a training app starts the next run.
    YpooTreadmillTestPeer::telemetry(device, distancePacket(0, 20), 16000);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(0, 0), 17000);
    EXPECT_NEAR(device.odometerFromStartup(), 0.02, 1e-12);
    YpooTreadmillTestPeer::startFromApp(device);
    EXPECT_NEAR(device.odometerFromStartup(), 0.02, 1e-12);
    // QZ starts a new recording by clearing its session offset, not the FTMS cumulative total.
    device.clearStats();
    EXPECT_DOUBLE_EQ(device.odometer(), 0);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 0), 20000);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 10), 30000);
    EXPECT_NEAR(device.odometer(), 0.01, 1e-12);
    EXPECT_NEAR(device.odometerFromStartup(), 0.03, 1e-12);
    CharacteristicNotifier2ACD notifier(&device);
    QByteArray packet;
    notifier.notify(packet);
    ASSERT_GE(packet.size(), 9);
    const unsigned offset = (uint8_t(packet[0]) & 2) ? 6 : 4;
    const uint32_t advertisedMeters = uint8_t(packet[offset]) | (uint32_t(uint8_t(packet[offset + 1])) << 8) |
                                      (uint32_t(uint8_t(packet[offset + 2])) << 16);
    EXPECT_EQ(advertisedMeters, 30u); // The app can subtract its 20 m baseline to record the new 10 m.
}

TEST_F(YpooTreadmillTest, CounterFollowsPausedSamplesAndDoesNotCountThemOnResume) {
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance, true);
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance_increment, true);
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 0), 0);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 10), 10000);
    device.setPaused(true);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 20), 20000);
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 30), 30000);
    EXPECT_NEAR(device.odometer(), 0.02, 1e-12);
}

TEST_F(YpooTreadmillTest, EnablingCounterIncrementsDoesNotAddDistanceAlreadyCountedFromSpeed) {
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(1000, 100), 0);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(1000, 110), 1000);
    const double previousDistance = device.odometer();
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance, true);
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance_increment, true);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(1000, 110), 1500);
    EXPECT_DOUBLE_EQ(device.odometer(), previousDistance);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(1000, 120), 2000);
    EXPECT_NEAR(device.odometer(), previousDistance + 0.01, 1e-12);
}

TEST_F(YpooTreadmillTest, IncrementOptionAloneLeavesDefaultSpeedDistanceUnchanged) {
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance_increment, true);
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(1000, 100), 0);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(1000, 110), 1000);
    EXPECT_NEAR(device.odometer(), 10.0 / 3600.0, 1e-12);
}

TEST_F(YpooTreadmillTest, DirectDistanceWithoutIncrementsKeepsExistingAbsoluteCounterBehavior) {
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance, true);
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(500, 5050), 0);
    EXPECT_DOUBLE_EQ(device.odometer(), 5.05);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(0, 0), 1000);
    EXPECT_DOUBLE_EQ(device.odometer(), 0);
}

TEST_F(YpooTreadmillTest, CapturedDistanceCountersAccumulateAcrossStopsWithoutSpeedIntegration) {
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance, true);
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance_increment, true);
    // De-identified counter-change packets from the owner's runs; unchanged packets are omitted.
    const QString fixtures[] = {QStringLiteral(":/ypoo-m5/distance-20261025.csv"),
                                QStringLiteral(":/ypoo-m5/distance-20261028.csv"),
                                QStringLiteral(":/ypoo-m5/distance-20261030.csv")};
    const double expectedDistances[] = {6.56, 7.21, 6.53}; // 28 October includes two runs separated by a reset.
    const unsigned expectedSamples[] = {658, 724, 655};
    for (unsigned index = 0; index < 3; ++index) {
        for (bool ypM1552 : {false, true}) {
            SCOPED_TRACE(fixtures[index].toStdString());
            SCOPED_TRACE(ypM1552);
            ypootreadmill capturedDevice{false, false};
            YpooTreadmillTestPeer::model(capturedDevice, ypM1552);
            capturedDevice.setPaused(false);
            QFile file(fixtures[index]);
            ASSERT_TRUE(file.open(QIODevice::ReadOnly));
            ASSERT_EQ(file.readLine().trimmed(), QByteArray("elapsed_ms,speed_centi_kmh,distance_m"));
            double previousDistance = 0;
            unsigned samples = 0;
            while (!file.atEnd()) {
                const auto fields = file.readLine().trimmed().split(',');
                ASSERT_EQ(fields.size(), 3);
                YpooTreadmillTestPeer::telemetry(capturedDevice,
                                               distancePacket(fields[1].toInt(), fields[2].toUInt()),
                                               fields[0].toLongLong());
                ASSERT_GE(capturedDevice.odometerFromStartup(), previousDistance);
                previousDistance = capturedDevice.odometerFromStartup();
                ++samples;
            }
            EXPECT_EQ(samples, expectedSamples[index]);
            EXPECT_NEAR(capturedDevice.odometer(), expectedDistances[index], 1e-9);
            CharacteristicNotifier2ACD notifier(&capturedDevice);
            QByteArray packet;
            notifier.notify(packet);
            ASSERT_GE(packet.size(), 9);
            const unsigned offset = (uint8_t(packet[0]) & 2) ? 6 : 4;
            const uint32_t advertisedMeters = uint8_t(packet[offset]) |
                                              (uint32_t(uint8_t(packet[offset + 1])) << 8) |
                                              (uint32_t(uint8_t(packet[offset + 2])) << 16);
            EXPECT_EQ(advertisedMeters, uint32_t(qRound(expectedDistances[index] * 1000.0)));
        }
    }
}

TEST_F(YpooTreadmillTest, MissingCounterPacketsDoNotMixSpeedDistanceIntoCounterMode) {
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance, true);
    settings.qsettings.setValue(QZSettings::treadmill_direct_distance_increment, true);
    device.setPaused(false);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(1000, 100), 0);
    YpooTreadmillTestPeer::telemetry(device, telemetryPacket(1000, 0), 1000);
    EXPECT_DOUBLE_EQ(device.odometer(), 0);
    YpooTreadmillTestPeer::telemetry(device, distancePacket(1000, 110), 2000);
    EXPECT_NEAR(device.odometer(), 0.01, 1e-12);
}
