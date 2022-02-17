#include "virtualbike.h"
#include "ftmsbike.h"

#include <QDataStream>
#include <QMetaEnum>
#include <QSettings>
#include <QtMath>
#include <chrono>

using namespace std::chrono_literals;

virtualbike::virtualbike(bluetoothdevice *t, bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
    Bike = t;

    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    QSettings settings;
    bool cadence = settings.value(QStringLiteral("bike_cadence_sensor"), false).toBool();
    bool bike_wheel_revs = settings.value(QStringLiteral("bike_wheel_revs"), false).toBool();
    bool power = settings.value(QStringLiteral("bike_power_sensor"), false).toBool();
    bool battery = settings.value(QStringLiteral("battery_service"), false).toBool();
    bool service_changed = settings.value(QStringLiteral("service_changed"), false).toBool();
    bool heart_only = settings.value(QStringLiteral("virtual_device_onlyheart"), false).toBool();
    bool echelon = settings.value(QStringLiteral("virtual_device_echelon"), false).toBool();
    bool ifit = settings.value(QStringLiteral("virtual_device_ifit"), false).toBool();

    Q_UNUSED(noWriteResistance)

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
    if (ios_peloton_workaround && !cadence && !echelon && !ifit && !heart_only && !power) {

        qDebug() << "ios_zwift_workaround activated!";
        h = new lockscreen();
        h->virtualbike_zwift_ios();
    } else

#endif
#endif
    {
        //! [Advertising Data]
        advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
        advertisingData.setIncludePowerLevel(true);
        if (!echelon && !ifit) {
            advertisingData.setLocalName(QStringLiteral("DomyosBridge"));
        } else if (ifit) {
            advertisingData.setLocalName(QStringLiteral("I_EB"));
        } else {
            advertisingData.setLocalName(QStringLiteral("ECHEX-5s-113399"));
        }
        QList<QBluetoothUuid> services;

        if (!echelon && !ifit) {
            if (!heart_only) {
                if (!cadence && !power) {
                    services << ((QBluetoothUuid::ServiceClassUuid)0x1826);
                } // FitnessMachineServiceUuid
                else if (power) {

                    services << (QBluetoothUuid::ServiceClassUuid::CyclingPower);
                } else {
                    services << (QBluetoothUuid::ServiceClassUuid::CyclingSpeedAndCadence);
                }
            }
            if (!this->noHeartService || heart_only) {
                services << QBluetoothUuid::HeartRate;
            }

            services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);
        } else if (ifit) {
            services << (QBluetoothUuid(QStringLiteral("00001533-1412-efde-1523-785feabcd123")));

            if (!this->noHeartService) {
                services << QBluetoothUuid::HeartRate;
            }
        } else {
            services << (QBluetoothUuid(QStringLiteral("0bf669f0-45f2-11e7-9598-0800200c9a66")));

            if (!this->noHeartService) {
                services << QBluetoothUuid::HeartRate;
            }
        }

        advertisingData.setServices(services);
        //! [Advertising Data]

        if (!echelon && !ifit) {
            if (!heart_only) {
                if (!cadence && !power) {

                    serviceDataFIT.setType(QLowEnergyServiceData::ServiceTypePrimary);
                    QLowEnergyCharacteristicData charDataFIT;
                    charDataFIT.setUuid(
                        (QBluetoothUuid::CharacteristicType)0x2ACC); // FitnessMachineFeatureCharacteristicUuid
                    QByteArray valueFIT;
                    valueFIT.append((char)0x83); // average speed, cadence and resistance level supported
                    valueFIT.append((char)0x14); // heart rate and elapsed time
                    valueFIT.append((char)0x00);
                    valueFIT.append((char)0x00);
                    valueFIT.append((char)0x0C); // resistance and power target supported
                    valueFIT.append((char)0xE0); // indoor simulation, wheel and spin down supported
                    valueFIT.append((char)0x00);
                    valueFIT.append((char)0x00);
                    charDataFIT.setValue(valueFIT);
                    charDataFIT.setProperties(QLowEnergyCharacteristic::Read);

                    QLowEnergyCharacteristicData charDataFIT2;
                    charDataFIT2.setUuid(
                        (QBluetoothUuid::
                             CharacteristicType)0x2AD6); // supported_resistance_level_rangeCharacteristicUuid
                    charDataFIT2.setProperties(QLowEnergyCharacteristic::Read);
                    QByteArray valueFIT2;
                    valueFIT2.append((char)0x0A); // min resistance value
                    valueFIT2.append((char)0x00); // min resistance value
                    valueFIT2.append((char)0x96); // max resistance value
                    valueFIT2.append((char)0x00); // max resistance value
                    valueFIT2.append((char)0x0A); // step resistance
                    valueFIT2.append((char)0x00); // step resistance
                    charDataFIT2.setValue(valueFIT2);

                    QLowEnergyCharacteristicData charDataFIT3;
                    charDataFIT3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
                    charDataFIT3.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate | QLowEnergyCharacteristic::Notify);
                    const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                                  QByteArray(3, 0));
                    charDataFIT3.addDescriptor(cpClientConfig);

                    QLowEnergyCharacteristicData charDataFIT4;
                    charDataFIT4.setUuid((QBluetoothUuid::CharacteristicType)0x2AD2); // indoor bike
                    charDataFIT4.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                                 descriptor);
                    charDataFIT4.addDescriptor(clientConfig4);

                    QLowEnergyCharacteristicData charDataFIT5;
                    charDataFIT5.setUuid((QBluetoothUuid::CharacteristicType)0x2ADA); // Fitness Machine status
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

                    serviceDataFIT.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
                    serviceDataFIT.addCharacteristic(charDataFIT);
                    serviceDataFIT.addCharacteristic(charDataFIT2);
                    serviceDataFIT.addCharacteristic(charDataFIT3);
                    serviceDataFIT.addCharacteristic(charDataFIT4);
                    serviceDataFIT.addCharacteristic(charDataFIT5);
                    serviceDataFIT.addCharacteristic(charDataFIT6);
                } else if (power) {

                    QLowEnergyCharacteristicData charData;
                    charData.setUuid(QBluetoothUuid::CharacteristicType::CyclingPowerFeature);
                    charData.setProperties(QLowEnergyCharacteristic::Read);
                    QByteArray value;
                    value.append((char)0x08); // crank supported
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
                } else {

                    QLowEnergyCharacteristicData charData;
                    charData.setUuid(QBluetoothUuid::CharacteristicType::CSCFeature);
                    charData.setProperties(QLowEnergyCharacteristic::Read);
                    QByteArray value;
                    if (!bike_wheel_revs) {
                        value.append((char)0x02); // crank supported
                    } else {
                        value.append((char)0x03); // crank and wheel supported
                    }
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
        } else if (ifit) {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(QStringLiteral("00001534-1412-efde-1523-785feabcd123")));
            charData.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse);

            QLowEnergyCharacteristicData charData2;
            charData2.setUuid(QBluetoothUuid(QStringLiteral("00001535-1412-efde-1523-785feabcd123")));
            charData2.setProperties(QLowEnergyCharacteristic::Notify);
            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor);

            charData2.addDescriptor(clientConfig2);

            serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceData.setUuid(QBluetoothUuid(QStringLiteral("00001533-1412-efde-1523-785feabcd123")));
            serviceData.addCharacteristic(charData);
            serviceData.addCharacteristic(charData2);
        } else {

            serviceEchelon.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceEchelon.setUuid((QBluetoothUuid(QStringLiteral("0bf669f0-45f2-11e7-9598-0800200c9a66"))));

            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(QStringLiteral("0bf669f2-45f2-11e7-9598-0800200c9a66")));
            charData.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse);

            QLowEnergyCharacteristicData charData2;
            charData2.setUuid(QBluetoothUuid(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66")));
            charData2.setProperties(QLowEnergyCharacteristic::Notify);
            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor);

            charData2.addDescriptor(clientConfig2);

            QLowEnergyCharacteristicData charData3;
            charData3.setUuid(QBluetoothUuid(QStringLiteral("0bf669f3-45f2-11e7-9598-0800200c9a66")));
            charData3.setProperties(QLowEnergyCharacteristic::Notify);
            QByteArray descriptor3;
            descriptor3.append((char)0x01);
            descriptor3.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                         descriptor3);
            charData3.addDescriptor(clientConfig4);

            serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceData.setUuid(QBluetoothUuid(QStringLiteral("0bf669f1-45f2-11e7-9598-0800200c9a66")));
            serviceData.addCharacteristic(charData);
            serviceData.addCharacteristic(charData3);
            serviceData.addCharacteristic(charData2);
        }

        if (battery) {

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

        if (!this->noHeartService || heart_only) {

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

        if (service_changed) {

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

        if (service_changed)
            serviceChanged = leController->addService(serviceDataChanged);

        if (!echelon && !ifit) {
            if (!heart_only) {
                if (!cadence && !power) {

                    serviceFIT = leController->addService(serviceDataFIT);
                } else {
                    service = leController->addService(serviceData);
                }
            }
        } else if (ifit) {
            service = leController->addService(serviceData);
        } else {

            service = leController->addService(serviceEchelon);
            service = leController->addService(serviceData);
        }

        if (battery) {
            serviceBattery = leController->addService(serviceDataBattery);
        }

        if (!this->noHeartService || heart_only) {
            serviceHR = leController->addService(serviceDataHR);
        }

        if (!echelon && !ifit) {
            if (!heart_only) {
                if (!cadence && !power) {
                    QObject::connect(serviceFIT, &QLowEnergyService::characteristicChanged, this,
                                     &virtualbike::characteristicChanged);
                } else {
                    QObject::connect(service, &QLowEnergyService::characteristicChanged, this,
                                     &virtualbike::characteristicChanged);
                }
            }
        } else if (ifit) {
            QObject::connect(service, &QLowEnergyService::characteristicChanged, this,
                             &virtualbike::characteristicChanged);
        } else {
            QObject::connect(service, &QLowEnergyService::characteristicChanged, this,
                             &virtualbike::characteristicChanged);
        }

        bool bluetooth_relaxed = settings.value(QStringLiteral("bluetooth_relaxed"), false).toBool();
        QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
        if (!bluetooth_relaxed) {
            pars.setInterval(100, 100);
        }

        leController->startAdvertising(pars, advertisingData, advertisingData);

        //! [Start Advertising]
    }

    //! [Provide Heartbeat]
    QObject::connect(&bikeTimer, &QTimer::timeout, this, &virtualbike::bikeProvider);
    bikeTimer.start(1s);
    //! [Provide Heartbeat]
    QObject::connect(leController, &QLowEnergyController::disconnected, this, &virtualbike::reconnect);
    QObject::connect(
        leController,
        static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error), this,
        &virtualbike::error);
}

