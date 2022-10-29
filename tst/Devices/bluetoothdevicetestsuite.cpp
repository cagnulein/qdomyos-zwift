#include <memory>
#include "bluetooth.h"
#include "bluetoothdevicetestsuite.h"
#include "ActivioTreadmill/activiotreadmilltestdata.h"
#include "BHFitnessElliptical/bhfitnessellipticaltestdata.h"


TEST_F(BluetoothDeviceTestSuite, ActivioTreadmillDetected) {
    ActivioTreadmillTestData testData;
    this->test_deviceDetection(testData);
}


TEST_F(BluetoothDeviceTestSuite, BHFitnessEllipticalDetected) {
    BHFitnessEllipticalTestData testData;
    this->test_deviceDetection(testData);
}

void BluetoothDeviceTestSuite::test_deviceDetection(const BluetoothDeviceTestData& testData)
{
    const QString testUUID = QStringLiteral("b8f79bac-32e5-11ed-a261-0242ac120002");
    QBluetoothUuid uuid{testUUID};

    QStringList names = testData.get_deviceNames();

    EXPECT_GT(names.length(), 0);

    bluetooth bt(false, "", false, false, 200, false, false, 4,1.0, false);
    devicediscoveryinfo discoveryInfo = bluetooth::getDiscoveryInfo();
    for(QString deviceName : names)
    {

        QBluetoothDeviceInfo deviceInfo{uuid, deviceName, 0};

        auto discovered = bt.discoverDevice(discoveryInfo, deviceInfo);

        EXPECT_EQ(discovered.type, testData.get_expectedDeviceType());
        //EXPECT_TRUE(testData.get_isExpectedDevice(detectedDevice));
    }


}



