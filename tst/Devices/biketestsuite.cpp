#include "biketestsuite.h"
#include "Erg/bikeergfunctions.h"

template<typename T>
QList<resistance_t> BikeTestSuite<T>::getResistanceSamples() {
    return this->getSamples(this->minResistance, this->maxResistance);
}

template<typename T>
QList<uint32_t> BikeTestSuite<T>::getCadenceSamples() {
    return this->getSamples(this->minRPM, this->maxRPM);
}

template<typename T>
BikeTestSuite<T>::BikeTestSuite() : testSettings("Roberto Viola", "QDomyos-Zwift Testing") {}

template<typename T>
void BikeTestSuite<T>::SetUp() {
    BikeOptions options;

    // activate the test settings before doing anything
    this->testSettings.activate();

    this->device =this->typeParam.createInstance(options);
    this->ergInterface = new bikeergfunctions(this->device);

    this->maxRPM = this->ergInterface->getMaxCadence().value_or(120);
    this->minRPM = this->ergInterface->getMinCadence().value_or(1);
    this->maxResistance = this->ergInterface->getMaxResistance().value_or(255);
    this->minResistance = this->ergInterface->getMinResistance().value_or(0);
}

template<typename T>
void BikeTestSuite<T>::TearDown() {
    if(this->device) delete this->device;
    if(this->ergInterface) delete this->ergInterface;
    this->device = nullptr;
    this->ergInterface = nullptr;
}

template<typename T>
void BikeTestSuite<T>::test_powerFunctions_minResistance() {
    const auto erg = this->ergInterface;
    uint16_t p0, p1;
    resistance_t r0, r1;
    QStringList errors;
    QString  powerBeyondResistanceLimit = QStringLiteral("Power at C:%1 RPM not bounded at %6 resistance (R:%2, P:%3W), (R:%4, P:%5W)");

    if(!erg->getMinResistance().has_value())
    {
        GTEST_SKIP() << "No minimum resistance defined";
    }

    // traverse the cadence edges checking the power is clipped to the values for the max and min resistance
    for( uint32_t cadenceRPM : this->getCadenceSamples())
    {
        r0 = minResistance;
        r1 = minResistance-1;
        p0 = erg->getPower(cadenceRPM, r0);
        p1 = erg->getPower(cadenceRPM, r1);

        if(p0 != p1)
            errors.append(powerBeyondResistanceLimit.arg(cadenceRPM).arg(r0).arg(p0).arg(r1).arg(p1).arg("minimum"));
    }

    ASSERT_TRUE(errors.empty()) << errors.join('\n').toStdString();
}


template<typename T>
void BikeTestSuite<T>::test_powerFunctions_maxResistance() {
    const auto erg = this->ergInterface;
    uint16_t p0, p1;
    resistance_t r0, r1;
    QStringList errors;
    QString  powerBeyondResistanceLimit = QStringLiteral("Power at C:%1 RPM not bounded at %6 resistance (R:%2, P:%3W), (R:%4, P:%5W)");

    if(!erg->getMaxResistance().has_value())
    {
        GTEST_SKIP() << "No maximum resistance defined.";
    }

    // traverse the cadence edges checking the power is clipped to the values for the max and min resistance
    for(uint16_t cadenceRPM : this->getCadenceSamples())
    {
        r0 = maxResistance;
        r1 = maxResistance+1;
        p0 = erg->getPower(cadenceRPM, r0);
        p1 = erg->getPower(cadenceRPM, r1);

        if(p0 != p1)
            errors.append(powerBeyondResistanceLimit.arg(cadenceRPM).arg(r0).arg(p0).arg(r1).arg(p1).arg("maximum"));
    }

    ASSERT_TRUE(errors.empty()) << errors.join('\n').toStdString();
}

