#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/trxappgateusbtreadmill/trxappgateusbtreadmill.h"
#include "trainprogram.h"

TEST(TrxAppGateUsTreadmillParserTest, BuildsCapturedFlowFitnessStartCommand) {
    EXPECT_EQ(trxappgateusbtreadmill::flowFitnessStartPacket(),
              QByteArray::fromHex("f0a52ed30298"));
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsCapturedFlowFitnessStopCommand) {
    EXPECT_EQ(trxappgateusbtreadmill::flowFitnessStopPacket(),
              QByteArray::fromHex("f0a52ed3049a"));
}

TEST(TrxAppGateUsTreadmillParserTest, ParsesFlowFitnessMetrics) {
    const QByteArray packet = QByteArray::fromHex("f0b02ed3020304330319024c01180404030101");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    ASSERT_TRUE(metrics.valid);
    EXPECT_DOUBLE_EQ(metrics.speed, 2.3);
    EXPECT_DOUBLE_EQ(metrics.inclination, 3.0);
    EXPECT_DOUBLE_EQ(metrics.kcal, 536.0);
    EXPECT_DOUBLE_EQ(metrics.distance, 3.5);
    EXPECT_EQ(metrics.heart, 175);
    EXPECT_EQ(metrics.elapsed, 121);
}

TEST(TrxAppGateUsTreadmillParserTest, ParsesRuntimeFlowFitnessTelemetryFrame) {
    // Captured from the Android debug log: telemetry is f0 b2, not f0 b0.
    const QByteArray packet = QByteArray::fromHex("f0b22ed30101010101010101010101010102b2");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    ASSERT_TRUE(metrics.valid);
    EXPECT_DOUBLE_EQ(metrics.speed, 0.0);
    EXPECT_DOUBLE_EQ(metrics.inclination, 0.0);
    EXPECT_DOUBLE_EQ(metrics.kcal, 0.0);
    EXPECT_DOUBLE_EQ(metrics.distance, 0.0);
    EXPECT_EQ(metrics.heart, 0);
    EXPECT_EQ(metrics.elapsed, 0);
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsFlowFitnessAcknowledgement) {
    const QByteArray packet = QByteArray::fromHex("f0b02ed3a1");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    EXPECT_FALSE(metrics.valid);
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsShortFlowFitnessPacket) {
    const QByteArray packet = QByteArray::fromHex("f0b02ed3020304330319024c011804040301");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    EXPECT_FALSE(metrics.valid);
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsDifferentProfileSelector) {
    const QByteArray packet = QByteArray::fromHex("f0b023d3020304330319024c01180404030101");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    EXPECT_FALSE(metrics.valid);
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsUnsupportedFlowFitnessFrameType) {
    const QByteArray packet = QByteArray::fromHex("f0b52ed3020304330319024c01180404030101");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    EXPECT_FALSE(metrics.valid);
}

TEST(TrxAppGateUsTreadmillParserTest, ParsesZeroMetricValuesWithoutUnsignedUnderflow) {
    const QByteArray packet = QByteArray::fromHex("f0b02ed3010101010101010101010101010101");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    ASSERT_TRUE(metrics.valid);
    EXPECT_DOUBLE_EQ(metrics.speed, 0.0);
    EXPECT_DOUBLE_EQ(metrics.inclination, 0.0);
    EXPECT_DOUBLE_EQ(metrics.kcal, 0.0);
    EXPECT_DOUBLE_EQ(metrics.distance, 0.0);
    EXPECT_EQ(metrics.heart, 0);
    EXPECT_EQ(metrics.elapsed, 0);
}

TEST(TrxAppGateUsTreadmillParserTest, ParsesHighBitMetricBytesAsUnsigned) {
    // These decoders are shared by the legacy AppGate treadmill profiles too.
    // Bytes >= 0x80 must not be sign-extended on platforms where char is signed.
    const QByteArray packet = QByteArray::fromHex("f0b22ed3828382838283024c82838204040301");

    const auto metrics = trxappgateusbtreadmill::flowFitnessMetricsFromPacket(packet);

    ASSERT_TRUE(metrics.valid);
    EXPECT_DOUBLE_EQ(metrics.speed, 1303.0);
    EXPECT_DOUBLE_EQ(metrics.inclination, 129.0);
    EXPECT_DOUBLE_EQ(metrics.kcal, 33154.0);
    EXPECT_DOUBLE_EQ(metrics.distance, 130.3);
    EXPECT_EQ(metrics.heart, 175);
    EXPECT_EQ(metrics.elapsed, 7929);
}

