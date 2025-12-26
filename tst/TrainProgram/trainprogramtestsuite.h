#ifndef TRAINPROGRAMTESTSUITE_H
#define TRAINPROGRAMTESTSUITE_H

#include "gtest/gtest.h"

class TrainProgramTestSuite: public testing::Test {

public:
    TrainProgramTestSuite();

    /**
     * @brief Test totalElapsedTime() for workouts shorter than 60 seconds
     */
    void test_totalElapsedTime_shortWorkout();

    /**
     * @brief Test totalElapsedTime() for workouts exactly 60 seconds
     */
    void test_totalElapsedTime_exactly60Seconds();

    /**
     * @brief Test totalElapsedTime() for workouts longer than 60 seconds (where the bug occurs)
     */
    void test_totalElapsedTime_longWorkout();

    /**
     * @brief Test totalElapsedTime() for a 40-minute workout (2400 seconds)
     * This is the specific case reported by the user
     */
    void test_totalElapsedTime_40MinuteWorkout();

    /**
     * @brief Test totalElapsedTime() for multi-hour workouts
     */
    void test_totalElapsedTime_multiHourWorkout();

    /**
     * @brief Regression test: Proves the old buggy code QTime(0, 0, ticks) was broken
     * This test PASSES when it confirms the bug existed with the old approach
     */
    void test_oldBuggyCode_proveBugExisted();
};

TEST_F(TrainProgramTestSuite, TestTotalElapsedTime_ShortWorkout) {
    this->test_totalElapsedTime_shortWorkout();
}

TEST_F(TrainProgramTestSuite, TestTotalElapsedTime_Exactly60Seconds) {
    this->test_totalElapsedTime_exactly60Seconds();
}

TEST_F(TrainProgramTestSuite, TestTotalElapsedTime_LongWorkout) {
    this->test_totalElapsedTime_longWorkout();
}

TEST_F(TrainProgramTestSuite, TestTotalElapsedTime_40MinuteWorkout) {
    this->test_totalElapsedTime_40MinuteWorkout();
}

TEST_F(TrainProgramTestSuite, TestTotalElapsedTime_MultiHourWorkout) {
    this->test_totalElapsedTime_multiHourWorkout();
}

TEST_F(TrainProgramTestSuite, TestOldBuggyCode_ProveBugExisted) {
    this->test_oldBuggyCode_proveBugExisted();
}

#endif // TRAINPROGRAMTESTSUITE_H
