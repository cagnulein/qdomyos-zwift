#include "trixterxdreamv1biketestsuite.h"
#include "testserialdatasource.h"


TrixterXDreamV1BikeTestSuite::TrixterXDreamV1BikeTestSuite() : testSettings("Roberto Viola", "QDomyos-Zwift Testing") {
    // use the test serial data source because the bike won't be there usually, during test runs.
    trixterxdreamv1serial::serialDataSourceFactory = TestSerialDatasource::create;
}

void TrixterXDreamV1BikeTestSuite::test_power_calculations() {
    std::shared_ptr<trixterxdreamv1bike> bike(new trixterxdreamv1bike(false, false, false));

    const uint32_t maxRPM = 120;
    const uint32_t minRPM = 30;
    const resistance_t maxResistance = bike->maxResistance();
    const resistance_t minResistance = 0;

    uint16_t p0, p1;

    // traverse the cadence edges checking the power is clipped to the values for the max and min resistance
    for(uint32_t cadenceRPM=minRPM; cadenceRPM<=maxRPM; cadenceRPM++)
    {
        bike->cadenceSensor(cadenceRPM);
        p0 = bike->powerFromResistanceRequest(maxResistance);
        p1 = bike->powerFromResistanceRequest(maxResistance+1);

        ASSERT_EQ(p0, p1) << "expected power to stop increasing at max resistance, at cadence " << cadenceRPM << " RPM";

        p0 = bike->powerFromResistanceRequest(minResistance);
        p1 = bike->powerFromResistanceRequest(minResistance-1);

        ASSERT_EQ(p0, p1) << "expected power to stop decreasing at min resistance, at cadence " << cadenceRPM << " RPM";
    }

    // traverse the resistance edge checking the power is clipped to the values for the max and min cadence
    for(resistance_t r=minResistance; r<=maxResistance; r++)
    {
        bike->cadenceSensor(minRPM);
        p0 = bike->powerFromResistanceRequest(r);
        bike->cadenceSensor(minRPM-1);
        p1 = bike->powerFromResistanceRequest(r);

        ASSERT_EQ(p0, p1) << "expected power to stop decreasing at min cadence, for resistance " << r ;

        bike->cadenceSensor(maxRPM);
        p0 = bike->powerFromResistanceRequest(r);
        bike->cadenceSensor(maxRPM+1);
        p1 = bike->powerFromResistanceRequest(r);

        ASSERT_EQ(p0, p1) << "expected power to stop increasing at max cadence, for resistance " << r ;
    }

    // test inverses
    for(uint32_t cadenceRPM=minRPM; cadenceRPM<=maxRPM; cadenceRPM++)
    {
        uint16_t lastPower=0xFFFF;
        for(resistance_t r=minResistance; r<=maxResistance; r++)
        {
            bike->cadenceSensor(cadenceRPM);
            uint16_t power = bike->powerFromResistanceRequest(r);

            if(power!=lastPower)
            {
                lastPower = power;
                resistance_t resistance = bike->resistanceFromPowerRequest(power);

                ASSERT_EQ(r, resistance) << "unexpected resistance to achieve " << power << "W at "<<cadenceRPM << "RPM";
            }

        }
    }
}

void TrixterXDreamV1BikeTestSuite::test_detection() {
    DeviceDiscoveryInfo ddi;

    ddi.trixter_xdream_v1_bike = true;
    this->testSettings.loadFrom(ddi);

    auto bike = trixterxdreamv1bike::tryCreate();
    auto bikePtr = std::unique_ptr<trixterxdreamv1bike>(bike);

    ASSERT_TRUE(bike!=nullptr) << "Bike should have been detected from fake serial data";

    this->testSettings.activate();
}



