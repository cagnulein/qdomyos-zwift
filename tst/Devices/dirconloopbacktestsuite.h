#ifndef DIRCONLOOPBACKTESTSUITE_H
#define DIRCONLOOPBACKTESTSUITE_H

#include "gtest/gtest.h"

class DirconLoopbackTestSuite : public testing::Test {
public:
    void test_fakebike_dircon_server_accepts_wahoodirconbike_client();
    void test_faketreadmill_dircon_server_accepts_wahoodircontreadmill_client();
    void test_two_fakebike_dircon_servers_accept_two_wahoodirconbike_clients();
};

TEST_F(DirconLoopbackTestSuite, FakeBikeDirconServerAcceptsWahooDirconBikeClient) {
    this->test_fakebike_dircon_server_accepts_wahoodirconbike_client();
}

TEST_F(DirconLoopbackTestSuite, FakeTreadmillDirconServerAcceptsWahooDirconTreadmillClient) {
    this->test_faketreadmill_dircon_server_accepts_wahoodircontreadmill_client();
}

TEST_F(DirconLoopbackTestSuite, TwoFakeBikeDirconServersAcceptTwoWahooDirconBikeClients) {
    this->test_two_fakebike_dircon_servers_accept_two_wahoodirconbike_clients();
}

#endif // DIRCONLOOPBACKTESTSUITE_H
