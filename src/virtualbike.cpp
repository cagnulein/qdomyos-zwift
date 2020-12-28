#include "virtualbike.h"
#include <QtMath>
#include <QMetaEnum>
#include <QDataStream>
#include <QSettings>

enum FtmsControlPointCommand {
    FTMS_REQUEST_CONTROL = 0x00,
    FTMS_RESET,
    FTMS_SET_TARGET_SPEED,
    FTMS_SET_TARGET_INCLINATION,
    FTMS_SET_TARGET_RESISTANCE_LEVEL,
    FTMS_SET_TARGET_POWER,
    FTMS_SET_TARGET_HEARTRATE,
    FTMS_START_RESUME,
    FTMS_STOP_PAUSE,
    FTMS_SET_TARGETED_EXP_ENERGY,
    FTMS_SET_TARGETED_STEPS,
    FTMS_SET_TARGETED_STRIDES,
    FTMS_SET_TARGETED_DISTANCE,
    FTMS_SET_TARGETED_TIME,
    FTMS_SET_TARGETED_TIME_TWO_HR_ZONES,
    FTMS_SET_TARGETED_TIME_THREE_HR_ZONES,
    FTMS_SET_TARGETED_TIME_FIVE_HR_ZONES,
    FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS,
    FTMS_SET_WHEEL_CIRCUMFERENCE,
    FTMS_SPIN_DOWN_CONTROL,
    FTMS_SET_TARGETED_CADENCE,
    FTMS_RESPONSE_CODE = 0x80
};

enum FtmsResultCode {
    FTMS_SUCCESS = 0x01,
    FTMS_NOT_SUPPORTED,
    FTMS_INVALID_PARAMETER,
    FTMS_OPERATION_FAILED,
    FTMS_CONTROL_NOT_PERMITTED
};

virtualbike::virtualbike(bike* t, bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset, uint8_t bikeResistanceGain)
{
    Bike = t;

    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    QSettings settings;
    bool cadence = settings.value("bike_cadence_sensor", false).toBool();
    bool power = settings.value("bike_power_sensor", false).toBool();
    bool battery = settings.value("battery_service", false).toBool();

    Q_UNUSED(noWriteResistance)

    //! [Advertising Data]    
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("DomyosBridge");  // save chars for service
    QList<QBluetoothUuid> services;

    if(!cadence && !power)
        services << ((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    else if(power)
        services << (QBluetoothUuid::ServiceClassUuid::CyclingPower);
    else
        services << (QBluetoothUuid::ServiceClassUuid::CyclingSpeedAndCadence);

    if(!this->noHeartService)
        services << QBluetoothUuid::HeartRate;

    services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);

    advertisingData.setServices(services);
    //! [Advertising Data]

    if(!cadence && !power)
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
        charDataFIT3.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
        const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                      QByteArray(2, 0));
        charDataFIT3.addDescriptor(cpClientConfig);

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
    }
    else if (power)
    {
        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid::CharacteristicType::CyclingPowerFeature);
        charData.setProperties(QLowEnergyCharacteristic::Read);
        QByteArray value;
        value.append((char)0x08);  // crank supported
        value.append((char)0x00);
        value.append((char)0x00);
        value.append((char)0x00);
        charData.setValue(value);

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
        charData3.setProperties(QLowEnergyCharacteristic::Notify);
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                     descriptor);
        charData3.addDescriptor(clientConfig4);

        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::CyclingPower);
        serviceData.addCharacteristic(charData);
        serviceData.addCharacteristic(charData3);
        serviceData.addCharacteristic(charData2);
    }
    else
    {
        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid::CharacteristicType::CSCFeature);
        charData.setProperties(QLowEnergyCharacteristic::Read);
        QByteArray value;
        value.append((char)0x02);  // crank supported
        value.append((char)0x00);
        charData.setValue(value);

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
        charData3.setUuid(QBluetoothUuid::CharacteristicType::CSCMeasurement);
        charData3.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify);
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                     descriptor);
        charData3.addDescriptor(clientConfig4);

        QLowEnergyCharacteristicData charData4;
        charData4.setUuid(QBluetoothUuid::CharacteristicType::SCControlPoint);
        charData4.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
        const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                      QByteArray(2, 0));
        charData4.addDescriptor(cpClientConfig);

        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::CyclingSpeedAndCadence);
        serviceData.addCharacteristic(charData);
        serviceData.addCharacteristic(charData3);
        serviceData.addCharacteristic(charData2);
        serviceData.addCharacteristic(charData4);
    }

    if(battery)
    {
        QLowEnergyCharacteristicData charDataBattery;
        charDataBattery.setUuid(QBluetoothUuid::BatteryLevel);
        charDataBattery.setValue(QByteArray(2, 0));
        charDataBattery.setProperties(QLowEnergyCharacteristic::Notify);
        const QLowEnergyDescriptorData clientConfigBattery(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                      QByteArray(2, 0));
        charDataBattery.addDescriptor(clientConfigBattery);

        serviceDataBattery.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceDataBattery.setUuid(QBluetoothUuid::BatteryService);
        serviceDataBattery.addCharacteristic(charDataBattery);
    }

    if(!this->noHeartService)
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
    if(!cadence && !power)
        serviceFIT = leController->addService(serviceDataFIT);
    else
        service = leController->addService(serviceData);

    if(battery)
        serviceBattery = leController->addService(serviceDataBattery);

    if(!this->noHeartService)
        serviceHR = leController->addService(serviceDataHR);


    if(!cadence && !power)
        QObject::connect(serviceFIT, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));
    else
        QObject::connect(service, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));

    bool bluetooth_relaxed = settings.value("bluetooth_relaxed", false).toBool();
    QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
    if(!bluetooth_relaxed)
        pars.setInterval(100, 100);

    leController->startAdvertising(pars, advertisingData,
                                   advertisingData);
    //! [Start Advertising]

    //! [Provide Heartbeat]    
    QObject::connect(&bikeTimer, SIGNAL(timeout()), this, SLOT(bikeProvider()));
    bikeTimer.start(1000);
    //! [Provide Heartbeat]
    QObject::connect(leController, SIGNAL(disconnected()), this, SLOT(reconnect()));
    QObject::connect(leController, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(error(QLowEnergyController::Error)));
}

void virtualbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    QByteArray reply;
    emit debug("characteristicChanged " + QString::number(characteristic.uuid().toUInt16()) + " " + newValue.toHex(' '));

    switch(characteristic.uuid().toUInt16())
    {
       case 0x2AD9: // Fitness Machine Control Point
         if((char)newValue.at(0) == FTMS_SET_TARGET_RESISTANCE_LEVEL)
         {
            // Set Target Resistance
            uint8_t uresistance = newValue.at(1);
            uresistance = uresistance / 10;
            Bike->changeResistance(uresistance);
            emit debug("new requested resistance " + QString::number(uresistance));
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_RESISTANCE_LEVEL);
            reply.append((quint8)FTMS_SUCCESS);
         }
         else if((char)newValue.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) // simulation parameter
         {
             emit debug("indoor bike simulation parameters");
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((quint8)FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS);
             reply.append((quint8)FTMS_SUCCESS);

             int16_t iresistance = (newValue.at(3) + (newValue.at(4) << 8));
             double resistance = ((double)iresistance) / 100.0;
             Bike->changeResistance((uint8_t)(resistance * bikeResistanceGain) + bikeResistanceOffset + 1); // resistance start from 1
         }
         else if((char)newValue.at(0) == FTMS_START_RESUME)
         {
             emit debug("start simulation!");
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((quint8)FTMS_START_RESUME);
             reply.append((quint8)FTMS_SUCCESS);
         }
         else if((char)newValue.at(0) == FTMS_REQUEST_CONTROL)
         {
             emit debug("control requested");
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((quint8)FTMS_REQUEST_CONTROL);
             reply.append((quint8)FTMS_SUCCESS);
         }
         else
         {
             emit debug("not supported");
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((quint8)newValue.at(0));
             reply.append((quint8)FTMS_NOT_SUPPORTED);
         }

         QLowEnergyCharacteristic characteristic
                 = serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9);
         Q_ASSERT(characteristic.isValid());
         if(leController->state() != QLowEnergyController::ConnectedState)
         {
             emit debug("virtual bike not connected");
             return;
         }
         writeCharacteristic(serviceFIT, characteristic, reply);
        break;
    }
}