void virtualbike::slopeChanged(int16_t iresistance) {

    QSettings settings;
    bool force_resistance = settings.value(QStringLiteral("virtualbike_forceresistance"), true).toBool();
    bool erg_mode = settings.value(QStringLiteral("zwift_erg"), false).toBool();
    bool zwift_negative_inclination_x2 =
        settings.value(QStringLiteral("zwift_negative_inclination_x2"), false).toBool();
    double offset = settings.value(QStringLiteral("zwift_inclination_offset"), 0.0).toDouble();
    double gain = settings.value(QStringLiteral("zwift_inclination_gain"), 1.0).toDouble();

    qDebug() << QStringLiteral("new requested resistance zwift erg grade ") + QString::number(iresistance) +
                    QStringLiteral(" enabled ") + force_resistance;
    double resistance = ((double)iresistance * 1.5) / 100.0;
    qDebug() << QStringLiteral("calculated erg grade ") + QString::number(resistance);

    if (iresistance >= 0 || !zwift_negative_inclination_x2)
        emit changeInclination(((iresistance / 100.0) * gain) + offset,
                               ((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * gain) + offset);
    else
        emit changeInclination((((iresistance / 100.0) * 2.0) * gain) + offset,
                               (((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * 2.0) * gain) + offset);

    if (force_resistance && !erg_mode) {
        // same on the training program
        Bike->changeResistance((int8_t)(round(resistance * bikeResistanceGain)) + bikeResistanceOffset +
                               1); // resistance start from 1
    }
}

void virtualbike::powerChanged(uint16_t power) { Bike->changePower(power); }

void virtualbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QByteArray reply;
    QSettings settings;
    bool force_resistance = settings.value(QStringLiteral("virtualbike_forceresistance"), true).toBool();
    bool erg_mode = settings.value(QStringLiteral("zwift_erg"), false).toBool();
    bool echelon = settings.value(QStringLiteral("virtual_device_echelon"), false).toBool();
    bool ifit = settings.value(QStringLiteral("virtual_device_ifit"), false).toBool();

    double normalizeWattage = Bike->wattsMetric().value();
    if (normalizeWattage < 0)
        normalizeWattage = 0;

    //    double erg_filter_upper =
    //        settings.value(QStringLiteral("zwift_erg_filter"), 0.0).toDouble(); //
    //        NOTE:clang-analyzer-deadcode.DeadStores
    //    double erg_filter_lower = settings.value(QStringLiteral("zwift_erg_filter_down"), 0.0)
    //                                  .toDouble(); // NOTE:clang-analyzer-deadcode.DeadStores
    qDebug() << QStringLiteral("characteristicChanged ") + QString::number(characteristic.uuid().toUInt16()) +
                    QStringLiteral(" ") + newValue.toHex(' ');

    lastFTMSFrameReceived = QDateTime::currentMSecsSinceEpoch();

    if (!echelon && !ifit)
        emit ftmsCharacteristicChanged(characteristic, newValue);

    switch (characteristic.uuid().toUInt16()) {

    case 0x2AD9: // Fitness Machine Control Point

        if ((char)newValue.at(0) == FTMS_SET_TARGET_RESISTANCE_LEVEL) {

            // Set Target Resistance
            uint8_t uresistance = newValue.at(1);
            uresistance = uresistance / 10;
            if (force_resistance && !erg_mode) {
                Bike->changeResistance(uresistance);
            }
            qDebug() << QStringLiteral("new requested resistance ") + QString::number(uresistance) +
                            QStringLiteral(" enabled ") + force_resistance;
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_RESISTANCE_LEVEL);
            reply.append((quint8)FTMS_SUCCESS);
        } else if ((char)newValue.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) // simulation parameter

        {
            qDebug() << QStringLiteral("indoor bike simulation parameters");
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS);
            reply.append((quint8)FTMS_SUCCESS);

            int16_t iresistance = (((uint8_t)newValue.at(3)) + (newValue.at(4) << 8));
            slopeChanged(iresistance);
        } else if ((char)newValue.at(0) == FTMS_SET_TARGET_POWER) // erg mode

        {
            qDebug() << QStringLiteral("erg mode");
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_POWER);
            reply.append((quint8)FTMS_SUCCESS);

            uint16_t power = (((uint8_t)newValue.at(1)) + (newValue.at(2) << 8));
            powerChanged(power);
        } else if ((char)newValue.at(0) == FTMS_START_RESUME) {
            qDebug() << QStringLiteral("start simulation!");

            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_START_RESUME);
            reply.append((quint8)FTMS_SUCCESS);
        } else if ((char)newValue.at(0) == FTMS_REQUEST_CONTROL) {
            qDebug() << QStringLiteral("control requested");

            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((char)FTMS_REQUEST_CONTROL);
            reply.append((quint8)FTMS_SUCCESS);
        } else {
            qDebug() << QStringLiteral("not supported");

            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)newValue.at(0));
            reply.append((quint8)FTMS_NOT_SUPPORTED);
        }

        QLowEnergyCharacteristic characteristic =
            serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9);
        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }
        writeCharacteristic(serviceFIT, characteristic, reply);
        break;
    }

    //******************** IFIT ******************
    if (characteristic.uuid().toString().contains(QStringLiteral("00001534-1412-efde-1523-785feabcd123"))) {
        QLowEnergyCharacteristic characteristic =
            service->characteristic(QBluetoothUuid(QStringLiteral("00001535-1412-efde-1523-785feabcd123")));

        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }

        QByteArray reply1;
        QByteArray reply2;
        QByteArray reply3;
        QByteArray reply4;
        if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x81) {
            // equipment information
            reply1 = QByteArray::fromHex("fe02210340ff7b81600080dfbf1404fffb4808b7");
            reply2 = QByteArray::fromHex("00120104021d071d810253010300000000000fbc");
            reply3 = QByteArray::fromHex("ff0fbcfdc3fcffca94e707c0c0d118180d000fbc");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x80) {
            reply1 = QByteArray::fromHex("fe021303c3fcffca94e707c0c0d118180d000fbc");
            reply2 = QByteArray::fromHex("00120104020f070f8002094c4745434d4e464f41");
            reply3 = QByteArray::fromHex("ff012d04020f070f8002094c4745434d4e464f41");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x88) {
            reply1 = QByteArray::fromHex("fe021102020f070f8002094c4745434d4e464f41");
            reply2 = QByteArray::fromHex("ff110104020d070d880209829083718984954f41");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x82) {
            reply1 = QByteArray::fromHex("fe022504020d070d880209829083718984954f41");
            reply2 = QByteArray::fromHex("00120104022107218202640001aff900002b5706");
            reply3 = QByteArray::fromHex("01120056002ae8030024f400f401000001020000");
            reply4 = QByteArray::fromHex("ff01bc56002ae8030024f400f401000001020000");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
            writeCharacteristic(service, characteristic, reply4);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x84) {
            reply1 = QByteArray::fromHex("fe022003002ae8030024f400f401000001020000");
            reply2 = QByteArray::fromHex("00120104021c071c8402539600302e312e303631");
            reply3 = QByteArray::fromHex("ff0e32323031372e30393038012a030f2e303631");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x95) {
            reply1 = QByteArray::fromHex("fe021c033031372e30393038012a030f2e303631");
            reply2 = QByteArray::fromHex("00120104021807189502123431373131302d4e4e");
            reply3 = QByteArray::fromHex("ff0a32335a313130313737af31373131302d4e4e");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x5d) {
            reply1 = QByteArray::fromHex("fe02240302060706900208a731373131302d4e4e");
            reply2 = QByteArray::fromHex("0012010402200720020202701750001e00780000");
            reply3 = QByteArray::fromHex("ff12104c2c2c010000000000000000b400000003");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 9 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x00 &&
                   ((uint8_t)newValue.at(9)) == 0xd1) {
            reply1 = QByteArray::fromHex("fe020a025a313130313737af31373131302d4e4e");
            reply2 = QByteArray::fromHex("ff0a010402060706900208a731373131302d4e4e");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 9 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x00 &&
                   ((uint8_t)newValue.at(3)) == 0x80) {
            reply1 = QByteArray::fromHex("fe0209020205070502021000000000b400000003");
            reply2 = QByteArray::fromHex("ff0901040205070502021000000000b400000003");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x3d) {
            reply1 = QByteArray::fromHex("fe0209020205070502021000000000b400000003");
            reply2 = QByteArray::fromHex("ff0901040205070502021000000000b400000003");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x00) {
            reply1 = QByteArray::fromHex("fe02330400caaf020000000000330000df130013");
            reply2 = QByteArray::fromHex("00120104022f072f020200003d00650000003700");
            reply3 = QByteArray::fromHex("01120000000114000000021400f5061400000067");
            reply4 = QByteArray::fromHex("ff0fd702002c013300b4000000000000a1000067");

            reply2[11] = Bike->currentResistance().value();
            reply2[12] = ((uint16_t)normalizeWattage) & 0xFF;
            reply2[13] = (((uint16_t)normalizeWattage) >> 8) & 0xFF;
            reply2[18] = Bike->currentCadence().value();

            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
            writeCharacteristic(service, characteristic, reply4);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(1)) == 0x07 &&
                   ((uint8_t)newValue.at(7)) == 0x10) {
            reply1 = QByteArray::fromHex("fe023304002c012700b400000000000005000085");
            reply2 = QByteArray::fromHex("00120104022f072f020200003900450000003500");
            reply3 = QByteArray::fromHex("01120000ffffffffffffffff00000000020d000d");
            reply4 = QByteArray::fromHex("ff0f000000bac00100000000002e0000aa0d000d");

            reply2[11] = Bike->currentResistance().value();
            reply2[12] = ((uint16_t)normalizeWattage) & 0xFF;
            reply2[13] = (((uint16_t)normalizeWattage) >> 8) & 0xFF;
            reply2[18] = Bike->currentCadence().value();

            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
            writeCharacteristic(service, characteristic, reply4);
        }
    }

    //******************** ECHELON ***************
    if (characteristic.uuid().toString().contains(QStringLiteral("0bf669f2-45f2-11e7-9598-0800200c9a66"))) {
        QLowEnergyCharacteristic characteristic =
            service->characteristic(QBluetoothUuid(QStringLiteral("0bf669f3-45f2-11e7-9598-0800200c9a66")));
        QLowEnergyCharacteristic characteristic2 =
            service->characteristic(QBluetoothUuid(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66")));

        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }

        QByteArray reply;
        if (((uint8_t)newValue.at(1)) == 0xA1) {

            // f0 a1 06 01 0b 00 33 06 03 df
            reply.append(0xf0);
            reply.append(0xa1);
            reply.append(0x06);
            reply.append((char)0x01);
            reply.append(0x0b);
            reply.append((char)0x00);
            reply.append(0x33);
            reply.append(0x06);
            reply.append(0x03);
            reply.append(0xdf);
            writeCharacteristic(service, characteristic, reply);
            echelonInitDone = true;
        } else if (((uint8_t)newValue.at(1)) == 0xA3) {

            // f0 a3 02 20 01 b6
            reply.append(0xf0);
            reply.append(0xa3);
            reply.append(0x02);
            reply.append(0x20);
            reply.append(0x01);
            reply.append(0xb6);
            writeCharacteristic(service, characteristic, reply);
        }
        // f0 b0 01 00 a1
        else if (((uint8_t)newValue.at(1)) == 0xB0 && ((uint8_t)newValue.at(3)) == 0x00) {

            // f0 d0 01 00 c1
            reply.append(0xf0);
            reply.append(0xd0);
            reply.append(0x01);
            reply.append((char)0x00);
            reply.append(0xc1);
            writeCharacteristic(service, characteristic, reply);
        }
        // f0 b0 01 01 a2
        else if (((uint8_t)newValue.at(1)) == 0xB0) {

            // f0 d0 01 01 c2
            reply.append(0xf0);
            reply.append(0xd0);
            reply.append(0x01);
            reply.append(0x01);
            reply.append(0xc2);
            writeCharacteristic(service, characteristic2, reply);
            echelonWriteResistance();
        } else if (((uint8_t)newValue.at(1)) == 0xA0) {

            reply = newValue;
            writeCharacteristic(service, characteristic, reply);
        }
    }
}

