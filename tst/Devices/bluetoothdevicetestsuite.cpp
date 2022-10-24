#include <memory>

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

    /* This is a rough sketch, can't really do this...the bluetooth object automatically starts bluetooth discovery
     all that's needed here is testing name detection and device object creation. */

    /*
    bluetooth bt( false);

    for(QString deviceName : names)
    {
        QBluetoothDeviceInfo deviceInfo{uuid, deviceName, 0};

        bt.deviceConnected(deviceInfo);

        bluetoothdevice * detectedDevice = bt.device();

        EXPECT_NE(detectedDevice, nullptr);
        EXPECT_TRUE(testData.get_isExpectedDevice(detectedDevice));
    }

*/
}



