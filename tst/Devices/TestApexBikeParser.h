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

TEST(ApexBikeWlt8266BmRegressionTest, Wlt8266BmFamilyRejectsLoggedCadenceSpikePackets) {
    // Observed on WLT8266BM_07E2 on 2026-10-30: 328, 394 and 302 RPM spikes.
    const QByteArray spikePackets[] = {
        QByteArray::fromHex("ea503102a40c00000b3e"),
        QByteArray::fromHex("ea503101c50f0000489e"),
        QByteArray::fromHex("ea503102970f0000547d")
    };
    const QString deviceNames[] = {
        QStringLiteral("WLT8266BM_07E2"),
        QStringLiteral("WLT8266BM_025B"),
        QStringLiteral("WLT8266BM_1234"),
        QStringLiteral("WLT8266BM"),
        QStringLiteral("wlt8266bm_07e2")
    };

    for (const QString &deviceName : deviceNames) {
        for (const QByteArray &packet : spikePackets) {
            EXPECT_TRUE(apexbike::isWlt8266bmCadenceSpikePacket(deviceName, packet))
                << "Device " << deviceName.toStdString() << " packet " << packet.toHex().toStdString();
        }
    }
}

TEST(ApexBikeWlt8266BmRegressionTest, Wlt8266BmFamilyAcceptsNormalAndStationaryCadencePackets) {
    const QString deviceNames[] = {
        QStringLiteral("WLT8266BM_07E2"),
        QStringLiteral("WLT8266BM_025B"),
        QStringLiteral("WLT8266BM")
    };
    for (const QString &deviceName : deviceNames) {
        EXPECT_FALSE(apexbike::isWlt8266bmCadenceSpikePacket(
            deviceName, QByteArray::fromHex("ea5031002c0c00000bc4")));
        EXPECT_FALSE(apexbike::isWlt8266bmCadenceSpikePacket(
            deviceName, QByteArray::fromHex("ea503100000c0000008d")));
    }
}

TEST(ApexBikeWlt8266BmRegressionTest, CadenceSpikeFilterDoesNotChangeOtherApexTypesOrPacketFormats) {
    const QByteArray spikePacket = QByteArray::fromHex("ea503101c50f0000489e");
    EXPECT_FALSE(apexbike::isWlt8266bmCadenceSpikePacket(QStringLiteral("APEX Bike"), spikePacket));
    EXPECT_FALSE(apexbike::isWlt8266bmCadenceSpikePacket(QStringLiteral("WLT8267BM_07E2"), spikePacket));
    EXPECT_FALSE(apexbike::isWlt8266bmCadenceSpikePacket(QStringLiteral("OTHER_BIKE"), spikePacket));
    EXPECT_FALSE(apexbike::isWlt8266bmCadenceSpikePacket(
        QStringLiteral("WLT8266BM_07E2"), QByteArray::fromHex("ea503000020000000082")));
    EXPECT_FALSE(apexbike::isWlt8266bmCadenceSpikePacket(
        QStringLiteral("WLT8266BM_07E2"), QByteArray::fromHex("ea503101c5")));
}
