#include "virtualtreadmill.h"

volatile double currentSpeed = 0;
volatile double currentIncline = 0;
volatile double currentHeart = 0;
volatile double requestSpeed = -1;
volatile double requestIncline = -1;

virtualtreadmill::virtualtreadmill()
{
    //! [Advertising Data]    
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("DomyosBridge");
    advertisingData.setServices(QList<QBluetoothUuid>() << (QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    //! [Advertising Data]

    //! [Service Data]
    QLowEnergyCharacteristicData charData;
    charData.setUuid((QBluetoothUuid::CharacteristicType)0x2ACC); //FitnessMachineFeatureCharacteristicUuid
    QByteArray value;
    value.append(0x08);
    value.append((char)0x14); // heart rate and elapsed time
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    charData.setValue(value);
    charData.setProperties(QLowEnergyCharacteristic::Read);
/*    const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                QByteArray(2, 0));
    charData.addDescriptor(clientConfig);*/

    QLowEnergyCharacteristicData charData2;
    charData2.setUuid((QBluetoothUuid::CharacteristicType)0x2ACD); //TreadmillDataCharacteristicUuid
    charData2.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);
    const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                descriptor);
    charData2.addDescriptor(clientConfig2);

    QLowEnergyCharacteristicData charData3;
    charData3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); //Fitness Machine Control Point
    charData3.setProperties(QLowEnergyCharacteristic::Write);

    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceData.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    serviceData.addCharacteristic(charData);
    serviceData.addCharacteristic(charData2);
    serviceData.addCharacteristic(charData3);
    //! [Service Data]

    //! [Start Advertising]
    leController = QLowEnergyController::createPeripheral();
    service = leController->addService(serviceData);

    QObject::connect(service, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));

    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                   advertisingData);
    //! [Start Advertising]

    //! [Provide Heartbeat]    
    QObject::connect(&treadmillTimer, SIGNAL(timeout()), this, SLOT(treadmillProvider()));
    treadmillTimer.start(1000);
    //! [Provide Heartbeat]
    QObject::connect(leController, SIGNAL(disconnected()), this, SLOT(reconnect()));
}

void virtualtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "characteristicChanged" << characteristic.uuid().toUInt16() << newValue;

    char a;
    char b;

    switch(characteristic.uuid().toUInt16())
    {
       case 0x2AD9: // Fitness Machine Control Point
         if((char)newValue.at(0)==0x02)
         {
            // Set Target Speed
            a = newValue.at(1);
            b = newValue.at(2);

            uint16_t uspeed = a + (((uint16_t)b) << 8);
            requestSpeed = uspeed / 100;
            qDebug() << "new requested speed" << requestSpeed;
         }
         else if ((char)newValue.at(0)== 0x03) // Set Target Inclination
         {
              a = newValue.at(1);
              b = newValue.at(2);

              int16_t sincline = a + (((int16_t)b) << 8);
              requestIncline = sincline / 10;
              if(requestIncline < 0)
                 requestIncline = 0;
              qDebug() << "new requested incline" << requestIncline;
         }
         break;
    }
}

void virtualtreadmill::reconnect()
{
    service = leController->addService(serviceData);
    if (service)
        leController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                       advertisingData, advertisingData);
}

void virtualtreadmill::treadmillProvider()
{
    QByteArray value;
    value.append(0x08); // Inclination avaiable
    value.append(0x01); // Heart rate avaiable

    uint32_t normalizeSpeed = (uint32_t)qRound(currentSpeed * 100);
    char a = (normalizeSpeed >> 24) & 0XFF;
    char b = (normalizeSpeed >> 16) & 0XFF;
    char c = (normalizeSpeed >> 8) & 0XFF;
    char d = normalizeSpeed & 0XFF;
    QByteArray speedBytes;
    speedBytes.append(d);
    speedBytes.append(c);
    speedBytes.append(b);
    speedBytes.append(a);
    uint16_t normalizeIncline = (uint32_t)qRound(currentIncline * 100);
    a = (normalizeIncline >> 8) & 0XFF;
    b = normalizeIncline & 0XFF;
    QByteArray inclineBytes;
    inclineBytes.append(b);
    inclineBytes.append(a);

    value.append(speedBytes); // Actual value.

    value.append(inclineBytes); //incline

    value.append(char(0xFF));  //ramp angle (auto calculated)
    value.append(char(0x7F));

    value.append(char(currentHeart)); // heart current

    QLowEnergyCharacteristic characteristic
            = service->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD); //TreadmillDataCharacteristicUuid
    Q_ASSERT(characteristic.isValid());
    service->writeCharacteristic(characteristic, value); // Potentially causes notification.

    //characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    //Q_ASSERT(characteristic.isValid());
    //service->readCharacteristic(characteristic);
}
