#include <gtest/gtest.h>

#include "devices/fitshowtreadmill/runnclosedloopcontroller.h"

TEST(FitShowRunnClosedLoop, IgnoresInaccurateStartupReadings) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    controller.sample(0.028, 1000);
    EXPECT_FALSE(controller.update(1000, true, 0.7, 0, 0, 22, 0.2).changed);

    bool corrected = false;
    for (int ms = 2000; ms <= 14000; ms += 1000) {
        controller.sample(9.5, ms);
        const double machine = ms < 5000 ? 0.7 : 10.0;
        const auto result = controller.update(ms, true, machine, 0, 0, 22, 0.2);
        if (result.changed) {
            corrected = true;
            EXPECT_DOUBLE_EQ(result.command, 10.2);
            break;
        }
    }
    EXPECT_TRUE(corrected);
}

TEST(FitShowRunnClosedLoop, DeadbandPreventsHunting) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    for (int ms = 1000; ms <= 30000; ms += 1000) {
        controller.sample(ms % 2000 == 0 ? 10.1 : 9.95, ms);
        EXPECT_FALSE(controller.update(ms, true, 10.0, 0, 0, 22, 0.2).changed);
    }
}

TEST(FitShowRunnClosedLoop, StaleOrSlowRunnReadingsDoNotAdjustMotor) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    controller.sample(0.028, 1000);
    controller.sample(4.5, 2000);
    EXPECT_FALSE(controller.update(9000, true, 10.0, 0, 0, 22, 0.2).changed);
    for (int ms = 10000; ms <= 14000; ms += 1000)
        controller.sample(9.0, ms);
    EXPECT_FALSE(controller.update(17000, true, 10.0, 0, 0, 22, 0.2).changed);
}

TEST(FitShowRunnClosedLoop, ManualConsoleChangeCancelsControl) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    EXPECT_FALSE(controller.update(5000, true, 10.0, 0, 0, 22, 0.2).changed);
    EXPECT_FALSE(controller.update(6000, true, 11.0, 0, 0, 22, 0.2).cancelled);
    const auto result = controller.update(8600, true, 11.0, 0, 0, 22, 0.2);
    EXPECT_TRUE(result.cancelled);
    EXPECT_FALSE(controller.active());
}

TEST(FitShowRunnClosedLoop, StopCancelsActiveControl) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    controller.update(5000, true, 10.0, 0, 0, 22, 0.2);
    EXPECT_TRUE(controller.update(5500, false, 0.0, 0, 0, 22, 0.2).cancelled);
    EXPECT_FALSE(controller.active());
}

TEST(FitShowRunnClosedLoop, NewWorkoutIntervalResetsFeedback) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    for (int ms = 1000; ms <= 9000; ms += 1000) {
        controller.sample(9.5, ms);
        controller.update(ms, true, 10.0, 0, 0, 22, 0.2);
    }
    controller.start(12.0, 12.0, 10000);
    EXPECT_DOUBLE_EQ(controller.target(), 12.0);
    EXPECT_DOUBLE_EQ(controller.command(), 12.0);
    EXPECT_FALSE(controller.update(10500, true, 10.0, 0, 0, 22, 0.2).changed);
}

TEST(FitShowRunnClosedLoop, CorrectionsStayBounded) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    double command = 10.0;
    for (int ms = 1000; ms <= 180000; ms += 1000) {
        controller.sample(9.0, ms);
        const auto decision = controller.update(ms, true, command, 0, 0, 22, 0.2);
        if (decision.changed) command = decision.command;
        EXPECT_LE(command, 11.5);
        EXPECT_GE(command, 10.0);
    }
}

TEST(FitShowRunnClosedLoop, LargeMismatchIsRejected) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    for (int ms = 1000; ms <= 30000; ms += 1000) {
        controller.sample(6.0, ms);
        EXPECT_FALSE(controller.update(ms, true, 10.0, 0, 0, 22, 0.2).changed);
    }
}

TEST(FitShowRunnClosedLoop, UnstartedTargetExpiresInsteadOfControllingLaterRun) {
    FitShowRunnClosedLoopController controller;
    controller.start(10.0, 10.0, 0);
    EXPECT_FALSE(controller.update(29000, false, 0.0, 0, 0, 22, 0.2).cancelled);
    EXPECT_TRUE(controller.update(30000, false, 0.0, 0, 0, 22, 0.2).cancelled);
    EXPECT_FALSE(controller.active());
}
