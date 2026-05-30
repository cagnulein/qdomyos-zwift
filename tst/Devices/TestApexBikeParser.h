#pragma once

#include <gtest/gtest.h>
#include <QByteArray>
#include <QString>

#include "devices/apexbike/apexbike.h"

TEST(ApexBikeWlt8266Bm025BRegressionTest, DistanceCounterMetricsAreScopedToExactLoggedDeviceName) {
    EXPECT_TRUE(apexbike::usesWlt8266bm025BDistanceCounterMetrics(QStringLiteral("WLT8266BM_025B")));

    EXPECT_FALSE(apexbike::usesWlt8266bm025BDistanceCounterMetrics(QStringLiteral("WLT8266BM")));
    EXPECT_FALSE(apexbike::usesWlt8266bm025BDistanceCounterMetrics(QStringLiteral("WLT8266BM_0000")));
    EXPECT_FALSE(apexbike::usesWlt8266bm025BDistanceCounterMetrics(QStringLiteral("APEX Bike")));
}

TEST(ApexBikeWlt8266Bm025BRegressionTest, LoggedDeviceUsesZeroX30PacketsForDistanceCounterMetrics) {
    const QByteArray movementPacket = QByteArray::fromHex("ea503000020000000082");
    const QByteArray resistancePacket = QByteArray::fromHex("ea503100001a0000009b");

    EXPECT_TRUE(apexbike::isWlt8266bm025BDistanceCounterMetricsPacket(
        QStringLiteral("WLT8266BM_025B"), movementPacket));
    EXPECT_FALSE(apexbike::isWlt8266bm025BDistanceCounterMetricsPacket(
        QStringLiteral("WLT8266BM_025B"), resistancePacket));
    EXPECT_FALSE(apexbike::isWlt8266bm025BDistanceCounterMetricsPacket(
        QStringLiteral("WLT8266BM_0000"), movementPacket));
}

TEST(ApexBikeWlt8266Bm025BRegressionTest, DistanceCounterComesFromBytesThreeAndFourInLoggedZeroX30Packets) {
    EXPECT_EQ(apexbike::wlt8266bm025BDistanceCounterFromPacket(QByteArray::fromHex("ea503000000000000080")), 0);
    EXPECT_EQ(apexbike::wlt8266bm025BDistanceCounterFromPacket(QByteArray::fromHex("ea503000020000000082")), 2);
    EXPECT_EQ(apexbike::wlt8266bm025BDistanceCounterFromPacket(QByteArray::fromHex("ea5030002100000000a1")), 33);
}

TEST(ApexBikeWlt8266Bm025BRegressionTest, LoggedDistanceCounterDeltaProducesNonZeroMetrics) {
    const double speed = apexbike::wlt8266bm025BSpeedFromDistanceCounterDelta(1, 1200);

    EXPECT_NEAR(speed, 15.999, 0.001);
    EXPECT_NEAR(apexbike::wlt8266bm025BCadenceFromSpeed(speed), 42.666, 0.001);
}

TEST(ApexBikeWlt8266Bm025BRegressionTest, InvalidDistanceCounterDeltaDoesNotProduceMetrics) {
    EXPECT_EQ(apexbike::wlt8266bm025BSpeedFromDistanceCounterDelta(0, 1200), 0);
    EXPECT_EQ(apexbike::wlt8266bm025BSpeedFromDistanceCounterDelta(1, 0), 0);
}
