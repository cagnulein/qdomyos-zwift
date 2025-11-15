#pragma once

#include "gtest/gtest.h"
#include "bluetooth.h"
#include "Tools/testsettings.h"


class TrixterXDreambikeTestSuite : public testing::Test {

protected:

    /**
     * @brief Manages the QSettings used during the tests, separate from QSettings stored in the system generally.
     */
    TestSettings testSettings;

public:
    TrixterXDreambikeTestSuite();

    // Sets up the test fixture.
    //void SetUp() override;

    // Tears down the test fixture.
    // virtual void TearDown();

    /**
     * @brief Power calculation tests.
     */
    void test_power_calculations();


    /**
     * @brief Test that the bike test stub works.
     */
    void test_stub();
};

TEST_F(TrixterXDreambikeTestSuite, TestPowerCalculations) {
    this->test_power_calculations();
}

TEST_F(TrixterXDreambikeTestSuite, TestStub) {
    this->test_stub();
}

