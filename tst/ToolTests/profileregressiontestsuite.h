#ifndef PROFILEREGRESSIONTESTSUITE_H
#define PROFILEREGRESSIONTESTSUITE_H

#include "gtest/gtest.h"

class ProfileRegressionTestSuite : public testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
};

#endif // PROFILEREGRESSIONTESTSUITE_H
