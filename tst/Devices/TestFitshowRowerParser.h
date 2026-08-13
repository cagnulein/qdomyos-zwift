#pragma once

#include "devices/fitshowrower/fitshowrower.h"
#include <gtest/gtest.h>

TEST(FitshowRowerParser, ParsesCapabilities) {
  const auto packet =
      fitshowrower::parsePacket(QByteArray::fromHex("024102000001004203"));
  ASSERT_TRUE(packet.valid);
  EXPECT_EQ(packet.maxResistance, 0);
  EXPECT_EQ(packet.maxIncline, 0);
}

TEST(FitshowRowerParser, ParsesIdleAndRunningMetrics) {
  const auto idle =
      fitshowrower::parsePacket(QByteArray::fromHex("0242004203"));
  ASSERT_TRUE(idle.valid);
  EXPECT_EQ(idle.status, 0);
  EXPECT_EQ(idle.cadence, 0);
  EXPECT_DOUBLE_EQ(idle.power, 0);

  const auto first = fitshowrower::parsePacket(
      QByteArray::fromHex("0242026100000d00006e0000004203"));
  ASSERT_TRUE(first.valid);
  EXPECT_EQ(first.cadence, 13);
  EXPECT_DOUBLE_EQ(first.power, 11.0);

  const auto second = fitshowrower::parsePacket(
      QByteArray::fromHex("024202ca00001b0000fa0000006b03"));
  ASSERT_TRUE(second.valid);
  EXPECT_EQ(second.cadence, 27);
  EXPECT_DOUBLE_EQ(second.power, 25.0);
}

TEST(FitshowRowerParser, ParsesCumulativeMetricsAndDistanceScaling) {
  const auto first = fitshowrower::parsePacket(
      QByteArray::fromHex("02430124000f8000000800e103"));
  ASSERT_TRUE(first.valid);
  EXPECT_EQ(first.elapsedSeconds, 36);
  EXPECT_EQ(first.distanceMeters, 150);
  EXPECT_DOUBLE_EQ(first.calories, 0);
  EXPECT_EQ(first.strokeCount, 8);

  const auto second = fitshowrower::parsePacket(
      QByteArray::fromHex("0243012e00118000000900f403"));
  ASSERT_TRUE(second.valid);
  EXPECT_EQ(second.elapsedSeconds, 46);
  EXPECT_EQ(second.distanceMeters, 170);
  EXPECT_EQ(second.strokeCount, 9);
}

TEST(FitshowRowerParser, RejectsBadChecksumAndTruncatedFrames) {
  EXPECT_FALSE(fitshowrower::parsePacket(
                   QByteArray::fromHex("0242026100000d00006e0000004303"))
                   .valid);
  EXPECT_FALSE(
      fitshowrower::parsePacket(QByteArray::fromHex("02420261")).valid);
}

TEST(FitshowRowerDetection, OnlyClaimsTheKnownTopiomName) {
  EXPECT_TRUE(fitshowrower::isTopiomDeviceName(QStringLiteral("FS-442900")));
  EXPECT_TRUE(fitshowrower::isTopiomDeviceName(QStringLiteral("fs-442900")));
  EXPECT_FALSE(fitshowrower::isTopiomDeviceName(QStringLiteral("FS-123456")));
}
