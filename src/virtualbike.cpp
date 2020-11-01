#include "virtualbike.h"
#include <QtMath>

virtualbike::virtualbike(bike* t, bool noWriteResistance)
{
    Bike = t;
    this->noWriteResistance = noWriteResistance;

    //! [Advertising Data]    
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("DomyosBridge");
    QList<QBluetoothUuid> services;
    services << (QBluetoothUuid::ServiceClassUuid::CyclingPower);    
    services << QBluetoothUuid::HeartRate;
    if(!noWriteResistance)
        services << ((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    advertisingData.setServices(services);
    //! [Advertising Data]

    //! [Service Data]
    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::CharacteristicType::CyclingPowerFeature);
    QByteArray value;
    value.append((char)0x08);
    value.append((char)0x00);
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

    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::CyclingPower);
    serviceData.addCharacteristic(charData);
    serviceData.addCharacteristic(charData2);
    serviceData.addCharacteristic(charData3);
    //! [Service Data]

    //! [Fitness Service Data]
    if(noWriteResistance == false)
    {
        serviceDataFIT.setType(QLowEnergyServiceData::ServiceTypePrimary);
        QLowEnergyCharacteristicData charDataFIT;
        charDataFIT.setUuid((QBluetoothUuid::CharacteristicType)0x2ACC); //FitnessMachineFeatureCharacteristicUuid
        QByteArray valueFIT;
        valueFIT.append((char)0x80); // resistance level supported
        valueFIT.append((char)0x14); // heart rate and elapsed time
        valueFIT.append((char)0x00);
        valueFIT.append((char)0x00);
        valueFIT.append((char)0x00);
        valueFIT.append((char)0x00);
        valueFIT.append((char)0x00);
        valueFIT.append((char)0x00);
        charDataFIT.setValue(valueFIT);
        charDataFIT.setProperties(QLowEnergyCharacteristic::Read);

        QLowEnergyCharacteristicData charDataFIT2;
        charDataFIT2.setUuid((QBluetoothUuid::CharacteristicType)0x2AD6); //supported_resistance_level_rangeCharacteristicUuid
        charDataFIT2.setProperties(QLowEnergyCharacteristic::Read);
        QByteArray valueFIT2;
        valueFIT2.append((char)0x0A);  // min resistance value
        valueFIT2.append((char)0x00);  // min resistance value
        valueFIT2.append((char)0x96);  // max resistance value
        valueFIT2.append((char)0x00);  // max resistance value
        valueFIT2.append((char)0x0A);  // step resistance
        valueFIT2.append((char)0x00);  // step resistance
        charDataFIT2.setValue(valueFIT2);

        QLowEnergyCharacteristicData charDataFIT3;
        charDataFIT3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); //Fitness Machine Control Point
        charDataFIT3.setProperties(QLowEnergyCharacteristic::Write);

        serviceDataFIT.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
        serviceDataFIT.addCharacteristic(charDataFIT);
        serviceDataFIT.addCharacteristic(charDataFIT2);
        serviceDataFIT.addCharacteristic(charDataFIT3);
    }
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
    if(!noWriteResistance)
        serviceFIT = leController->addService(serviceDataFIT);

    QObject::connect(service, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));
    if(!noWriteResistance)
        QObject::connect(serviceFIT, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));

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
    emit debug("characteristicChanged " + QString::number(characteristic.uuid().toUInt16()) + " " + newValue);

    switch(characteristic.uuid().toUInt16())
    {
       case 0x2AD9: // Fitness Machine Control Point
         if((char)newValue.at(0)==0x04)
         {
            // Set Target Resistance
            uint8_t uresistance = newValue.at(1);
            Bike->changeResistance(uresistance);
            emit debug("new requested resistance " + QString::number(uresistance));
         }
        break;
    }
}

void virtualbike::reconnect()
{
    emit debug("virtualbike::reconnect");
    service = leController->addService(serviceData);
    serviceHR = leController->addService(serviceDataHR);
    if(!noWriteResistance)
        serviceFIT = leController->addService(serviceDataFIT);

    if (service)
        leController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                       advertisingData, advertisingData);
}

void virtualbike::bikeProvider()
{
    if(leController->state() != QLowEnergyController::ConnectedState)
    {
        emit debug("virtual bike not connected");
        return;
    }

    QByteArray value;

    value.append((char)0x20); // crank data present
    value.append((char)0x00); // crank data present    
    value.append((char)(Bike->watts() & 0xFF)); // watts
    value.append((char)(Bike->watts() >> 8) & 0xFF); // watts    
    value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF)); // revs count
    value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count    
    value.append((char)(Bike->lastCrankEventTime() & 0xff)); // eventtime
    value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF); // eventtime    

    QLowEnergyCharacteristic characteristic
            = service->characteristic(QBluetoothUuid::CharacteristicType::CyclingPowerMeasurement);
    Q_ASSERT(characteristic.isValid());
    if(leController->state() != QLowEnergyController::ConnectedState)
    {
        emit debug("virtual bike not connected");
        return;
    }
    try {
       service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    } catch (...) {
        emit debug("virtual bike error!");
    }

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
    if(leController->state() != QLowEnergyController::ConnectedState)
    {
        emit debug("virtual bike not connected");
        return;
    }
    try {
      serviceHR->writeCharacteristic(characteristicHR, valueHR); // Potentially causes notification.
    } catch (...) {
        emit debug("virtual bike error!");
    }
}

bool virtualbike::connected()
{
    if(!leController)
        return false;
    return leController->state() == QLowEnergyController::ConnectedState;
}
