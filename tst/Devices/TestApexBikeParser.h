#pragma once

#include <gtest/gtest.h>
#include <QByteArray>
#include <QString>

#include "devices/apexbike/apexbike.h"

TEST(ApexBikeWlt8266BmRegressionTest, DistanceCounterMetricsAreDisabledForWlt8266BmBikes) {
    EXPECT_FALSE(apexbike::usesWlt8266bmDistanceCounterMetrics(QStringLiteral("WLT8266BM_025B")));
    EXPECT_FALSE(apexbike::usesWlt8266bmDistanceCounterMetrics(QStringLiteral("WLT8266BM_07E2")));
    EXPECT_FALSE(apexbike::usesWlt8266bmDistanceCounterMetrics(QStringLiteral("WLT8266BM_1234")));
    EXPECT_FALSE(apexbike::usesWlt8266bmDistanceCounterMetrics(QStringLiteral("wlt8266bm_07e2")));
    EXPECT_FALSE(apexbike::usesWlt8266bmDistanceCounterMetrics(QStringLiteral("APEX Bike")));
}

TEST(ApexBikeWlt8266BmRegressionTest, Wlt8266BmPacketsDoNotUseZeroX30DistanceCounterMetrics) {
    const QByteArray movementPacket = QByteArray::fromHex("ea503000020000000082");
    const QByteArray resistancePacket = QByteArray::fromHex("ea503100001a0000009b");

    EXPECT_FALSE(apexbike::isWlt8266bmDistanceCounterMetricsPacket(
        QStringLiteral("WLT8266BM_025B"), movementPacket));
    EXPECT_FALSE(apexbike::isWlt8266bmDistanceCounterMetricsPacket(
        QStringLiteral("WLT8266BM_0000"), movementPacket));
    EXPECT_FALSE(apexbike::isWlt8266bmDistanceCounterMetricsPacket(
        QStringLiteral("WLT8266BM_025B"), resistancePacket));
    EXPECT_FALSE(apexbike::isWlt8266bmDistanceCounterMetricsPacket(
        QStringLiteral("WLT8266BM"), movementPacket));
}

TEST(ApexBikeWlt8266BmRegressionTest, DistanceCounterComesFromBytesThreeAndFourInLoggedZeroX30Packets) {
    EXPECT_EQ(apexbike::wlt8266bmDistanceCounterFromPacket(QByteArray::fromHex("ea503000000000000080")), 0);
    EXPECT_EQ(apexbike::wlt8266bmDistanceCounterFromPacket(QByteArray::fromHex("ea503000020000000082")), 2);
    EXPECT_EQ(apexbike::wlt8266bmDistanceCounterFromPacket(QByteArray::fromHex("ea5030002100000000a1")), 33);
}

TEST(ApexBikeWlt8266BmRegressionTest, LoggedDistanceCounterDeltaProducesNonZeroMetrics) {
    const double speed = apexbike::wlt8266bmSpeedFromDistanceCounterDelta(1, 1200);

    EXPECT_NEAR(speed, 15.999, 0.001);
    EXPECT_NEAR(apexbike::wlt8266bmCadenceFromSpeed(speed), 42.666, 0.001);
}

TEST(ApexBikeWlt8266BmRegressionTest, InvalidDistanceCounterDeltaDoesNotProduceMetrics) {
    EXPECT_EQ(apexbike::wlt8266bmSpeedFromDistanceCounterDelta(0, 1200), 0);
    EXPECT_EQ(apexbike::wlt8266bmSpeedFromDistanceCounterDelta(1, 0), 0);
}

TEST(ApexBikeWlt8266BmRegressionTest, Wlt8266Bm07e2RejectsLoggedCadenceSpikePackets) {
    const QString deviceName = QStringLiteral("WLT8266BM_07E2");

    // Captured on 2026-10-30: these packets previously generated 328, 394, and 302 RPM.
    EXPECT_TRUE(apexbike::isWlt8266bm07e2CadenceSpikePacket(
        deviceName, QByteArray::fromHex("ea503102a40c00000b3e")));
    EXPECT_TRUE(apexbike::isWlt8266bm07e2CadenceSpikePacket(
        deviceName, QByteArray::fromHex("ea503101c50f0000489e")));
    EXPECT_TRUE(apexbike::isWlt8266bm07e2CadenceSpikePacket(
        deviceName, QByteArray::fromHex("ea503102970f0000547d")));

    // Device-name matching should not depend on the advertisement's case.
    EXPECT_TRUE(apexbike::isWlt8266bm07e2CadenceSpikePacket(
        QStringLiteral("wlt8266bm_07e2"), QByteArray::fromHex("ea503101c50f0000489e")));
}

TEST(ApexBikeWlt8266BmRegressionTest, Wlt8266Bm07e2AcceptsNormalAndStationaryCadencePackets) {
    const QString deviceName = QStringLiteral("WLT8266BM_07E2");
    EXPECT_FALSE(apexbike::isWlt8266bm07e2CadenceSpikePacket(
        deviceName, QByteArray::fromHex("ea5031002c0c00000bc4")));
    EXPECT_FALSE(apexbike::isWlt8266bm07e2CadenceSpikePacket(
        deviceName, QByteArray::fromHex("ea503100000c0000008d")));
}

TEST(ApexBikeWlt8266BmRegressionTest, CadenceSpikeFilterDoesNotChangeOtherApexVariantsOrPacketTypes) {
    const QByteArray spikePacket = QByteArray::fromHex("ea503101c50f0000489e");
    EXPECT_FALSE(apexbike::isWlt8266bm07e2CadenceSpikePacket(QStringLiteral("WLT8266BM_025B"), spikePacket));
    EXPECT_FALSE(apexbike::isWlt8266bm07e2CadenceSpikePacket(QStringLiteral("WLT8266BM"), spikePacket));
    EXPECT_FALSE(apexbike::isWlt8266bm07e2CadenceSpikePacket(QStringLiteral("APEX Bike"), spikePacket));
    EXPECT_FALSE(apexbike::isWlt8266bm07e2CadenceSpikePacket(
        QStringLiteral("WLT8266BM_07E2"), QByteArray::fromHex("ea503000020000000082")));
    EXPECT_FALSE(apexbike::isWlt8266bm07e2CadenceSpikePacket(
        QStringLiteral("WLT8266BM_07E2"), QByteArray::fromHex("ea503101c5")));
}