void virtualbike::writeCharacteristic(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic,
                                      const QByteArray &value) {
    try {
        qDebug() << QStringLiteral("virtualbike::writeCharacteristic ") + service->serviceName() + QStringLiteral(" ") +
                        characteristic.name() + QStringLiteral(" ") + value.toHex(' ');
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    } catch (...) {
        qDebug() << QStringLiteral("virtual bike error!");
    }
}

void virtualbike::reconnect() {

    QSettings settings;
    bool bluetooth_relaxed = settings.value(QStringLiteral("bluetooth_relaxed"), false).toBool();

    if (bluetooth_relaxed) {
        return;
    }

    bool cadence = settings.value(QStringLiteral("bike_cadence_sensor"), false).toBool();
    bool battery = settings.value(QStringLiteral("battery_service"), false).toBool();
    bool power = settings.value(QStringLiteral("bike_power_sensor"), false).toBool();
    bool service_changed = settings.value(QStringLiteral("service_changed"), false).toBool();
    bool heart_only = settings.value(QStringLiteral("virtual_device_onlyheart"), false).toBool();
    bool echelon = settings.value(QStringLiteral("virtual_device_echelon"), false).toBool();
    bool ifit = settings.value(QStringLiteral("virtual_device_ifit"), false).toBool();

    qDebug() << QStringLiteral("virtualbike::reconnect");
    leController->disconnectFromDevice();

#ifndef Q_OS_IOS
    if (service_changed) {
        serviceChanged = leController->addService(serviceDataChanged);
    }

    if (!echelon && !ifit) {
        if (!heart_only) {
            if (!cadence && !power) {

                serviceFIT = leController->addService(serviceDataFIT);
            } else {
                service = leController->addService(serviceData);
            }
        }
    } else if (ifit) {
        service = leController->addService(serviceData);
    } else {

        service = leController->addService(serviceEchelon);
        service = leController->addService(serviceData);
    }

    if (battery)
        serviceBattery = leController->addService(serviceDataBattery);

    if (!this->noHeartService || heart_only)
        serviceHR = leController->addService(serviceDataHR);
#endif

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);
    leController->startAdvertising(pars, advertisingData, advertisingData);
}