TEST(TrxAppGateUsTreadmillParserTest, MatchesCapturedFlowFitnessDeviceName) {
    EXPECT_TRUE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("TREADMILL00028")));
    EXPECT_TRUE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("treadmill00028")));
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsGenericTreadmillDeviceNames) {
    EXPECT_FALSE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("TREADMILL")));
    EXPECT_FALSE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("TREADMILLABC")));
    EXPECT_FALSE(trxappgateusbtreadmill::isFlowFitnessDeviceName(QStringLiteral("OTHER00028")));
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsCapturedIconsoleSpeedProgramFrames) {
    const QList<double> speeds = {4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 12.0, 10.0, 8.0, 6.0};

    EXPECT_EQ(trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa6, speeds),
              QByteArray::fromHex("f0a601d30129013d0151020102150229021502010151013d13"));
    EXPECT_EQ(trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa7, speeds),
              QByteArray::fromHex("f0a701d30129013d0151020102150229021502010151013d14"));
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsIconsoleWorkoutWithNoRows) {
    QList<trainrow> rows;
    QList<QByteArray> frames;
    QString error;

    EXPECT_FALSE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("exactly 10 rows")));
    EXPECT_TRUE(frames.isEmpty());
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsIconsoleWorkoutFromTenOneMinuteRows) {
    QList<trainrow> rows;
    for (int i = 0; i < 10; ++i) {
        trainrow row;
        row.duration = QTime(0, 1, 0);
        row.speed = 6.0;
        rows.append(row);
    }

    QList<QByteArray> frames;
    QString error;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error)) << error.toStdString();
    ASSERT_EQ(frames.size(), 8);

    const QList<double> speeds = {6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0, 6.0};
    EXPECT_EQ(frames.at(2), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa6, speeds));
    EXPECT_EQ(frames.at(3), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa7, speeds));
}

TEST(TrxAppGateUsTreadmillParserTest, MapsTenRowsToTwentyHalfMinuteSpeedSlots) {
    QList<trainrow> rows;
    for (int i = 0; i < 10; ++i) {
        trainrow row;
        row.duration = QTime(0, 1, 0);
        row.speed = i < 5 ? 4.0 : 8.0;
        rows.append(row);
    }

    QList<QByteArray> frames;
    QString error;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error)) << error.toStdString();
    ASSERT_EQ(frames.size(), 8);

    const QList<double> fourSpeedSlots = {4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0};
    const QList<double> eightSpeedSlots = {8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0};
    EXPECT_EQ(frames.at(2), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa6, fourSpeedSlots));
    EXPECT_EQ(frames.at(3), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(0xa7, eightSpeedSlots));
}

TEST(TrxAppGateUsTreadmillParserTest, RejectsIconsoleWorkoutWithInvalidRows) {
    QList<trainrow> rows;
    for (int i = 0; i < 10; ++i) {
        trainrow row;
        row.duration = QTime(0, 1, 0);
        row.speed = 6.0;
        rows.append(row);
    }
    rows[0].duration = QTime(0, 0, 0);

    QList<QByteArray> frames;
    QString error;
    EXPECT_FALSE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("ten one-minute")));
    EXPECT_TRUE(frames.isEmpty());
}

TEST(TrxAppGateUsTreadmillParserTest, BuildsCompleteIconsoleWorkoutUploadSequence) {
    const QList<double> speeds = {4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 12.0, 10.0, 8.0, 6.0};
    QList<trainrow> rows;
    for (int i = 0; i < speeds.size(); ++i) {
        trainrow row;
        row.duration = QTime(0, 1, 0);
        row.speed = speeds.at(i);
        row.inclination = i;
        rows.append(row);
    }

    QList<QByteArray> frames;
    QString error;
    ASSERT_TRUE(trxappgateusbtreadmill::buildIconsoleWorkoutFrames(rows, &frames, &error)) << error.toStdString();
    ASSERT_EQ(frames.size(), 8);

    EXPECT_EQ(frames.at(0), QByteArray::fromHex("f0a301d3020101010101016f"));
    EXPECT_EQ(frames.at(1), QByteArray::fromHex("f0a401d30a010101010101010101017c"));
    EXPECT_EQ(frames.at(2), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(
                                0xa6, {4.0, 4.0, 6.0, 6.0, 8.0, 8.0, 10.0, 10.0, 12.0, 12.0}));
    EXPECT_EQ(frames.at(3), trxappgateusbtreadmill::iconsoleSpeedProgramFrame(
                                0xa7, {14.0, 14.0, 12.0, 12.0, 10.0, 10.0, 8.0, 8.0, 6.0, 6.0}));
    EXPECT_EQ(frames.at(4), QByteArray::fromHex("f0a801d30000010102020303040405050606070708080909c6"));
    EXPECT_EQ(frames.at(5), QByteArray::fromHex("f0a901d3001e001e001e001e001e001e001e001e001e001e99"));
    EXPECT_EQ(frames.at(6), QByteArray::fromHex("f0aa01d3001e001e001e001e001e001e001e001e001e001e9a"));
    EXPECT_EQ(frames.at(7), QByteArray::fromHex("f0af01d30275"));
}
