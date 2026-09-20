#pragma once

#include <gtest/gtest.h>
#include <QString>

#include "devices/ftmsrower/ftmsrower.h"
#include "devices/ftmsrower/ftmsrowercadence.h"

TEST(FtmsRowerCadenceTest, UsesStrokeCountOnlyForJorotoMr280Pro) {
    EXPECT_TRUE(ftmsrower::usesJorotoStrokeCountCadence(QStringLiteral("JOROTO-MR280PRO")));
    EXPECT_TRUE(ftmsrower::usesJorotoStrokeCountCadence(QStringLiteral("joroto-mr280pro")));
    EXPECT_FALSE(ftmsrower::usesJorotoStrokeCountCadence(QStringLiteral("JOROTO-MR280")));
    EXPECT_FALSE(ftmsrower::usesJorotoStrokeCountCadence(QStringLiteral("Concept2 PM5")));
}

TEST(FtmsRowerCadenceTest, DoesNotUseReportedCadenceDuringStartup) {
    ftmsrowerCadenceCalculator calculator;

    EXPECT_DOUBLE_EQ(calculator.update(100, 0, 22), 0);
    EXPECT_DOUBLE_EQ(calculator.update(100, 480, 80), 0);
    EXPECT_DOUBLE_EQ(calculator.update(101, 3000, 22), 0);
}

TEST(FtmsRowerCadenceTest, CalculatesCadenceOnlyAfterFourRealStrokeIntervals) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3000, 22);
    calculator.update(102, 6000, 22);
    calculator.update(103, 9000, 22);

    EXPECT_DOUBLE_EQ(calculator.cadence(), 0);
    calculator.update(104, 12000, 22);
    EXPECT_DOUBLE_EQ(calculator.cadence(), 0);
    EXPECT_NEAR(calculator.update(105, 15000, 22), 20, 0.001);
}

TEST(FtmsRowerCadenceTest, WarmupStaysFreshWhileCollectingFourIntervals) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3000, 22);
    calculator.update(102, 6000, 22);

    EXPECT_FALSE(calculator.isStale(11000));
    EXPECT_DOUBLE_EQ(calculator.cadence(), 0);

    calculator.update(103, 9000, 22);
    EXPECT_FALSE(calculator.isStale(12000));
    calculator.update(104, 12000, 22);
    EXPECT_DOUBLE_EQ(calculator.cadence(), 0);
    EXPECT_NEAR(calculator.update(105, 15000, 22), 20, 0.001);
}

TEST(FtmsRowerCadenceTest, RollingWindowSmoothsCadenceFromStrokeCount) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3000, 22);
    calculator.update(102, 6000, 22);
    calculator.update(103, 9000, 44);
    calculator.update(104, 12000, 22);
    calculator.update(105, 15000, 22);

    EXPECT_NEAR(calculator.cadence(), 20, 0.001);
}

TEST(FtmsRowerCadenceTest, MedianRejectsOneJorotoTimingSpike) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3000, 22);
    calculator.update(102, 6000, 22);
    calculator.update(103, 9000, 22);
    calculator.update(104, 12000, 22);
    calculator.update(105, 13000, 22);
    calculator.update(106, 18000, 22);
    calculator.update(107, 21000, 22);

    EXPECT_NEAR(calculator.cadence(), 20, 0.001);
}

TEST(FtmsRowerCadenceTest, MedianUsesIndividualIntervalsNotOverlappingWindows) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3121, 22);
    calculator.update(102, 4081, 22);
    calculator.update(103, 6241, 22);
    calculator.update(104, 8041, 22);
    calculator.update(105, 10082, 22);
    calculator.update(106, 11041, 22);
    calculator.update(107, 14041, 22);
    calculator.update(108, 17161, 22);

    // The seven latest individual intervals have a median of about 2.041 s.
    EXPECT_NEAR(calculator.cadence(), 60000.0 / 2041.0, 0.001);
}

TEST(FtmsRowerCadenceTest, StalePauseResetsHistoryBeforeResume) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3000, 22);
    calculator.update(102, 6000, 22);
    calculator.update(103, 9000, 22);
    calculator.update(104, 12000, 22);
    ASSERT_TRUE(calculator.isStale(18001));

    EXPECT_DOUBLE_EQ(calculator.update(104, 18001, 22), 0);
    EXPECT_DOUBLE_EQ(calculator.update(105, 21001, 22), 0);
    EXPECT_DOUBLE_EQ(calculator.update(106, 24001, 22), 0);
    EXPECT_DOUBLE_EQ(calculator.update(107, 27001, 22), 0);
    EXPECT_DOUBLE_EQ(calculator.update(108, 30001, 22), 0);
    EXPECT_NEAR(calculator.update(109, 33001, 22), 20, 0.001);
}

TEST(FtmsRowerCadenceTest, SlowRowingGetsAWindowBasedTimeout) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 15);
    calculator.update(101, 4000, 15);

    EXPECT_FALSE(calculator.isStale(7000));
    EXPECT_TRUE(calculator.isStale(14001));
}
