#include "virtualtreadmill.h"
#include <QtMath>
#include <QSettings>

virtualtreadmill::virtualtreadmill(treadmill* t, bool noHeartService)
{
    QSettings settings;
    treadMill = t;
    this->noHeartService = noHeartService;

    //! [Advertising Data]    
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("DomyosBridge");
    QList<QBluetoothUuid> services;
    services << ((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    services << QBluetoothUuid::HeartRate;

#ifdef Q_OS_LINUX
    services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);
#endif

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

    if(noHeartService == false)
    {
        QLowEnergyCharacteristicData charDataHR;
        charDataHR.setUuid(QBluetoothUuid::HeartRateMeasurement);
        charDataHR.setValue(QByteArray(2, 0));
        charDataHR.setProperties(QLowEnergyCharacteristic::Notify);
        const QLowEnergyDescriptorData clientConfigHR(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                      QByteArray(2, 0));
        charDataHR.addDescriptor(clientConfigHR);

        serviceDataHR.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceDataHR.setUuid(QBluetoothUuid::HeartRate);
        serviceDataHR.addCharacteristic(charDataHR);
    }

    //! [Start Advertising]
    leController = QLowEnergyController::createPeripheral();
    Q_ASSERT(leController);
    service = leController->addService(serviceData);
    if(noHeartService == false)
        serviceHR = leController->addService(serviceDataHR);

    QObject::connect(service, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));

    bool bluetooth_relaxed = settings.value("bluetooth_relaxed", false).toBool();
    QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
    if(!bluetooth_relaxed)
        pars.setInterval(100, 100);

    leController->startAdvertising(pars, advertisingData,
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
    emit debug("characteristicChanged " + QString::number(characteristic.uuid().toUInt16()) + " " + newValue);

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
            double requestSpeed = (double)uspeed / 100.0;
            treadMill->changeSpeed(requestSpeed);
            emit debug("new requested speed " + QString::number(requestSpeed));
         }
         else if ((char)newValue.at(0)== 0x03) // Set Target Inclination
         {
              a = newValue.at(1);
              b = newValue.at(2);

              int16_t sincline = a + (((int16_t)b) << 8);
              double requestIncline = (double)sincline / 10.0;
              if(requestIncline < 0)
                 requestIncline = 0;
              treadMill->changeInclination(requestIncline);
              emit debug("new requested incline " +QString::number(requestIncline));
         }
         else if ((char)newValue.at(0)== 0x07) // Start request
         {
              treadMill->start();
              emit debug("request to start");
         }
         else if ((char)newValue.at(0)== 0x08) // Stop request
         {
              treadMill->stop();
              emit debug("request to stop");
         }
         break;
    }
}

void virtualtreadmill::reconnect()
{
    QSettings settings;
    bool bluetooth_relaxed = settings.value("bluetooth_relaxed", false).toBool();

    if(bluetooth_relaxed) return;

    emit debug("virtualtreadmill reconnect");
    service = leController->addService(serviceData);

    if(noHeartService == false)
        serviceHR = leController->addService(serviceDataHR);

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);

    if (service)
        leController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                       advertisingData, advertisingData);
}

void virtualtreadmill::treadmillProvider()
{
    if(leController->state() != QLowEnergyController::ConnectedState)
    {
        emit debug("virtualtreadmill connection error");
        return;
    }
    else
    {
        QSettings settings;
        bool bluetooth_relaxed = settings.value("bluetooth_relaxed", false).toBool();
        if(bluetooth_relaxed)
            leController->stopAdvertising();
    }

    QByteArray value;
    value.append(0x08); // Inclination avaiable
    value.append((char)0x01); // heart rate avaiable

    uint16_t normalizeSpeed = (uint16_t)qRound(treadMill->currentSpeed() * 100);
    char a = (normalizeSpeed >> 8) & 0XFF;
    char b = normalizeSpeed & 0XFF;
    QByteArray speedBytes;
    speedBytes.append(b);
    speedBytes.append(a);
    uint16_t normalizeIncline = (uint32_t)qRound(treadMill->currentInclination() * 10);
    a = (normalizeIncline >> 8) & 0XFF;
    b = normalizeIncline & 0XFF;
    QByteArray inclineBytes;
    inclineBytes.append(b);
    inclineBytes.append(a);
    double ramp = qRadiansToDegrees(qAtan(treadMill->currentInclination()/100));
    int16_t normalizeRamp = (int32_t)qRound(ramp * 10);
    a = (normalizeRamp >> 8) & 0XFF;
    b = normalizeRamp & 0XFF;
    QByteArray rampBytes;
    rampBytes.append(b);
    rampBytes.append(a);

    value.append(speedBytes); // Actual value.

    value.append(inclineBytes); //incline

    value.append(rampBytes);  //ramp angle

    value.append(treadMill->currentHeart()); // current heart rate

    QLowEnergyCharacteristic characteristic
            = service->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD); //TreadmillDataCharacteristicUuid
    Q_ASSERT(characteristic.isValid());
    if(leController->state() != QLowEnergyController::ConnectedState)
    {
        emit debug("virtualtreadmill connection error");
        return;
    }
    try {
       service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    } catch (...) {
        emit debug("virtualtreadmill error!");
    }

    //characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    //Q_ASSERT(characteristic.isValid());
    //service->readCharacteristic(characteristic);

    if(noHeartService == false)
    {
        QByteArray valueHR;
        valueHR.append(char(0)); // Flags that specify the format of the value.
        valueHR.append(char(treadMill->currentHeart())); // Actual value.
        QLowEnergyCharacteristic characteristicHR
                = serviceHR->characteristic(QBluetoothUuid::HeartRateMeasurement);
        Q_ASSERT(characteristicHR.isValid());
        if(leController->state() != QLowEnergyController::ConnectedState)
        {
            emit debug("virtualtreadmill connection error");
            return;
        }
        try {
            serviceHR->writeCharacteristic(characteristicHR, valueHR); // Potentially causes notification.
        } catch (...) {
            emit debug("virtualtreadmill error!");
        }
    }
}

bool virtualtreadmill::connected()
{
    if(!leController)
        return false;
    return leController->state() == QLowEnergyController::ConnectedState;
}
