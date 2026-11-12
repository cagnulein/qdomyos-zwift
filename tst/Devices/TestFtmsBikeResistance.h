#pragma once

#include <gtest/gtest.h>
#include <QString>

#include "devices/ftmsbike/ftmsbike.h"

TEST(FtmsBikeResistanceTest, NormalizesOnlyExactZBike20Name) {
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 50), 0);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 60), 1);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 70), 2);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 100), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 110), 6);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 120), 7);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 200), 15);
}

TEST(FtmsBikeResistanceTest, RoundsAndClampsZBike20Resistance) {
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 98), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 99), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 101), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 102), 5);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 0), 0);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0"), 999), 15);
}

TEST(FtmsBikeResistanceTest, KeepsRawResistanceForOtherFtmsDeviceNames) {
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("ZBike2.0 Pro"), 100), 100);
    EXPECT_EQ(ftmsbike::normalizedResistanceForDevice(QStringLiteral("Other FTMS Bike"), 100), 100);
}

TEST(FtmsBikeResistanceTest, ConvertsDisplayedZBike20ResistanceBackToRawOnlyWhenNeeded) {
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0"), 0), 50);
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0"), 5), 100);
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0"), 15), 200);
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0"), -1), -1);
    EXPECT_EQ(ftmsbike::rawResistanceForDevice(QStringLiteral("ZBike2.0 Pro"), 5), 5);
}