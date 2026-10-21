#include <gtest/gtest.h>
#include <QFile>
#include <vector>

#include "devices/horizontreadmill/ypoom5control.h"
#include "devices/horizontreadmill/ypoom5distance.h"

namespace {
class YpooM5DistanceTest : public ::testing::Test {
  protected:
    struct Sample {
        int64_t time;
        double speed;
    };
    std::vector<Sample> samples;

    void SetUp() override {
        // Only relative timestamps and belt speeds from the owner's 2026-10-21
        // console-started run; no identifiers or account data are included.
        QFile file(QStringLiteral(":/ypoo-m5/console-run.csv"));
        ASSERT_TRUE(file.open(QIODevice::ReadOnly));
        ASSERT_EQ(file.readLine().trimmed(), QByteArray("elapsed_ms,speed_centi_kmh"));
        while (!file.atEnd()) {
            const auto fields = file.readLine().trimmed().split(',');
            ASSERT_EQ(fields.size(), 2);
            bool validTime = false;
            bool validSpeed = false;
            const int64_t time = fields[0].toLongLong(&validTime);
            const double speed = fields[1].toInt(&validSpeed) / 100.0;
            ASSERT_TRUE(validTime && validSpeed);
            samples.push_back({time, speed});
        }
        ASSERT_EQ(samples.size(), 5867u);
    }
};
} // namespace

TEST_F(YpooM5DistanceTest, CapturedRunPreservesTheFullSpeedIntegratedDistance) {
    YpooM5Distance distance;
    double total = 0;
    for (const auto &sample : samples)
        total += distance.update(sample.time, sample.speed, true);
    // Independent audit of the source log: QZ displayed 5.33126 km, while
    // integration of the recorded belt speed gives 6.658622083333333 km.
    EXPECT_NEAR(total, 6.658622083333333, 0.000000001);
}

TEST_F(YpooM5DistanceTest, CapturedConsoleStartResumesRecordingWithoutSendingMotorCommands) {
    YpooM5Control control;
    YpooM5Distance distance;
    bool recording = false; // Reproduce the stopped QZ state at the start of the captured run.
    int64_t nextUiUpdate = 0;
    unsigned resumes = 0;
    double total = 0;
    for (const auto &sample : samples) {
        // homeform checks the recording transition once per second, independently
        // of the approximately 450 ms Bluetooth packet interval.
        while (nextUiUpdate <= sample.time) {
            if (!recording && control.canResumeRecording(nextUiUpdate)) {
                recording = true;
                ++resumes;
            } else if (!control.running()) {
                recording = false;
            }
            nextUiUpdate += 1000;
        }
        control.telemetry(sample.speed, 0, sample.time);
        total += distance.update(sample.time, sample.speed, recording);
        EXPECT_EQ(control.next(sample.time).action, YpooM5Control::Action::None);
    }
    EXPECT_GT(resumes, 0u);
    // Allow at most one UI tick plus one packet of start-detection latency.
    EXPECT_NEAR(total, 6.658622083333333, 0.004);
}

TEST(YpooM5DistanceClockTest, PausedSamplesAdvanceTheClockWithoutCountingPausedTime) {
    YpooM5Distance distance;
    EXPECT_DOUBLE_EQ(distance.update(0, 9.5, true), 0);
    EXPECT_NEAR(distance.update(1000, 9.5, true), 9.5 / 3600.0, 1e-12);
    for (int64_t time = 2000; time <= 12000; time += 1000)
        EXPECT_DOUBLE_EQ(distance.update(time, 9.5, false), 0);
    EXPECT_NEAR(distance.update(13000, 9.5, true), 9.5 / 3600.0, 1e-12);
}

TEST(YpooM5DistanceClockTest, SpeedChangesUseThePreviousSampleAndDuplicateTimesDoNotAddDistance) {
    YpooM5Distance distance;
    EXPECT_DOUBLE_EQ(distance.update(0, 9.5, true), 0);
    EXPECT_NEAR(distance.update(450, 9.6, true), 9.5 * 450 / 3600000.0, 1e-12);
    EXPECT_DOUBLE_EQ(distance.update(450, 9.6, true), 0);
    EXPECT_NEAR(distance.update(900, 0, true), 9.6 * 450 / 3600000.0, 1e-12);
    EXPECT_DOUBLE_EQ(distance.update(1350, 0, true), 0);
}

TEST(YpooM5DistanceClockTest, MissingTelemetryAndReconnectDoNotExtrapolateUnobservedMotion) {
    YpooM5Distance distance;
    EXPECT_DOUBLE_EQ(distance.update(0, 9.5, true), 0);
    EXPECT_DOUBLE_EQ(distance.update(10000, 9.5, true), 0);
    EXPECT_NEAR(distance.update(10450, 9.5, true), 9.5 * 450 / 3600000.0, 1e-12);
    distance.reset();
    EXPECT_DOUBLE_EQ(distance.update(20000, 9.5, true), 0);
}
