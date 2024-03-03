#include "ergtabletestsuite.h"

#include <QCoreApplication>

#include "Tools/testsettings.h"


ErgTableTestSuite::ErgTableTestSuite()
{

}



void ErgTableTestSuite::test_wattageEstimation(const QList<DataPoint> &inputs, const QList<DataPoint>& expectedOutputs) {

    TestSettings testSettings("Roberto Viola", "QDomyos-Zwift Testing");
    testSettings.activate(); // previous settings should be restored automatically when this object goes out of scope via its destructor

    // "just in case", clear the DataPoints settings so that the ergtable is blank
    testSettings.qsettings.remove("DataPoints");

    ergTable erg;

    // set up the erg table
    for(const DataPoint& dataPoint : inputs)
        erg.collectData(dataPoint.cadence, dataPoint.wattage, dataPoint.resistance);

    // tst the results
    for(const DataPoint& dataPoint : expectedOutputs)
    {
        auto wattage = erg.estimateWattage(dataPoint.cadence, dataPoint.resistance);
        EXPECT_DOUBLE_EQ(dataPoint.wattage, wattage) << "Expected estimated wattage from C:" << dataPoint.cadence << " R:"<<dataPoint.resistance << " to be " << dataPoint.wattage;
    }
}

void ErgTableTestSuite::test_dynamicErgTable() {

    QList<DataPoint> inputs, expected;

    inputs.append(DataPoint(60, 100, 50));
    inputs.append(DataPoint(50, 100, 60));
    inputs.append(DataPoint(40, 100, 70));
    inputs.append(DataPoint(1, 100, 500));
    inputs.append(DataPoint(60, 200, 100));
    inputs.append(DataPoint(50, 200, 120));
    inputs.append(DataPoint(40, 200, 140));
    inputs.append(DataPoint(1, 200, 1000));

    // test that the input points are mapped directly
    expected.append(inputs);

    this->test_wattageEstimation(inputs, expected);

    // Now try some interpolated points
    expected.clear();

    expected.append(DataPoint(65,100, 75)); // closer to (60,100,50) or (60,200,100)?
    expected.append(DataPoint(51,100, 59));
    this->test_wattageEstimation(inputs, expected);

}
