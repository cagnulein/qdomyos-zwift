#include "trixterxdreamv1biketestsuite.h"



void TrixterXDreamV1BikeTestSuite::test_power_calculations() {
    std::shared_ptr<trixterxdreamv1bike> bike(new trixterxdreamv1bike(false, false, false));

    uint16_t testPower = 70;
    uint8_t testCadence = 30;
    resistance_t testResistance = 230;

    bike->cadenceSensor(testCadence);
    resistance_t r = bike->resistanceFromPowerRequest(testPower);

    ASSERT_EQ(r, testResistance);

    uint16_t power = bike->powerFromResistanceRequest(r);

    ASSERT_EQ(power, testPower);

}



