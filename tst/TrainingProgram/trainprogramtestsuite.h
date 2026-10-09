#ifndef TRAINPROGRAMTESTSUITE_H
#define TRAINPROGRAMTESTSUITE_H

#include "gtest/gtest.h"

class TrainProgramTestSuite : public testing::Test {
public:
    void test_lapButtonBarrierBlocksSkippedZeroDurationRow();
    void test_lapButtonBarrierFindsRowBeforeWorkoutEnd();
    void test_lapButtonBarrierIgnoresRowsOutsideCandidateRange();
    void test_heartRateThresholdBarrierBlocksSkippedZeroDurationRow();
    void test_heartRatePidSuppressionRequiresStartedTrainingProgram();
    void test_heartRatePidSuppressionCoversForcedSpeedRows();
    void test_heartRatePidSuppressionCoversRowBoundaryWindow();
    void test_heartRatePidSuppressionCoversPostTransitionWindow();
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

TEST_F(TrainProgramTestSuite, HeartRatePidSuppressionRequiresStartedTrainingProgram) {
    this->test_heartRatePidSuppressionRequiresStartedTrainingProgram();
}

TEST_F(TrainProgramTestSuite, HeartRatePidSuppressionCoversForcedSpeedRows) {
    this->test_heartRatePidSuppressionCoversForcedSpeedRows();
}

TEST_F(TrainProgramTestSuite, HeartRatePidSuppressionCoversRowBoundaryWindow) {
    this->test_heartRatePidSuppressionCoversRowBoundaryWindow();
}

TEST_F(TrainProgramTestSuite, HeartRatePidSuppressionCoversPostTransitionWindow) {
    this->test_heartRatePidSuppressionCoversPostTransitionWindow();
}

#endif // TRAINPROGRAMTESTSUITE_H
