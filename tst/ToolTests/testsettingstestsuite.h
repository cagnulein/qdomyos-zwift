#ifndef TESTSETTINGSTESTSUITE_H
#define TESTSETTINGSTESTSUITE_H

#include "gtest/gtest.h"

class TestSettingsTestSuite: public testing::Test {

public:
    TestSettingsTestSuite();

    // Sets up the test fixture.
    //void SetUp() override;

    // Tears down the test fixture.
    // void TearDown() override;

    /**
     * @brief Test the overall functionality of a TestSetting object
     */
    void test_testSettings();

    /**
     * @brief Test that the destructor restores the original state of the QCoreApplication
     */
    void test_destructor();
};

TEST_F(TestSettingsTestSuite, TestTestSettings) {
    this->test_testSettings();
}

TEST_F(TestSettingsTestSuite, TestDestructor) {
    this->test_destructor();
}

#endif // TESTSETTINGSTESTSUITE_H
