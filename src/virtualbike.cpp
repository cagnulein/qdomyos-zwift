#include "virtualbike.h"
#include <QtMath>

virtualbike::virtualbike(bike* t)
{
    Bike = t;

    //! [Advertising Data]    
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("DomyosBridge");
    QList<QBluetoothUuid> services;
    services << (QBluetoothUuid::ServiceClassUuid::CyclingPower);
    services << QBluetoothUuid::HeartRate;
    advertisingData.setServices(services);
    //! [Advertising Data]

    //! [Service Data]
    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::CharacteristicType::CyclingPowerFeature);
    QByteArray value;
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x08);   // crank revolution
    charData.setValue(value);
    charData.setProperties(QLowEnergyCharacteristic::Read);
    /*const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                QByteArray(4, 0));
    charData.addDescriptor(clientConfig);*/

    QLowEnergyCharacteristicData charData2;
    charData2.setUuid(QBluetoothUuid::CharacteristicType::SensorLocation);
    charData2.setProperties(QLowEnergyCharacteristic::Read);
    QByteArray valueLocaltion;
    valueLocaltion.append((char)13); // rear hub
    charData2.setValue(valueLocaltion);
    /*const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                QByteArray(2, 0));
    charData2.addDescriptor(clientConfig2);*/

    QLowEnergyCharacteristicData charData3;
    charData3.setUuid(QBluetoothUuid::CharacteristicType::CyclingPowerMeasurement);
    charData3.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify);
    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);
    const QLowEnergyDescriptorData clientConfig3(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                descriptor);
    charData3.addDescriptor(clientConfig3);

/*    QLowEnergyCharacteristicData charData3;
    charData3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); //Fitness Machine Control Point
    charData3.setProperties(QLowEnergyCharacteristic::Write);*/

    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::CyclingPower);
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

    serviceDataHR.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceDataHR.setUuid(QBluetoothUuid::HeartRate);
    serviceDataHR.addCharacteristic(charDataHR);

    //! [Start Advertising]
    leController = QLowEnergyController::createPeripheral();
    Q_ASSERT(leController);
    service = leController->addService(serviceData);
    serviceHR = leController->addService(serviceDataHR);

    QObject::connect(service, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);
    leController->startAdvertising(pars, advertisingData,
                                   advertisingData);
    //! [Start Advertising]

    //! [Provide Heartbeat]    
    QObject::connect(&bikeTimer, SIGNAL(timeout()), this, SLOT(bikeProvider()));
    bikeTimer.start(1000);
    //! [Provide Heartbeat]
    QObject::connect(leController, SIGNAL(disconnected()), this, SLOT(reconnect()));
}

void virtualbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "characteristicChanged" << characteristic.uuid().toUInt16() << newValue;

    switch(characteristic.uuid().toUInt16())
    {
         break;
    }
}

void virtualbike::reconnect()
{
    service = leController->addService(serviceData);
    service = leController->addService(serviceDataHR);
    if (service)
        leController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                       advertisingData, advertisingData);
}

void virtualbike::bikeProvider()
{
    if(leController->state() != QLowEnergyController::ConnectedState) return;

    static int i=0;
    i++;
    QByteArray value;
    value.append((char)0x00); // crank data present
    value.append((char)0x20); // crank data present
    value.append((char)0x00); // watts
    value.append((char)0x00); // watts
    value.append((char)0x00); // revcount
    value.append((char)0x00);    // revcount
    value.append((char)0x00); // eventtime
    value.append((char)0x00); // eventtime

    QLowEnergyCharacteristic characteristic
            = service->characteristic(QBluetoothUuid::CharacteristicType::CyclingPowerMeasurement);
    Q_ASSERT(characteristic.isValid());
    if(leController->state() != QLowEnergyController::ConnectedState) return;
    try {
       service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    } catch (...) {}

    //characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    //Q_ASSERT(characteristic.isValid());
    //service->readCharacteristic(characteristic);

    QByteArray valueHR;
    valueHR.append(char(0)); // Flags that specify the format of the value.
    valueHR.append(char(Bike->currentHeart())); // Actual value.
    QLowEnergyCharacteristic characteristicHR
            = serviceHR->characteristic(QBluetoothUuid::HeartRateMeasurement);
    Q_ASSERT(characteristicHR.isValid());
    if(leController->state() != QLowEnergyController::ConnectedState) return;
    try {
      serviceHR->writeCharacteristic(characteristicHR, valueHR); // Potentially causes notification.
    } catch (...) {}
}

bool virtualbike::connected()
{
    if(!leController)
        return false;
    return leController->state() == QLowEnergyController::ConnectedState;
}
