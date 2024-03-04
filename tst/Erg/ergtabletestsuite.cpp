#include "ergtabletestsuite.h"

#include <QCoreApplication>

#include "Tools/testsettings.h"


ErgTableTestSuite::ErgTableTestSuite()
{

}



void ErgTableTestSuite::test_wattageEstimation(const QList<ergDataPoint> &inputs, const QList<ergDataPoint>& expectedOutputs) {
    ergTable erg;

    // set up the erg table
    for(const ergDataPoint& dataPoint : inputs)
        erg.collectData(dataPoint.cadence, dataPoint.wattage, dataPoint.resistance);

    // tst the results
    for(const ergDataPoint& dataPoint : expectedOutputs)
    {
        auto wattage = erg.estimateWattage(dataPoint.cadence, dataPoint.resistance);
        EXPECT_DOUBLE_EQ(dataPoint.wattage, wattage) << "Expected estimated wattage from C:" << dataPoint.cadence << " R:"<<dataPoint.resistance << " to be " << dataPoint.wattage;
    }
}

void ErgTableTestSuite::test_dynamicErgTable() {

    QList<ergDataPoint> inputs, expected;

    inputs.append(ergDataPoint(60, 100, 50));
    inputs.append(ergDataPoint(50, 100, 60));
    inputs.append(ergDataPoint(40, 100, 70));
    inputs.append(ergDataPoint(1, 100, 500));
    inputs.append(ergDataPoint(60, 200, 100));
    inputs.append(ergDataPoint(50, 200, 120));
    inputs.append(ergDataPoint(40, 200, 140));
    inputs.append(ergDataPoint(1, 200, 1000));

    // test that the input points are mapped directly
    expected.append(inputs);

    this->test_wattageEstimation(inputs, expected);

    // Now try some interpolated points
    expected.clear();

    expected.append(ergDataPoint(65,100, 75)); // closer to (60,100,50) or (60,200,100)?
    expected.append(ergDataPoint(51,100, 59));
    this->test_wattageEstimation(inputs, expected);

}
