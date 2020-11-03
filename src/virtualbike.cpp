#include "virtualbike.h"
#include <QtMath>

virtualbike::virtualbike(bike* t, bool noWriteResistance)
{
    Bike = t;
    Q_UNUSED(noWriteResistance)

    //! [Advertising Data]    
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("DomyosBridge");
    QList<QBluetoothUuid> services;
    services << ((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    services << QBluetoothUuid::HeartRate;
    advertisingData.setServices(services);
    //! [Advertising Data]

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

    QLowEnergyCharacteristicData charDataFIT4;
    charDataFIT4.setUuid((QBluetoothUuid::CharacteristicType)0x2AD2); //indoor bike
    charDataFIT4.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);
    const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                descriptor);
    charDataFIT4.addDescriptor(clientConfig4);

    serviceDataFIT.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    serviceDataFIT.addCharacteristic(charDataFIT);
    serviceDataFIT.addCharacteristic(charDataFIT2);
    serviceDataFIT.addCharacteristic(charDataFIT3);
    serviceDataFIT.addCharacteristic(charDataFIT4);

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
    serviceHR = leController->addService(serviceDataHR);
    serviceFIT = leController->addService(serviceDataFIT);

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
    serviceHR = leController->addService(serviceDataHR);
    serviceFIT = leController->addService(serviceDataFIT);

    if (serviceFIT)
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

    value.append((char)0x64); // speed, inst. cadence, resistance lvl, instant power
    value.append((char)0x02); // heart rate

    uint16_t normalizeSpeed = (uint16_t)qRound(Bike->currentSpeed() * 100);
    value.append((char)(normalizeSpeed & 0xFF)); // speed
    value.append((char)(normalizeSpeed >> 8) & 0xFF); // speed

    value.append((char)(Bike->currentCadence() * 2)); // cadence
    value.append((char)(0)); // cadence

    value.append((char)Bike->currentResistance()); // resistance
    value.append((char)(0)); // resistance

    value.append((char)(Bike->watts() & 0xFF)); // watts
    value.append((char)(Bike->watts() >> 8) & 0xFF); // watts

    value.append(char(Bike->currentHeart())); // Actual value.
    value.append(char(0)); // Actual value.

    QLowEnergyCharacteristic characteristic
            = serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
    Q_ASSERT(characteristic.isValid());
    if(leController->state() != QLowEnergyController::ConnectedState)
    {
        emit debug("virtual bike not connected");
        return;
    }
    try {
       serviceFIT->writeCharacteristic(characteristic, value); // Potentially causes notification.
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
