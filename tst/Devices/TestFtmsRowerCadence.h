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

TEST(FtmsRowerCadenceTest, CalculatesCadenceOnlyAfterTwoRealStrokeIncrements) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3000, 22);

    EXPECT_NEAR(calculator.update(102, 6000, 22), 20, 0.001);
}

TEST(FtmsRowerCadenceTest, RollingWindowSmoothsCadenceFromStrokeCount) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3000, 22);
    calculator.update(102, 6000, 22);
    calculator.update(103, 9000, 44);

    EXPECT_NEAR(calculator.cadence(), 20, 0.001);
}

TEST(FtmsRowerCadenceTest, StalePauseResetsHistoryBeforeResume) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 22);
    calculator.update(101, 3000, 22);
    calculator.update(102, 6000, 22);
    ASSERT_TRUE(calculator.isStale(12001));

    EXPECT_DOUBLE_EQ(calculator.update(102, 12001, 22), 0);
    EXPECT_DOUBLE_EQ(calculator.update(103, 15001, 22), 0);
    EXPECT_NEAR(calculator.update(104, 18001, 22), 20, 0.001);
}

TEST(FtmsRowerCadenceTest, SlowRowingGetsAWindowBasedTimeout) {
    ftmsrowerCadenceCalculator calculator;

    calculator.update(100, 0, 15);
    calculator.update(101, 4000, 15);

    EXPECT_FALSE(calculator.isStale(7000));
    EXPECT_TRUE(calculator.isStale(12001));
}
