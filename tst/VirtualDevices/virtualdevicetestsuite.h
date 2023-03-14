#pragma once

#include "gtest/gtest.h"
#include "Tools/testlockscreen.h"
#include "Tools/testsettings.h"

#include "Bike/virtualbiketestdata.h"
#include "Treadmill/virtualtreadmilltestdata.h"
#include "Rower/virtualrowertestdata.h"

template <typename T>
class VirtualDeviceTestSuite : public testing::Test {
protected:
    T typeParam;

    /**
     * @brief Manages the QSettings used during the tests, separate from QSettings stored in the system generally.
     */
    TestSettings testSettings;

    /**
     * @brief The mock lockscreen object.
     */
    TestLockscreen * testLockscreen = nullptr;

public:
    VirtualDeviceTestSuite();

    // Sets up the test fixture.
    void SetUp() override;

    // Tears down the test fixture.
    void TearDown() override;


    /**
     * @brief Test that the Peloton workaround is activated if the settings are as required by the device,
     * or not if not.
     */
    void test_lockscreenConfiguration();


};

using VirtualDeviceTestDataTypes = ::testing::Types<VirtualBikeTestData, VirtualRowerTestData, VirtualTreadmillTestData>;


TYPED_TEST_SUITE(VirtualDeviceTestSuite, VirtualDeviceTestDataTypes);


TYPED_TEST(VirtualDeviceTestSuite, TestLockscreenConfiguration) {
    this->test_lockscreenConfiguration();
}
