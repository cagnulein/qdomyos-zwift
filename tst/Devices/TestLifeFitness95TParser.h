#pragma once

#include <QByteArray>
#include <gtest/gtest.h>

#include "devices/lifefitnesstreadmill/lifefitnesstreadmill.h"

TEST(LifeFitness95TFtmsParserTest, ParsesTheSanitizedTelemetryFixture) {
    const auto data = lifefitnesstreadmill::parseFtmsTreadmillData(
        QByteArray::fromHex("9e045c03e2022a00001e00ff7f0900ff7f0300ffffff1600"));

    ASSERT_TRUE(data.valid);
    ASSERT_TRUE(data.hasSpeed);
    EXPECT_DOUBLE_EQ(data.speedKmh, 8.6);
    ASSERT_TRUE(data.hasAverageSpeed);
    EXPECT_DOUBLE_EQ(data.averageSpeedKmh, 7.38);
    ASSERT_TRUE(data.hasDistance);
    EXPECT_DOUBLE_EQ(data.distanceMeters, 42.0);
    ASSERT_TRUE(data.hasInclination);
    EXPECT_DOUBLE_EQ(data.inclinationPercent, 3.0);
    ASSERT_TRUE(data.hasEnergy);
    EXPECT_DOUBLE_EQ(data.totalEnergyKcal, 3.0);
    ASSERT_TRUE(data.hasElapsedTime);
    EXPECT_EQ(data.elapsedSeconds, 22);
}

TEST(LifeFitness95TFtmsParserTest, RejectsAChoppedMeasurement) {
    const auto data = lifefitnesstreadmill::parseFtmsTreadmillData(QByteArray::fromHex("9e045c03"));

    EXPECT_FALSE(data.valid);
}

TEST(LifeFitness95TFtmsParserTest, DoesNotExposeFtmsSentinels) {
    const auto data = lifefitnesstreadmill::parseFtmsTreadmillData(
        QByteArray::fromHex("88000000ff7fff7fffffffffff"));

    ASSERT_TRUE(data.valid);
    EXPECT_FALSE(data.hasInclination);
    EXPECT_FALSE(data.hasEnergy);
}
