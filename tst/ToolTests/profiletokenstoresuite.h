#ifndef PROFILETOKENSTORESUITE_H
#define PROFILETOKENSTORESUITE_H

#include "gtest/gtest.h"

class ProfileTokenStoreTestSuite : public testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
};

#endif // PROFILETOKENSTORESUITE_H
