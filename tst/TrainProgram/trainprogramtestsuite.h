#ifndef TRAINPROGRAMTESTSUITE_H
#define TRAINPROGRAMTESTSUITE_H

#include "gtest/gtest.h"

class TrainProgramTestSuite: public testing::Test {

public:
    TrainProgramTestSuite();

    /**
     * @brief Test that a single-row 40-minute workout completes at exactly 40:00
     * Bug: Currently stops at 39:59 (missing 1 second)
     */
    void test_singleRow40MinuteWorkout_shouldReach40Minutes();

    /**
     * @brief Test that a multi-row workout completes all rows correctly
     * Bug: Last row stops 1 second early (first rows complete correctly)
     */
    void test_multiRowWorkout_lastRowMissing1Second();
};

TEST_F(TrainProgramTestSuite, Test_SingleRow40MinuteWorkout_ShouldReach40Minutes) {
    this->test_singleRow40MinuteWorkout_shouldReach40Minutes();
}

TEST_F(TrainProgramTestSuite, Test_MultiRowWorkout_LastRowMissing1Second) {
    this->test_multiRowWorkout_lastRowMissing1Second();
}

#endif // TRAINPROGRAMTESTSUITE_H