template<typename T>
void BikeTestSuite<T>::test_powerFunctions_minCadence() {
    const auto erg = this->ergInterface;
    uint16_t p0, p1;
    QStringList errors;

    if(!erg->getMinCadence().has_value())
    {
        GTEST_SKIP() << "No minimum cadence defined.";
    }

    // traverse the resistance edge checking the power is clipped to the values for the max and min cadence

    QString  powerBeyondCadenceLimit = QStringLiteral("Power at R:%1 not bounded at %6 cadence (C:%2 RPM, P:%3W), (C:%4 RPM, P:%5W)");

    for( resistance_t r : this->getResistanceSamples())
    {
        const int32_t c0 = minRPM, c1=minRPM-1;
        p0 = erg->getPower(c0, r);
        p1 = erg->getPower(c1, r);

        if(p0!=p1)
            errors.append(powerBeyondCadenceLimit.arg(r).arg(c0).arg(p0).arg(c1).arg(p1).arg("minimum"));
    }

    ASSERT_TRUE(errors.empty()) << errors.join('\n').toStdString();
}

template<typename T>
void BikeTestSuite<T>::test_powerFunctions_maxCadence() {
    const auto erg = this->ergInterface;
    uint16_t p0, p1;
    QStringList errors;

    if(!erg->getMaxCadence().has_value())
    {
        GTEST_SKIP() << "No maximum cadence defined";
    }

    // traverse the resistance edge checking the power is clipped to the values for the max and min cadence

    QString  powerBeyondCadenceLimit = QStringLiteral("Power at R:%1 not bounded at %6 cadence (C:%2 RPM, P:%3W), (C:%4 RPM, P:%5W)");

    for( resistance_t r : this->getResistanceSamples())
    {
        const int32_t c0 = maxRPM, c1=maxRPM+1;
        p0 = erg->getPower(c0, r);
        p1 = erg->getPower(c1, r);

        if(p0!=p1)
            errors.append(powerBeyondCadenceLimit.arg(r).arg(c0).arg(p0).arg(c1).arg(p1).arg("maximum"));
    }

    ASSERT_TRUE(errors.empty()) << errors.join('\n').toStdString();
}

template<typename T>
void BikeTestSuite<T>::test_powerFunctions_resistancePowerConversion() {
    const auto erg = this->ergInterface;
    QStringList errors;

    ASSERT_TRUE(this->testSettings.get_active()) << "TestSettings object should be active.";
    this->testSettings.qsettings.setValue(QZSettings::watt_gain, 1.0);
    this->testSettings.qsettings.setValue(QZSettings::watt_offset, 0.0);

    // test inverses
    QString unexpectedResistance=QStringLiteral("P(C:%3, R:%1)=%2 but R(C:%3, P:%2)=%4 and P(C:%3, R:%4)=%5");
    //for(uint32_t cadenceRPM=minRPM; cadenceRPM<=maxRPM; cadenceRPM++)
    for(uint16_t cadenceRPM : this->getCadenceSamples())
    {
        uint16_t lastPower=0xFFFF;
        for( resistance_t resistanceToPower : this->getResistanceSamples())
        //for(resistance_t resistanceToPower=minResistance; resistanceToPower<=maxResistance; resistanceToPower++)
        {
            uint16_t power = erg->getPower(cadenceRPM, resistanceToPower);

            // if this resistance reaches a new power level, check the inverse
            if(power!=lastPower)
            {
                lastPower = power;
                resistance_t resistanceFromPower = erg->getResistance(cadenceRPM, power);

                if(resistanceToPower!=resistanceFromPower) {
                    uint16_t newPower = erg->getPower(cadenceRPM, resistanceFromPower);

                    errors.append(unexpectedResistance.arg(resistanceToPower).arg(power).arg(cadenceRPM).arg(resistanceFromPower).arg(newPower));
                }

            }

        }
    }

    ASSERT_TRUE(errors.empty()) << errors.join('\n').toStdString();
}

template<typename T>
template<typename T0>
QList<T0> BikeTestSuite<T>::getSamples(const T0 min, const T0 max) {
    QList<T0> result;
    T0 d = max-min;
    T0 inc = d/10;

    if(inc<1) inc = 1;

    for(T0 v=min; v<=max; v+=inc)
        result.append(v);
    if(result.last()!=max)
        result.append(max);

    return result;
}
