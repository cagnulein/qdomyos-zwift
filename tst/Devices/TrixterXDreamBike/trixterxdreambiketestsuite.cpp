#include "trixterxdreambiketestsuite.h"
#include "trixterxdreambikestub.h"


TrixterXDreambikeTestSuite::TrixterXDreambikeTestSuite() : testSettings("Roberto Viola", "QDomyos-Zwift Testing") {
    // use the test serial data source because the bike won't be there usually, during test runs.
    trixterxdreamserial::serialDataSourceFactory = [](QObject*) { return new TrixterXDreamBikeStub(); };
}

void TrixterXDreambikeTestSuite::test_power_calculations() {
    std::shared_ptr<trixterxdreambike> bike(new trixterxdreambike(false, false, false));

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


void TrixterXDreambikeTestSuite::test_stub() {

    TrixterXDreamBikeStub serial;

    EXPECT_TRUE(serial.open("stub")) << "failed to open";

    QThread::msleep(1);

    bool ready = serial.waitForReadyRead();
    EXPECT_TRUE(ready) << "should have been ready to read";

    auto data = serial.readAll();

    EXPECT_LE(32, data.size()) << "readAll() should have returned 32+ chars";
}