void virtualbike::bikeProvider() {

    QSettings settings;
    bool cadence = settings.value(QStringLiteral("bike_cadence_sensor"), false).toBool();
    bool battery = settings.value(QStringLiteral("battery_service"), false).toBool();
    bool power = settings.value(QStringLiteral("bike_power_sensor"), false).toBool();
    bool bike_wheel_revs = settings.value(QStringLiteral("bike_wheel_revs"), false).toBool();
    bool heart_only = settings.value(QStringLiteral("virtual_device_onlyheart"), false).toBool();
    bool echelon = settings.value(QStringLiteral("virtual_device_echelon"), false).toBool();
    bool ifit = settings.value(QStringLiteral("virtual_device_ifit"), false).toBool();
    bool erg_mode = settings.value(QStringLiteral("zwift_erg"), false).toBool();

    double normalizeWattage = Bike->wattsMetric().value();
    if (normalizeWattage < 0)
        normalizeWattage = 0;

    uint16_t normalizeSpeed = (uint16_t)qRound(Bike->currentSpeed().value() * 100);

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    if (h) {
        // really connected to a device
        if (h->virtualbike_updateFTMS(normalizeSpeed, (char)Bike->currentResistance().value(),
                                      (uint16_t)Bike->currentCadence().value() * 2, (uint16_t)normalizeWattage)) {
            h->virtualbike_setHeartRate(Bike->currentHeart().value());
            if (!erg_mode)
                slopeChanged(h->virtualbike_getCurrentSlope());
            else {
                qDebug() << "ios workaround power changed request" << h->virtualbike_getPowerRequested();
                powerChanged(h->virtualbike_getPowerRequested());
            }
            uint8_t ftms_message[255];
            int ret = h->virtualbike_getLastFTMSMessage(ftms_message);
            if (ret > 0) {
                lastFTMSFrameReceived = QDateTime::currentMSecsSinceEpoch();
                emit ftmsCharacteristicChanged(QLowEnergyCharacteristic(),
                                               QByteArray::fromRawData((char *)ftms_message, ret));
            }
        }
        return;
    }
#endif
#else
    Q_UNUSED(erg_mode);
#endif

    if (leController->state() != QLowEnergyController::ConnectedState) {
        qDebug() << QStringLiteral("virtual bike not connected");

        return;
    } else {
        bool bluetooth_relaxed = settings.value(QStringLiteral("bluetooth_relaxed"), false).toBool();
        bool bluetooth_30m_hangs = settings.value(QStringLiteral("bluetooth_30m_hangs"), false).toBool();
        if (bluetooth_relaxed) {

            leController->stopAdvertising();
        }

        if (lastFTMSFrameReceived > 0 && QDateTime::currentMSecsSinceEpoch() > (lastFTMSFrameReceived + 5000) &&
            bluetooth_30m_hangs) {
            lastFTMSFrameReceived = 0;
            qDebug() << QStringLiteral("virtual bike timeout, reconnecting...");

            reconnect();
            return;
        }

        qDebug() << QStringLiteral("virtual bike connected");
    }

    QByteArray value;

    qDebug() << QStringLiteral("bikeProvider") << lastFTMSFrameReceived
             << (qint64)(lastFTMSFrameReceived + ((qint64)2000)) << erg_mode;
    // zwift with the last update, seems to sending power request only when it actually wants to change it
    // so i need to keep this on to the bike
    if (lastFTMSFrameReceived > 0 &&
        (QDateTime::currentMSecsSinceEpoch() > (qint64)(lastFTMSFrameReceived + ((qint64)2000))) && erg_mode) {
        qDebug() << QStringLiteral("zwift is not sending the power anymore, let's continue with the last value");
        powerChanged(((bike *)Bike)->lastRequestedPower().value());
    }

    if (!echelon && !ifit) {
        if (!heart_only) {
            if (!cadence && !power) {

                value.append((char)0x64); // speed, inst. cadence, resistance lvl, instant power
                value.append((char)0x02); // heart rate

                value.append((char)(normalizeSpeed & 0xFF));      // speed
                value.append((char)(normalizeSpeed >> 8) & 0xFF); // speed

                value.append((char)((uint16_t)(Bike->currentCadence().value() * 2) & 0xFF));        // cadence
                value.append((char)(((uint16_t)(Bike->currentCadence().value() * 2) >> 8) & 0xFF)); // cadence

                value.append((char)Bike->currentResistance().value()); // resistance
                value.append((char)(0));                               // resistance

                value.append((char)(((uint16_t)normalizeWattage) & 0xFF));      // watts
                value.append((char)(((uint16_t)normalizeWattage) >> 8) & 0xFF); // watts

                value.append(char(Bike->currentHeart().value())); // Actual value.
                value.append((char)0);                            // Bkool FTMS protocol HRM offset 1280 fix

                if (!serviceFIT) {
                    qDebug() << QStringLiteral("serviceFIT not available");

                    return;
                }

                QLowEnergyCharacteristic characteristic =
                    serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
                Q_ASSERT(characteristic.isValid());
                if (leController->state() != QLowEnergyController::ConnectedState) {
                    qDebug() << QStringLiteral("virtual bike not connected");

                    return;
                }
                writeCharacteristic(serviceFIT, characteristic, value);
            } else if (power) {

                value.append((char)0x20); // crank data present
                value.append((char)0x00);
                value.append((char)(((uint16_t)normalizeWattage) & 0xFF));                     // watt
                value.append((char)(((uint16_t)normalizeWattage) >> 8) & 0xFF);                // watt
                value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF));      // revs count
                value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count
                value.append((char)(Bike->lastCrankEventTime() & 0xff));                       // eventtime
                value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF);                  // eventtime

                if (!service) {
                    qDebug() << QStringLiteral("service not available");

                    return;
                }

                QLowEnergyCharacteristic characteristic =
                    service->characteristic(QBluetoothUuid::CharacteristicType::CyclingPowerMeasurement);
                Q_ASSERT(characteristic.isValid());
                if (leController->state() != QLowEnergyController::ConnectedState) {
                    qDebug() << QStringLiteral("virtual bike not connected");

                    return;
                }
                writeCharacteristic(service, characteristic, value);
            } else {
                if (!bike_wheel_revs) {

                    value.append((char)0x02); // crank data present
                } else {

                    value.append((char)0x03); // crank and wheel data present

                    if (Bike->currentSpeed().value()) {

                        const double wheelCircumference = 2000.0; // millimeters
                        wheelRevs++;
                        lastWheelTime +=
                            (uint16_t)(1024.0 / ((Bike->currentSpeed().value() / 3.6) / (wheelCircumference / 1000.0)));
                    }
                    value.append((char)((wheelRevs & 0xFF)));        // wheel count
                    value.append((char)((wheelRevs >> 8) & 0xFF));   // wheel count
                    value.append((char)((wheelRevs >> 16) & 0xFF));  // wheel count
                    value.append((char)((wheelRevs >> 24) & 0xFF));  // wheel count
                    value.append((char)(lastWheelTime & 0xff));      // eventtime
                    value.append((char)(lastWheelTime >> 8) & 0xFF); // eventtime
                }
                value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF));      // revs count
                value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count
                value.append((char)(Bike->lastCrankEventTime() & 0xff));                       // eventtime
                value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF);                  // eventtime

                if (!service) {
                    qDebug() << QStringLiteral("service not available");

                    return;
                }

                QLowEnergyCharacteristic characteristic =
                    service->characteristic(QBluetoothUuid::CharacteristicType::CSCMeasurement);
                Q_ASSERT(characteristic.isValid());
                if (leController->state() != QLowEnergyController::ConnectedState) {
                    qDebug() << QStringLiteral("virtual bike not connected");

                    return;
                }
                writeCharacteristic(service, characteristic, value);
            }
        }
    } else if (ifit) {

    } else {

        if (echelonInitDone) {
            // TODO: set it do dynamic
            // f0 d1 09 00 00 00 00 00 01 00 5f 00 2a
            value.append(0xf0);
            value.append(0xd1);
            value.append(0x09);
            value.append((char)0x00);                                            // elapsed
            value.append((char)0x00);                                            // elapsed
            value.append((uint8_t)(((uint32_t)(Bike->odometer() * 100)) >> 24)); // distance
            value.append((uint8_t)(((uint32_t)(Bike->odometer() * 100)) >> 16)); // distance
            value.append((uint8_t)(((uint32_t)(Bike->odometer() * 100)) >> 8));  // distance
            value.append((uint8_t)(Bike->odometer() * 100));                     // distance
            value.append((char)0x00);
            value.append(Bike->currentCadence().value());
            value.append((uint8_t)Bike->currentHeart().value());

            uint8_t sum = 0;
            for (uint8_t i = 0; i < value.length(); i++) {

                sum += value[i]; // the last byte is a sort of a checksum
            }
            value.append(sum);

            if (!service) {
                qDebug() << QStringLiteral("service not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66")));
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual bike not connected");

                return;
            }
            writeCharacteristic(service, characteristic, value);

            echelonWriteResistance();
        }
    }
    // characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    // Q_ASSERT(characteristic.isValid());
    // service->readCharacteristic(characteristic);

    if (battery) {
        if (!serviceBattery) {
            qDebug() << QStringLiteral("serviceBattery not available");

            return;
        }

        QByteArray valueBattery;
        valueBattery.append(100); // Actual value.
        QLowEnergyCharacteristic characteristicBattery = serviceBattery->characteristic(QBluetoothUuid::BatteryLevel);

        Q_ASSERT(characteristicBattery.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }
        writeCharacteristic(serviceBattery, characteristicBattery, valueBattery);
    }

    if (!this->noHeartService || heart_only) {
        if (!serviceHR) {
            qDebug() << QStringLiteral("serviceHR not available");

            return;
        }

        QByteArray valueHR;
        valueHR.append(char(0));                                  // Flags that specify the format of the value.
        valueHR.append(char(Bike->metrics_override_heartrate())); // Actual value.
        QLowEnergyCharacteristic characteristicHR = serviceHR->characteristic(QBluetoothUuid::HeartRateMeasurement);

        Q_ASSERT(characteristicHR.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }
        writeCharacteristic(serviceHR, characteristicHR, valueHR);
    }
}

