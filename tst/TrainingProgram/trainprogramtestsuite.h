#ifndef TRAINPROGRAMTESTSUITE_H
#define TRAINPROGRAMTESTSUITE_H

#include "gtest/gtest.h"

class TrainProgramTestSuite : public testing::Test {
public:
    void test_lapButtonBarrierBlocksSkippedZeroDurationRow();
    void test_lapButtonBarrierFindsRowBeforeWorkoutEnd();
    void test_lapButtonBarrierIgnoresRowsOutsideCandidateRange();
    void test_heartRateThresholdBarrierBlocksSkippedZeroDurationRow();
    void test_gpxInclinationFilterIsDisabledAtZero();
    void test_gpxInclinationFilterSuppressesSmallChanges();
    void test_gpxInclinationFilterAllowsThresholdChanges();
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

TEST_F(TrainProgramTestSuite, GpxInclinationFilterIsDisabledAtZero) {
    this->test_gpxInclinationFilterIsDisabledAtZero();
}

TEST_F(TrainProgramTestSuite, GpxInclinationFilterSuppressesSmallChanges) {
    this->test_gpxInclinationFilterSuppressesSmallChanges();
}

TEST_F(TrainProgramTestSuite, GpxInclinationFilterAllowsThresholdChanges) {
    this->test_gpxInclinationFilterAllowsThresholdChanges();
}

#endif // TRAINPROGRAMTESTSUITE_H
