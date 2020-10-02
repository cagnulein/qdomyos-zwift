#include "virtualtreadmill.h"
#include <QtMath>

volatile double currentSpeed = 0;
volatile double currentIncline = 0;
volatile uint8_t currentHeart = 0;
volatile double requestSpeed = -1;
volatile double requestIncline = -1;
volatile int8_t requestStart = -1;
volatile int8_t requestStop = -1;

virtualtreadmill::virtualtreadmill()
{
    //! [Advertising Data]    
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("DomyosBridge");
    QList<QBluetoothUuid> services;
    services << ((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    services << QBluetoothUuid::HeartRate;
    advertisingData.setServices(services);
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

    QLowEnergyCharacteristicData charDataHR;
    charDataHR.setUuid(QBluetoothUuid::HeartRateMeasurement);
    charDataHR.setValue(QByteArray(2, 0));
    charDataHR.setProperties(QLowEnergyCharacteristic::Notify);
    const QLowEnergyDescriptorData clientConfigHR(QBluetoothUuid::ClientCharacteristicConfiguration,
                                            QByteArray(2, 0));
    charDataHR.addDescriptor(clientConfigHR);

    QLowEnergyServiceData serviceDataHR;
    serviceDataHR.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceDataHR.setUuid(QBluetoothUuid::HeartRate);
    serviceDataHR.addCharacteristic(charDataHR);

    //! [Start Advertising]
    leController = QLowEnergyController::createPeripheral();
    service = leController->addService(serviceData);
    serviceHR = leController->addService(serviceDataHR);

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
            requestSpeed = (double)uspeed / 100.0;
            qDebug() << "new requested speed" << requestSpeed;
         }
         else if ((char)newValue.at(0)== 0x03) // Set Target Inclination
         {
              a = newValue.at(1);
              b = newValue.at(2);

              int16_t sincline = a + (((int16_t)b) << 8);
              requestIncline = (double)sincline / 10.0;
              if(requestIncline < 0)
                 requestIncline = 0;
              qDebug() << "new requested incline" << requestIncline;
         }
         else if ((char)newValue.at(0)== 0x07) // Start request
         {
              requestStart = 1;
              qDebug() << "request to start";
         }
         else if ((char)newValue.at(0)== 0x08) // Stop request
         {
              requestStop = 1;
              qDebug() << "request to stop";
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
    value.append((char)0x00);

    uint16_t normalizeSpeed = (uint16_t)qRound(currentSpeed * 100);
    char a = (normalizeSpeed >> 8) & 0XFF;
    char b = normalizeSpeed & 0XFF;
    QByteArray speedBytes;
    speedBytes.append(b);
    speedBytes.append(a);
    uint16_t normalizeIncline = (uint32_t)qRound(currentIncline * 10);
    a = (normalizeIncline >> 8) & 0XFF;
    b = normalizeIncline & 0XFF;
    QByteArray inclineBytes;
    inclineBytes.append(b);
    inclineBytes.append(a);
    double ramp = qRadiansToDegrees(qAtan(currentIncline/100));
    int16_t normalizeRamp = (int32_t)qRound(ramp * 10);
    a = (normalizeRamp >> 8) & 0XFF;
    b = normalizeRamp & 0XFF;
    QByteArray rampBytes;
    rampBytes.append(b);
    rampBytes.append(a);

    value.append(speedBytes); // Actual value.

    value.append(inclineBytes); //incline

    value.append(rampBytes);  //ramp angle

    QLowEnergyCharacteristic characteristic
            = service->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD); //TreadmillDataCharacteristicUuid
    Q_ASSERT(characteristic.isValid());
    service->writeCharacteristic(characteristic, value); // Potentially causes notification.

    //characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    //Q_ASSERT(characteristic.isValid());
    //service->readCharacteristic(characteristic);

    QByteArray valueHR;
    valueHR.append(char(0)); // Flags that specify the format of the value.
    valueHR.append(char(currentHeart)); // Actual value.
    QLowEnergyCharacteristic characteristicHR
            = serviceHR->characteristic(QBluetoothUuid::HeartRateMeasurement);
    Q_ASSERT(characteristicHR.isValid());
    serviceHR->writeCharacteristic(characteristicHR, valueHR); // Potentially causes notification.

    QFile* log;
    QDomDocument docStatus;
    QDomElement docRoot;
    QDomElement docTreadmill;
    QDomElement docHeart;
    log = new QFile("status.xml");
    if(!log->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Open status.xml for writing failed";
    }
    docRoot = docStatus.createElement("Gym");
    docStatus.appendChild(docRoot);
    docTreadmill = docStatus.createElement("Treadmill");
    docTreadmill.setAttribute("Speed", QString::number(currentSpeed));
    docTreadmill.setAttribute("Incline", QString::number(currentIncline));
    docRoot.appendChild(docTreadmill);
    docHeart = docStatus.createElement("Heart");
    docHeart.setAttribute("Rate", QString::number(currentHeart));
    docRoot.appendChild(docHeart);
    docRoot.setAttribute("Updated", QDateTime::currentDateTime().toString());
    QTextStream stream(log);
    stream << docStatus.toString();
    log->close();
}

uint16_t virtualtreadmill::watts()
{
    // calc Watts ref. https://alancouzens.com/blog/Run_Power.html

    uint16_t watts=0;
    if(currentSpeed > 0)
    {
       double weight=75.0; // TODO: config need
       double pace=60/currentSpeed;
       double VO2R=210.0/pace;
       double VO2A=(VO2R*weight)/1000.0;
       double hwatts=75*VO2A;
       double vwatts=((9.8*weight) * (currentIncline/100));
       watts=hwatts+vwatts;
    }
    return watts;
}
