#include <gtest/gtest.h>
#include <QFile>

#include "Tools/testsettings.h"
#include "devices/ypootreadmill/ypootreadmill.h"

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

class YpooTreadmillTest : public ::testing::Test {
  protected:
    TestSettings settings{QStringLiteral("QZTests"), QStringLiteral("YpooTreadmill")};
    ypootreadmill device{false, false};
    void SetUp() override { settings.activate(); }
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

TEST_F(YpooTreadmillTest, DefaultDriverDoesNotOptIntoIdleStopOrVirtualControl) {
    StopPolicyTestTreadmill otherTreadmill;
    treadmill &base = otherTreadmill;
    base.stopWhenIdle();
    EXPECT_EQ(otherTreadmill.stopCalls, 0u);
    const auto control = base.virtualTreadmillControl();
    EXPECT_FALSE(control.enabled);
    EXPECT_TRUE(control.speedRange.isEmpty());
    EXPECT_TRUE(control.inclineRange.isEmpty());
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
