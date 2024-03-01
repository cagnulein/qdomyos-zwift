#pragma once

#include "gtest/gtest.h"
#include "devices.h"
#include "ErgInterface/erginterface.h"

#include "Tools/testsettings.h"

template <typename T>
class BikeTestSuite : public testing::Test {
private:
    bike * device = nullptr;

protected:
    T typeParam;

    uint32_t maxRPM, minRPM;
    resistance_t maxResistance, minResistance;

    erginterface * ergInterface = nullptr;

    /**
     * @brief Manages the QSettings used during the tests, separate from QSettings stored in the system generally.
     */
    TestSettings testSettings;
public:
    BikeTestSuite();

    // Sets up the test fixture.
    void SetUp() override;

    // Tears down the test fixture.
    virtual void TearDown() override;

    /**
     * @brief Test that power doesn't change below the minimum cadence for a constant resistance.
     */
    void test_powerFunctions_minCadence();

    /**
     * @brief Test that power doesn't change below the minimum resistance for a constant cadence.
     */
    void test_powerFunctions_minResistance();

    /**
     * @brief Test that power doesn't change below the maximum resistance for a constant cadence.
     */
    void test_powerFunctions_maxResistance();

    /**
     * @brief Test that power doesn't change above the maximum cadence for a constant resistance.
     */
    void test_powerFunctions_maxCadence();

    /**
     * @brief Test that the resistance from power function returns the minimum resistance required to get the same power
     * from the power-from-resistance function at the same cadence.
     */
    void test_powerFunctions_resistancePowerConversion();

};


TYPED_TEST_SUITE(BikeTestSuite, BikeTestDataTypes);

TYPED_TEST(BikeTestSuite, TestPowerFunctionsMinCadence) {
    this->test_powerFunctions_minCadence();
}

TYPED_TEST(BikeTestSuite, TestPowerFunctionsMaxCadence) {
    this->test_powerFunctions_maxCadence();
}

TYPED_TEST(BikeTestSuite, TestPowerFunctionsMinResistance) {
    this->test_powerFunctions_minResistance();
}

TYPED_TEST(BikeTestSuite, TestPowerFunctionsMaxResistance) {
    this->test_powerFunctions_maxResistance();
}

TYPED_TEST(BikeTestSuite, TestPowerFunctionsResistancePowerConversion) {
    this->test_powerFunctions_resistancePowerConversion();
}