void virtualbike::writeCharacteristic(QLowEnergyService* service, QLowEnergyCharacteristic characteristic, QByteArray value)
{
    try {
       emit debug("virtualbike::writeCharacteristic " + service->serviceName() + " " + characteristic.name() + " " + value.toHex(' '));
       service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    } catch (...) {
        emit debug("virtual bike error!");
    }
}

void virtualbike::reconnect()
{
    QSettings settings;
    bool bluetooth_relaxed = settings.value("bluetooth_relaxed", false).toBool();

    if(bluetooth_relaxed) return;

    bool cadence = settings.value("bike_cadence_sensor", false).toBool();
    bool battery = settings.value("battery_service", false).toBool();
    bool power = settings.value("bike_power_sensor", false).toBool();

    emit debug("virtualbike::reconnect");
    leController->disconnectFromDevice();

    if(!cadence && !power)
        serviceFIT = leController->addService(serviceDataFIT);
    else
        service = leController->addService(serviceData);

    if(battery)
        serviceBattery = leController->addService(serviceDataBattery);

    if(!this->noHeartService)
        serviceHR = leController->addService(serviceDataHR);

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);
    leController->startAdvertising(pars,
                                   advertisingData, advertisingData);
}

void virtualbike::bikeProvider()
{
    QSettings settings;
    bool cadence = settings.value("bike_cadence_sensor", false).toBool();
    bool battery = settings.value("battery_service", false).toBool();
    bool power = settings.value("bike_power_sensor", false).toBool();

    if(leController->state() != QLowEnergyController::ConnectedState)
    {
        emit debug("virtual bike not connected");
        return;
    }
    else
    {
        bool bluetooth_relaxed = settings.value("bluetooth_relaxed", false).toBool();
        if(bluetooth_relaxed)
            leController->stopAdvertising();
        emit debug("virtual bike connected");
    }

    QByteArray value;

    if(!cadence && !power)
    {
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

        QLowEnergyCharacteristic characteristic
                = serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
        Q_ASSERT(characteristic.isValid());
        if(leController->state() != QLowEnergyController::ConnectedState)
        {
            emit debug("virtual bike not connected");
            return;
        }
        writeCharacteristic(serviceFIT, characteristic, value);
    }
    else if(power)
    {
        value.append((char)0x10); // crank data present
        value.append((char)(((uint16_t)Bike->watts()) & 0xFF)); // watt
        value.append((char)(((uint16_t)Bike->watts()) >> 8) & 0xFF); // watt
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
        writeCharacteristic(service, characteristic, value);
    }
    else
    {
        value.append((char)0x02); // crank data present
        value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF)); // revs count
        value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count
        value.append((char)(Bike->lastCrankEventTime() & 0xff)); // eventtime
        value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF); // eventtime

        QLowEnergyCharacteristic characteristic
                = service->characteristic(QBluetoothUuid::CharacteristicType::CSCMeasurement);
        Q_ASSERT(characteristic.isValid());
        if(leController->state() != QLowEnergyController::ConnectedState)
        {
            emit debug("virtual bike not connected");
            return;
        }
        writeCharacteristic(service, characteristic, value);
    }
    //characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    //Q_ASSERT(characteristic.isValid());
    //service->readCharacteristic(characteristic);

    if(battery)
    {
        QByteArray valueBattery;
        valueBattery.append(100); // Actual value.
        QLowEnergyCharacteristic characteristicBattery
                = serviceBattery->characteristic(QBluetoothUuid::BatteryLevel);
        Q_ASSERT(characteristicBattery.isValid());
        if(leController->state() != QLowEnergyController::ConnectedState)
        {
            emit debug("virtual bike not connected");
            return;
        }
        writeCharacteristic(serviceBattery, characteristicBattery, valueBattery);
    }

    if(!this->noHeartService)
    {
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
        writeCharacteristic(serviceHR, characteristicHR, valueHR);
    }
}

bool virtualbike::connected()
{
    if(!leController)
        return false;
    return leController->state() == QLowEnergyController::ConnectedState;
}

void virtualbike::error(QLowEnergyController::Error newError)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("virtualbike::controller:ERROR " + QString::fromLocal8Bit(metaEnum.valueToKey(newError)));

    if(newError != QLowEnergyController::RemoteHostClosedError)
        reconnect();
}