void virtualbike::echelonWriteResistance() {

    QSettings settings;
    double bikeResistanceOffset = settings.value(QStringLiteral("echelon_resistance_offset"), 0).toInt();
    double bikeResistanceGain = settings.value(QStringLiteral("echelon_resistance_gain"), 1).toDouble();
    double CurrentResistance = (Bike->currentResistance().value() * bikeResistanceGain) + bikeResistanceOffset;

    // resistance change notification
    // f0 d2 01 0b ce
    QByteArray resistance;
    static uint8_t oldresistance = 255;
    resistance.append(0xf0);
    resistance.append(0xd2);
    resistance.append(0x01);
    resistance.append(CurrentResistance);

    uint8_t sum = 0;
    for (uint8_t i = 0; i < resistance.length(); i++) {

        sum += resistance[i]; // the last byte is a sort of a checksum
    }
    resistance.append(sum);
    if (oldresistance != ((uint8_t)Bike->currentResistance().value())) {
        QLowEnergyCharacteristic characteristic =
            service->characteristic(QBluetoothUuid(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66")));
        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }

        writeCharacteristic(service, characteristic, resistance);
    }
    oldresistance = ((uint8_t)CurrentResistance);
}

bool virtualbike::connected() {
    if (!leController) {

        return false;
    }
    return leController->state() == QLowEnergyController::ConnectedState;
}

void virtualbike::error(QLowEnergyController::Error newError) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("virtualbike::controller:ERROR ") +
                    QString::fromLocal8Bit(metaEnum.valueToKey(newError));

    if (newError != QLowEnergyController::RemoteHostClosedError) {
        reconnect();
    }
}
