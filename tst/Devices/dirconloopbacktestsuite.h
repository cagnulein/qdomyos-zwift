#ifndef DIRCONLOOPBACKTESTSUITE_H
#define DIRCONLOOPBACKTESTSUITE_H

#include "gtest/gtest.h"

class DirconLoopbackTestSuite : public testing::Test {
public:
    void test_fakebike_dircon_server_accepts_wahoodirconbike_client();
};

TEST_F(DirconLoopbackTestSuite, FakeBikeDirconServerAcceptsWahooDirconBikeClient) {
    this->test_fakebike_dircon_server_accepts_wahoodirconbike_client();
}

#endif // DIRCONLOOPBACKTESTSUITE_H
