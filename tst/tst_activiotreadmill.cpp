
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;


class ActivioTreadmill : public ::testing::Test {
protected:


    ActivioTreadmill() {
        // You can do set-up work for each test here.
    }

    ~ActivioTreadmill() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Class members declared here can be used by all tests in the test suite




};

TEST_F(ActivioTreadmill, testCase1)
{
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}

