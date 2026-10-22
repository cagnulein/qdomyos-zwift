#ifndef TESTTRAININGLOADTESTSUITE_H
#define TESTTRAININGLOADTESTSUITE_H

#include "gtest/gtest.h"

class TestTrainingLoadTestSuite: public testing::Test {

public:
    TestTrainingLoadTestSuite();

    /**
     * @brief Test TRIMP calculation with real Fenix 8 data
     * Tests that our TRIMP calculation matches Garmin Fenix 8 output
     * Using real data: 48:44 run, avg HR 167, max HR 177, resting HR 60
     * Expected result: ~207.7
     */
    void test_trimpCalculationFenix8();

    /**
     * @brief Test TRIMP calculation with various scenarios
     */
    void test_trimpCalculationVariousScenarios();

    /**
     * @brief Test TSS calculation for cycling with power
     */
    void test_tssCalculation();

private:
    /**
     * @brief Calculate TRIMP using Bannister's formula
     * This replicates the logic from qfit.cpp
     */
    float calculateTRIMP(uint32_t duration_minutes, double avg_hr, uint8_t max_hr,
                        uint8_t resting_hr, bool is_male);

    /**
     * @brief Calculate TSS for cycling with power
     * This replicates the logic from qfit.cpp
     */
    float calculateTSS(uint32_t duration_seconds, double avg_power, float ftp);
};

TEST_F(TestTrainingLoadTestSuite, TestTRIMPCalculationFenix8) {
    this->test_trimpCalculationFenix8();
}

TEST_F(TestTrainingLoadTestSuite, TestTRIMPCalculationVariousScenarios) {
    this->test_trimpCalculationVariousScenarios();
}

TEST_F(TestTrainingLoadTestSuite, TestTSSCalculation) {
    this->test_tssCalculation();
}

#endif // TESTTRAININGLOADTESTSUITE_H
