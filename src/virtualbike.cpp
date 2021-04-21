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

virtualbike::virtualbike(bike* t, bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset, double bikeResistanceGain)
{
    Bike = t;

    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    QSettings settings;
    bool cadence = settings.value("bike_cadence_sensor", false).toBool();
    bool bike_wheel_revs = settings.value("bike_wheel_revs", false).toBool();
    bool power = settings.value("bike_power_sensor", false).toBool();
    bool battery = settings.value("battery_service", false).toBool();
    bool service_changed = settings.value("service_changed", false).toBool();
    bool heart_only = settings.value("virtual_device_onlyheart", false).toBool();
    bool echelon = settings.value("virtual_device_echelon", false).toBool();

    Q_UNUSED(noWriteResistance)

    //! [Advertising Data]    
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    if(!echelon)
        advertisingData.setLocalName("DomyosBridge");  // save chars for service
    else
        advertisingData.setLocalName("ECHEX-5s-113399");
    QList<QBluetoothUuid> services;

    if(!echelon)
    {
        if(!heart_only)
        {
            if(!cadence && !power)
                services << ((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
            else if(power)
                services << (QBluetoothUuid::ServiceClassUuid::CyclingPower);
            else
                services << (QBluetoothUuid::ServiceClassUuid::CyclingSpeedAndCadence);
        }
        if(!this->noHeartService || heart_only)
            services << QBluetoothUuid::HeartRate;

        services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);
    }
    else
    {
        services << (QBluetoothUuid((QString)"0bf669f0-45f2-11e7-9598-0800200c9a66"));
    }

    advertisingData.setServices(services);
    //! [Advertising Data]

    if(!echelon)
    {
        if(!heart_only)
        {
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

                QLowEnergyCharacteristicData charDataFIT5;
                charDataFIT5.setUuid((QBluetoothUuid::CharacteristicType)0x2ADA); //Fitness Machine status
                charDataFIT5.setProperties(QLowEnergyCharacteristic::Notify);
                QByteArray descriptor5;
                descriptor5.append((char)0x01);
                descriptor5.append((char)0x00);
                const QLowEnergyDescriptorData clientConfig5(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                             descriptor5);
                charDataFIT5.addDescriptor(clientConfig5);

                QLowEnergyCharacteristicData charDataFIT6;
                charDataFIT6.setUuid((QBluetoothUuid::CharacteristicType)0x2AD3);
                charDataFIT6.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                QByteArray valueFIT6;
                valueFIT6.append((char)0x00);
                valueFIT6.append((char)0x01);
                charDataFIT6.setValue(valueFIT6);
                QByteArray descriptor6;
                descriptor6.append((char)0x01);
                descriptor6.append((char)0x00);
                const QLowEnergyDescriptorData clientConfig6(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                             descriptor6);
                charDataFIT6.addDescriptor(clientConfig6);
                charDataFIT6.setProperties(QLowEnergyCharacteristic::Read);

                serviceDataFIT.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
                serviceDataFIT.addCharacteristic(charDataFIT);
                serviceDataFIT.addCharacteristic(charDataFIT2);
                serviceDataFIT.addCharacteristic(charDataFIT3);
                serviceDataFIT.addCharacteristic(charDataFIT4);
                serviceDataFIT.addCharacteristic(charDataFIT5);
                serviceDataFIT.addCharacteristic(charDataFIT6);
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
                if(!bike_wheel_revs)
                    value.append((char)0x02);  // crank supported
                else
                    value.append((char)0x03);  // crank and wheel supported
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
        }
    }
    else
    {
        serviceEchelon.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceEchelon.setUuid((QBluetoothUuid((QString)"0bf669f0-45f2-11e7-9598-0800200c9a66")));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid((QString)"0bf669f2-45f2-11e7-9598-0800200c9a66"));
        charData.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse);

        QLowEnergyCharacteristicData charData2;
        charData2.setUuid(QBluetoothUuid((QString)"0bf669f4-45f2-11e7-9598-0800200c9a66"));
        charData2.setProperties(QLowEnergyCharacteristic::Notify);
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                     descriptor);
        charData2.addDescriptor(clientConfig2);

        QLowEnergyCharacteristicData charData3;
        charData3.setUuid(QBluetoothUuid((QString)"0bf669f3-45f2-11e7-9598-0800200c9a66"));
        charData3.setProperties(QLowEnergyCharacteristic::Notify);
        QByteArray descriptor3;
        descriptor3.append((char)0x01);
        descriptor3.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                     descriptor3);
        charData3.addDescriptor(clientConfig4);

        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid((QString)"0bf669f1-45f2-11e7-9598-0800200c9a66"));
        serviceData.addCharacteristic(charData);
        serviceData.addCharacteristic(charData3);
        serviceData.addCharacteristic(charData2);    
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

    if(!this->noHeartService || heart_only)
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

    if(service_changed)
    {
        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid::CharacteristicType::ServiceChanged);
        charData.setProperties(QLowEnergyCharacteristic::Indicate);
        const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                      QByteArray(2, 0));
        charData.addDescriptor(cpClientConfig);

        serviceDataChanged.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceDataChanged.setUuid(QBluetoothUuid::ServiceClassUuid::GenericAttribute);
        serviceDataChanged.addCharacteristic(charData);
    }

    //! [Start Advertising]
    leController = QLowEnergyController::createPeripheral();
    Q_ASSERT(leController);

    if(service_changed)
        serviceChanged = leController->addService(serviceDataChanged);

    if(!echelon)
    {
        if(!heart_only)
        {
            if(!cadence && !power)
                serviceFIT = leController->addService(serviceDataFIT);
            else
                service = leController->addService(serviceData);
        }
    }
    else
    {
        service = leController->addService(serviceEchelon);
        service = leController->addService(serviceData);
    }

    if(battery)
        serviceBattery = leController->addService(serviceDataBattery);

    if(!this->noHeartService || heart_only)
        serviceHR = leController->addService(serviceDataHR);

    if(!echelon)
    {
        if(!heart_only)
        {
            if(!cadence && !power)
                QObject::connect(serviceFIT, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));
            else
                QObject::connect(service, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));
        }
    }
    else
    {
        QObject::connect(service, SIGNAL(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)), this, SLOT(characteristicChanged(const QLowEnergyCharacteristic, const QByteArray)));
    }

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
    QSettings settings;
    bool force_resistance = settings.value("virtualbike_forceresistance", true).toBool();
    bool erg_mode = settings.value("zwift_erg", false).toBool();
    double erg_filter_upper = settings.value("zwift_erg_filter", 0.0).toDouble();
    double erg_filter_lower = settings.value("zwift_erg_filter_down", 0.0).toDouble();
    qDebug() << "characteristicChanged " + QString::number(characteristic.uuid().toUInt16()) + " " + newValue.toHex(' ');

    switch(characteristic.uuid().toUInt16())
    {
       case 0x2AD9: // Fitness Machine Control Point
         if((char)newValue.at(0) == FTMS_SET_TARGET_RESISTANCE_LEVEL)
         {
            // Set Target Resistance
            uint8_t uresistance = newValue.at(1);
            uresistance = uresistance / 10;
            if(force_resistance && !erg_mode)
                Bike->changeResistance(uresistance);
            qDebug() << "new requested resistance " + QString::number(uresistance) + " enabled " + force_resistance;
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_RESISTANCE_LEVEL);
            reply.append((quint8)FTMS_SUCCESS);
         }
         else if((char)newValue.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) // simulation parameter
         {
             qDebug() << "indoor bike simulation parameters";
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((quint8)FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS);
             reply.append((quint8)FTMS_SUCCESS);

             int16_t iresistance = (((uint8_t)newValue.at(3)) + (newValue.at(4) << 8));
             qDebug() << "new requested resistance zwift erg grade " + QString::number(iresistance) + " enabled " + force_resistance;
             double resistance = ((double)iresistance * 1.5) / 100.0;
             qDebug() << "calculated erg grade " + QString::number(resistance);
             if(force_resistance && !erg_mode)
                 Bike->changeResistance((int8_t)(round(resistance * bikeResistanceGain)) + bikeResistanceOffset + 1); // resistance start from 1
         }
         else if((char)newValue.at(0) == FTMS_SET_TARGET_POWER) // erg mode
         {
             qDebug() << "erg mode";
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((quint8)FTMS_SET_TARGET_POWER);
             reply.append((quint8)FTMS_SUCCESS);

             uint16_t power = (((uint8_t)newValue.at(1)) + (newValue.at(2) << 8));
             qDebug() << "power erg  " + QString::number(power) + " " + erg_mode;
             Bike->changePower(power);
             double deltaDown = Bike->wattsMetric().value() - ((double)power);
             double deltaUp = ((double)power) - Bike->wattsMetric().value();
             qDebug() << "filter  " + QString::number(deltaUp) + " " + QString::number(deltaDown) + " " +QString::number(erg_filter_upper) + " " +QString::number(erg_filter_lower);
             if(force_resistance && erg_mode && (deltaUp > erg_filter_upper || deltaDown > erg_filter_lower))
                 Bike->changeResistance((int8_t)Bike->resistanceFromPowerRequest(power)); // resistance start from 1
         }
         else if((char)newValue.at(0) == FTMS_START_RESUME)
         {
             qDebug() << "start simulation!";
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((quint8)FTMS_START_RESUME);
             reply.append((quint8)FTMS_SUCCESS);
         }
         else if((char)newValue.at(0) == FTMS_REQUEST_CONTROL)
         {
             qDebug() << "control requested";
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((char)FTMS_REQUEST_CONTROL);
             reply.append((quint8)FTMS_SUCCESS);
         }
         else
         {
             qDebug() << "not supported";
             reply.append((quint8)FTMS_RESPONSE_CODE);
             reply.append((quint8)newValue.at(0));
             reply.append((quint8)FTMS_NOT_SUPPORTED);
         }

         QLowEnergyCharacteristic characteristic
                 = serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9);
         Q_ASSERT(characteristic.isValid());
         if(leController->state() != QLowEnergyController::ConnectedState)
         {
             qDebug() << "virtual bike not connected";
             return;
         }
         writeCharacteristic(serviceFIT, characteristic, reply);
        break;


    }

    //******************** ECHELON ***************
    if(characteristic.uuid().toString().contains("0bf669f2-45f2-11e7-9598-0800200c9a66"))
    {
        QLowEnergyCharacteristic characteristic
                = service->characteristic(QBluetoothUuid((QString)"0bf669f3-45f2-11e7-9598-0800200c9a66"));
        Q_ASSERT(characteristic.isValid());
        if(leController->state() != QLowEnergyController::ConnectedState)
        {
            qDebug() << "virtual bike not connected";
            return;
        }

        QByteArray reply;
        if(((uint8_t)newValue.at(1)) == 0xA1)
        {
            // f0a106000700290104cc
            reply.append(0xf0);
            reply.append(0xa1);
            reply.append(0x06);
            reply.append((char)0x00);
            reply.append(0x07);
            reply.append((char)0x00);
            reply.append(0x29);
            reply.append(0x01);
            reply.append(0x04);
            reply.append(0xcc);
        }
        else if(((uint8_t)newValue.at(1)) == 0xA3)
        {
            // f0 a3 02 20 01 b6
            reply.append(0xf0);
            reply.append(0xa3);
            reply.append(0x02);
            reply.append(0x20);
            reply.append(0x01);
            reply.append(0xb6);
        }
        // f0 b0 01 00 a1
        else if(((uint8_t)newValue.at(1)) == 0xB0 && ((uint8_t)newValue.at(3)) == 0x00)
        {
            // f0 d0 01 00 c1
            reply.append(0xf0);
            reply.append(0xd0);
            reply.append(0x01);
            reply.append((char)0x00);
            reply.append(0xc1);
        }
        // f0 b0 01 01 a2
        else if(((uint8_t)newValue.at(1)) == 0xB0)
        {
            // f0 d0 01 01 c2
            reply.append(0xf0);
            reply.append(0xd0);
            reply.append(0x01);
            reply.append(0x01);
            reply.append(0xc2);
        }
        else if(((uint8_t)newValue.at(1)) == 0xA0)
        {
            reply = newValue;
        }

        writeCharacteristic(service, characteristic, reply);
    }
}

