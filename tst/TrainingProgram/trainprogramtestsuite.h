#ifndef TRAINPROGRAMTESTSUITE_H
#define TRAINPROGRAMTESTSUITE_H

#include "gtest/gtest.h"

class TrainProgramTestSuite : public testing::Test {
public:
    void test_lapButtonBarrierBlocksSkippedZeroDurationRow();
    void test_lapButtonBarrierFindsRowBeforeWorkoutEnd();
    void test_lapButtonBarrierIgnoresRowsOutsideCandidateRange();
    void test_heartRateThresholdBarrierBlocksSkippedZeroDurationRow();
};

TEST_F(TrainProgramTestSuite, LapButtonBarrierBlocksSkippedZeroDurationRow) {
    this->test_lapButtonBarrierBlocksSkippedZeroDurationRow();
}

TEST_F(TrainProgramTestSuite, LapButtonBarrierFindsRowBeforeWorkoutEnd) {
    this->test_lapButtonBarrierFindsRowBeforeWorkoutEnd();
}

TEST_F(TrainProgramTestSuite, LapButtonBarrierIgnoresRowsOutsideCandidateRange) {
    this->test_lapButtonBarrierIgnoresRowsOutsideCandidateRange();
}

TEST_F(TrainProgramTestSuite, HeartRateThresholdBarrierBlocksSkippedZeroDurationRow) {
    this->test_heartRateThresholdBarrierBlocksSkippedZeroDurationRow();
}

#endif // TRAINPROGRAMTESTSUITE_H
