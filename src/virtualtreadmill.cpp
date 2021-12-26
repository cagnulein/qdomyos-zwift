#include "virtualtreadmill.h"
#include "elliptical.h"
#include "ftmsbike.h"
#include <QSettings>
#include <QtMath>
#include <chrono>

using namespace std::chrono_literals;

virtualtreadmill::virtualtreadmill(bluetoothdevice *t, bool noHeartService) {
    QSettings settings;
    treadMill = t;
    this->noHeartService = noHeartService;
    if (settings.value("dircon_yes", false).toBool()) {
        dirconManager = new DirconManager(t, 0, 0, this);
        connect(dirconManager, SIGNAL(changeInclination(double, double)), this,
                SIGNAL(changeInclination(double, double)));
        connect(dirconManager, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    }
    if (!settings.value("virtual_device_bluetooth", true).toBool())
        return;
    notif2AD2 = new CharacteristicNotifier2AD2(t, this);
    notif2ACD = new CharacteristicNotifier2ACD(t, this);
    notif2A53 = new CharacteristicNotifier2A53(t, this);
    notif2A37 = new CharacteristicNotifier2A37(t, this);
    writeP2AD9 = new CharacteristicWriteProcessor2AD9(0, 0, t, this);
    connect(writeP2AD9, SIGNAL(changeInclination(grade, perc)), this, SIGNAL(changeInclination(grade, perc)));

    bool cadence = settings.value(QStringLiteral("run_cadence_sensor"), false).toBool();

    //! [Advertising Data]
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName(QStringLiteral("DomyosBridge"));
    QList<QBluetoothUuid> services;
    if (!cadence) {
        services << ((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
    } else {
        services << (QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
    }

    if (noHeartService == false) {
        services << QBluetoothUuid::HeartRate;
    }

    services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);

    advertisingData.setServices(services);
    //! [Advertising Data]

    //! [Service Data]
    if (!cadence) {
        QLowEnergyCharacteristicData charData;
        charData.setUuid((QBluetoothUuid::CharacteristicType)0x2ACC); // FitnessMachineFeatureCharacteristicUuid
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
        charData2.setUuid((QBluetoothUuid::CharacteristicType)0x2ACD); // TreadmillDataCharacteristicUuid
        charData2.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor);
        charData2.addDescriptor(clientConfig2);

        QLowEnergyCharacteristicData charData3;
        charData3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
        charData3.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
        const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                      QByteArray(2, 0));
        charData3.addDescriptor(cpClientConfig);

        QLowEnergyCharacteristicData charDataFIT5;
        charDataFIT5.setUuid((QBluetoothUuid::CharacteristicType)0x2ADA); // Fitness Machine status
        charDataFIT5.setProperties(QLowEnergyCharacteristic::Notify);
        QByteArray descriptor5;
        descriptor5.append((char)0x01);
        descriptor5.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig5(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor5);
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
        const QLowEnergyDescriptorData clientConfig6(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor6);
        charDataFIT6.addDescriptor(clientConfig6);
        charDataFIT6.setProperties(QLowEnergyCharacteristic::Read);

        // indoor bike in order to exploit a zwift bug that allows treadmill to get the incline values
        QLowEnergyCharacteristicData charDataFIT7;
        charDataFIT7.setUuid((QBluetoothUuid::CharacteristicType)0x2AD2); // indoor bike
        charDataFIT7.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
        QByteArray descriptor7;
        descriptor7.append((char)0x01);
        descriptor7.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig7(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor7);
        charDataFIT7.addDescriptor(clientConfig7);

        QLowEnergyCharacteristicData charDataFIT2;
        charDataFIT2.setUuid(
            (QBluetoothUuid::CharacteristicType)0x2AD6); // supported_resistance_level_rangeCharacteristicUuid
        charDataFIT2.setProperties(QLowEnergyCharacteristic::Read);
        QByteArray valueFIT2;
        valueFIT2.append((char)0x0A); // min resistance value
        valueFIT2.append((char)0x00); // min resistance value
        valueFIT2.append((char)0x96); // max resistance value
        valueFIT2.append((char)0x00); // max resistance value
        valueFIT2.append((char)0x0A); // step resistance
        valueFIT2.append((char)0x00); // step resistance
        charDataFIT2.setValue(valueFIT2);

        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
        serviceData.addCharacteristic(charData);
        serviceData.addCharacteristic(charData2);
        serviceData.addCharacteristic(charData3);
        serviceData.addCharacteristic(charDataFIT5);
        serviceData.addCharacteristic(charDataFIT6);
        serviceData.addCharacteristic(charDataFIT7);
        serviceData.addCharacteristic(charDataFIT2);
    } else {
        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid::CharacteristicType::RSCFeature);
        charData.setProperties(QLowEnergyCharacteristic::Read);
        QByteArray value;
        value.append((char)0x02); // Total Distance Measurement Supported
        value.append((char)0x00);
        charData.setValue(value);

        QLowEnergyCharacteristicData charData2;
        charData2.setUuid(QBluetoothUuid::CharacteristicType::SensorLocation);
        charData2.setProperties(QLowEnergyCharacteristic::Read);
        QByteArray valueLocaltion;
        valueLocaltion.append((char)1); // Top of shoe
        charData2.setValue(valueLocaltion);
        /*const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                    QByteArray(2, 0));
        charData2.addDescriptor(clientConfig2);*/

        QLowEnergyCharacteristicData charData3;
        charData3.setUuid(QBluetoothUuid::CharacteristicType::RSCMeasurement);
        charData3.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify);
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor);
        charData3.addDescriptor(clientConfig4);

        QLowEnergyCharacteristicData charData4;
        charData4.setUuid(QBluetoothUuid::CharacteristicType::SCControlPoint);
        charData4.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
        const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                      QByteArray(2, 0));
        charData4.addDescriptor(cpClientConfig);

        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
        serviceData.addCharacteristic(charData);
        serviceData.addCharacteristic(charData3);
        serviceData.addCharacteristic(charData2);
        serviceData.addCharacteristic(charData4);
    }
    //! [Service Data]

    if (noHeartService == false) {
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
    if (noHeartService == false) {
        serviceHR = leController->addService(serviceDataHR);
    }

    QObject::connect(service, &QLowEnergyService::characteristicChanged, this,
                     &virtualtreadmill::characteristicChanged);

    bool bluetooth_relaxed = settings.value(QStringLiteral("bluetooth_relaxed"), false).toBool();
    QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
    if (!bluetooth_relaxed) {
        pars.setInterval(100, 100);
    }

    leController->startAdvertising(pars, advertisingData, advertisingData);
    //! [Start Advertising]

    //! [Provide Heartbeat]
    QObject::connect(&treadmillTimer, &QTimer::timeout, this, &virtualtreadmill::treadmillProvider);
    treadmillTimer.start(1s);
    //! [Provide Heartbeat]
    QObject::connect(leController, &QLowEnergyController::disconnected, this, &virtualtreadmill::reconnect);
}

void virtualtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicChanged ") + QString::number(characteristic.uuid().toUInt16()) +
                    QStringLiteral(" ") + newValue;
    QByteArray reply;

    switch (characteristic.uuid().toUInt16()) {
    case 0x2AD9: // Fitness Machine Control Point
        if (writeP2AD9->writeProcess(0x2AD9, newValue, reply) == CP_OK) {

            QLowEnergyCharacteristic characteristic =
                service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                qDebug() << QStringLiteral("virtualtreadmill::writeCharacteristic ") + service->serviceName() +
                                QStringLiteral(" ") + characteristic.name() + QStringLiteral(" ") + reply.toHex(' ');
                service->writeCharacteristic(characteristic, reply); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtual treadmill error!");
            }
        }
        break;
    }
}

void virtualtreadmill::slopeChanged(int16_t iresistance) {

    QSettings settings;
    qDebug() << QStringLiteral("new requested resistance zwift erg grade ") + QString::number(iresistance);
    double resistance = ((double)iresistance * 1.5) / 100.0;
    qDebug() << QStringLiteral("calculated erg grade ") + QString::number(resistance);

    emit changeInclination(iresistance / 100.0, qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0);
}

void virtualtreadmill::reconnect() {
    QSettings settings;
    bool bluetooth_relaxed = settings.value(QStringLiteral("bluetooth_relaxed"), false).toBool();

    if (bluetooth_relaxed) {
        return;
    }

    qDebug() << QStringLiteral("virtualtreadmill reconnect ") << treadMill->connected();
    service = leController->addService(serviceData);

    if (noHeartService == false) {
        serviceHR = leController->addService(serviceDataHR);
    }

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);

    if (service) {
        leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData, advertisingData);
    }
}

void virtualtreadmill::treadmillProvider() {
    QSettings settings;
    bool cadence = settings.value(QStringLiteral("run_cadence_sensor"), false).toBool();

    if (leController->state() != QLowEnergyController::ConnectedState) {
        qDebug() << QStringLiteral("virtualtreadmill connection error");
        return;
    } else {
        QSettings settings;
        bool bluetooth_relaxed = settings.value(QStringLiteral("bluetooth_relaxed"), false).toBool();
        if (bluetooth_relaxed) {
            leController->stopAdvertising();
        }
    }

    QByteArray value;

    if (!cadence) {
        if (notif2ACD->notify(value) == CN_OK) {
            if (!service) {
                qDebug() << QStringLiteral("service not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                service->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                service->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
        if (notif2AD2->notify(value) == CN_OK) {
            if (!service) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                service->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    } else {
        if (notif2A53->notify(value) == CN_OK) {
            if (!service) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid::CharacteristicType::RSCMeasurement);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                service->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }

    // characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    // Q_ASSERT(characteristic.isValid());
    // service->readCharacteristic(characteristic);

    if (noHeartService == false) {
        if (notif2A53->notify(value) == CN_OK) {
            if (!serviceHR) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceHR->characteristic(QBluetoothUuid::CharacteristicType::RSCMeasurement);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceHR->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }
}

bool virtualtreadmill::connected() {
    if (!leController) {
        return false;
    }
    return leController->state() == QLowEnergyController::ConnectedState;
}