void virtualbike::writeCharacteristic(QLowEnergyService* service, QLowEnergyCharacteristic characteristic, QByteArray value)
{
    try {
       qDebug() << "virtualbike::writeCharacteristic " + service->serviceName() + " " + characteristic.name() + " " + value.toHex(' ');
       service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    } catch (...) {
        qDebug() << "virtual bike error!";
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
    bool service_changed = settings.value("service_changed", false).toBool();
    bool heart_only = settings.value("virtual_device_onlyheart", false).toBool();
    bool echelon = settings.value("virtual_device_echelon", false).toBool();

    qDebug() << "virtualbike::reconnect";
    leController->disconnectFromDevice();

#ifndef Q_OS_IOS
    if(service_changed)
        serviceChanged = leController->addService(serviceDataChanged);

    if(!echelon)
    {
        if(!heart_only)
        {
            if(!cadence && !power)
                serviceFIT = leController->addService(serviceDataFIT);
            else
                service = leController->addService(serviceData);
        }
    }
    else
    {
        service = leController->addService(serviceEchelon);
        service = leController->addService(serviceData);
    }

    if(battery)
        serviceBattery = leController->addService(serviceDataBattery);

    if(!this->noHeartService || heart_only)
        serviceHR = leController->addService(serviceDataHR);
#endif

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
    bool bike_wheel_revs = settings.value("bike_wheel_revs", false).toBool();
    bool heart_only = settings.value("virtual_device_onlyheart", false).toBool();
    bool echelon = settings.value("virtual_device_echelon", false).toBool();

    if(leController->state() != QLowEnergyController::ConnectedState)
    {
        qDebug() << "virtual bike not connected";
        return;
    }
    else
    {
        bool bluetooth_relaxed = settings.value("bluetooth_relaxed", false).toBool();
        if(bluetooth_relaxed)
            leController->stopAdvertising();
        qDebug() << "virtual bike connected";
    }

    QByteArray value;

    if(!echelon)
    {
        if(!heart_only)
        {
            if(!cadence && !power)
            {
                value.append((char)0x64); // speed, inst. cadence, resistance lvl, instant power
                value.append((char)0x02); // heart rate

                uint16_t normalizeSpeed = (uint16_t)qRound(Bike->currentSpeed().value() * 100);
                value.append((char)(normalizeSpeed & 0xFF)); // speed
                value.append((char)(normalizeSpeed >> 8) & 0xFF); // speed

                value.append((char)((uint16_t)(Bike->currentCadence().value() * 2) & 0xFF)); // cadence
                value.append((char)(((uint16_t)(Bike->currentCadence().value() * 2) >> 8) & 0xFF)); // cadence

                value.append((char)Bike->currentResistance().value()); // resistance
                value.append((char)(0)); // resistance

                value.append((char)(((uint16_t)Bike->wattsMetric().value()) & 0xFF)); // watts
                value.append((char)(((uint16_t)Bike->wattsMetric().value()) >> 8) & 0xFF); // watts

                value.append(char(Bike->currentHeart().value())); // Actual value.
                value.append((char)0); // Bkool FTMS protocol HRM offset 1280 fix

                if(!serviceFIT)
                {
                    qDebug() << "serviceFIT not available";
                    return;
                }

                QLowEnergyCharacteristic characteristic
                        = serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
                Q_ASSERT(characteristic.isValid());
                if(leController->state() != QLowEnergyController::ConnectedState)
                {
                    qDebug() << "virtual bike not connected";
                    return;
                }
                writeCharacteristic(serviceFIT, characteristic, value);
            }
            else if(power)
            {
                value.append((char)0x10); // crank data present
                value.append((char)(((uint16_t)Bike->wattsMetric().value()) & 0xFF)); // watt
                value.append((char)(((uint16_t)Bike->wattsMetric().value()) >> 8) & 0xFF); // watt
                value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF)); // revs count
                value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count
                value.append((char)(Bike->lastCrankEventTime() & 0xff)); // eventtime
                value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF); // eventtime

                if(!service)
                {
                    qDebug() << "service not available";
                    return;
                }

                QLowEnergyCharacteristic characteristic
                        = service->characteristic(QBluetoothUuid::CharacteristicType::CyclingPowerMeasurement);
                Q_ASSERT(characteristic.isValid());
                if(leController->state() != QLowEnergyController::ConnectedState)
                {
                    qDebug() << "virtual bike not connected";
                    return;
                }
                writeCharacteristic(service, characteristic, value);
            }
            else
            {
                if(!bike_wheel_revs)
                    value.append((char)0x02); // crank data present
                else
                {
                    value.append((char)0x03); // crank and wheel data present

                    if(Bike->currentSpeed().value())
                    {
                        const double wheelCircumference = 2000.0; // millimeters
                        wheelRevs++;
                        lastWheelTime += (uint16_t)(1024.0 / ((Bike->currentSpeed().value() / 3.6) / (wheelCircumference / 1000.0) ));
                    }
                    value.append((char)((wheelRevs & 0xFF))); // wheel count
                    value.append((char)((wheelRevs >> 8) & 0xFF)); // wheel count
                    value.append((char)((wheelRevs >> 16) & 0xFF)); // wheel count
                    value.append((char)((wheelRevs >> 24) & 0xFF)); // wheel count
                    value.append((char)(lastWheelTime & 0xff)); // eventtime
                    value.append((char)(lastWheelTime >> 8) & 0xFF); // eventtime
                }
                value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF)); // revs count
                value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count
                value.append((char)(Bike->lastCrankEventTime() & 0xff)); // eventtime
                value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF); // eventtime

                if(!service)
                {
                    qDebug() << "service not available";
                    return;
                }

                QLowEnergyCharacteristic characteristic
                        = service->characteristic(QBluetoothUuid::CharacteristicType::CSCMeasurement);
                Q_ASSERT(characteristic.isValid());
                if(leController->state() != QLowEnergyController::ConnectedState)
                {
                    qDebug() << "virtual bike not connected";
                    return;
                }
                writeCharacteristic(service, characteristic, value);
            }
        }
    }
    else
    {
        // TODO: set it do dynamic
        // f0 d1 09 00 00 00 00 00 01 00 5f 00 2a
        value.append(0xf0);
        value.append(0xd1);
        value.append(0x09);
        value.append((char)0x00); // elapsed
        value.append((char)0x00); // elapsed
        value.append((uint8_t)(((uint32_t)(Bike->odometer() * 100)) >> 24)); // distance
        value.append((uint8_t)(((uint32_t)(Bike->odometer() * 100)) >> 16)); // distance
        value.append((uint8_t)(((uint32_t)(Bike->odometer() * 100)) >> 8)); // distance
        value.append((uint8_t)(Bike->odometer() * 100));       // distance
        value.append((char)0x00);
        value.append(Bike->currentCadence().value());
        value.append((uint8_t)Bike->currentHeart().value());

        uint8_t sum = 0;
        for(uint8_t i=0; i<value.length(); i++)
        {
           sum += value[i]; // the last byte is a sort of a checksum
        }
        value.append(sum);

        if(!service)
        {
            qDebug() << "service not available";
            return;
        }

        QLowEnergyCharacteristic characteristic
                        = service->characteristic(QBluetoothUuid((QString)"0bf669f4-45f2-11e7-9598-0800200c9a66"));
        Q_ASSERT(characteristic.isValid());
        if(leController->state() != QLowEnergyController::ConnectedState)
        {
            qDebug() << "virtual bike not connected";
            return;
        }
        writeCharacteristic(service, characteristic, value);

        // resistance change notification
        // f0 d2 01 0b ce
        QByteArray resistance;
        static uint8_t oldresistance = 255;
        resistance.append(0xf0);
        resistance.append(0xd2);
        resistance.append(0x01);
        resistance.append(Bike->currentResistance().value());

        sum = 0;
        for(uint8_t i=0; i<resistance.length(); i++)
        {
           sum += resistance[i]; // the last byte is a sort of a checksum
        }
        resistance.append(sum);
        if(oldresistance != ((uint8_t)Bike->currentResistance().value()))
            writeCharacteristic(service, characteristic, resistance);
        oldresistance = ((uint8_t)Bike->currentResistance().value());

    }
    //characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    //Q_ASSERT(characteristic.isValid());
    //service->readCharacteristic(characteristic);

    if(battery)
    {
        if(!serviceBattery)
        {
            qDebug() << "serviceBattery not available";
            return;
        }

        QByteArray valueBattery;
        valueBattery.append(100); // Actual value.
        QLowEnergyCharacteristic characteristicBattery
                = serviceBattery->characteristic(QBluetoothUuid::BatteryLevel);
        Q_ASSERT(characteristicBattery.isValid());
        if(leController->state() != QLowEnergyController::ConnectedState)
        {
            qDebug() << "virtual bike not connected";
            return;
        }
        writeCharacteristic(serviceBattery, characteristicBattery, valueBattery);
    }

    if(!this->noHeartService || heart_only)
    {
        if(!serviceHR)
        {
            qDebug() << "serviceHR not available";
            return;
        }

        QByteArray valueHR;
        valueHR.append(char(0)); // Flags that specify the format of the value.
        valueHR.append(char(Bike->currentHeart().value())); // Actual value.
        QLowEnergyCharacteristic characteristicHR
                = serviceHR->characteristic(QBluetoothUuid::HeartRateMeasurement);
        Q_ASSERT(characteristicHR.isValid());
        if(leController->state() != QLowEnergyController::ConnectedState)
        {
            qDebug() << "virtual bike not connected";
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
    qDebug() << "virtualbike::controller:ERROR " + QString::fromLocal8Bit(metaEnum.valueToKey(newError));

    if(newError != QLowEnergyController::RemoteHostClosedError)
        reconnect();
}
