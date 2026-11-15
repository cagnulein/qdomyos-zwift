#pragma once

#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/trxappgateusbtreadmill/trxappgateusbtreadmill.h"

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
