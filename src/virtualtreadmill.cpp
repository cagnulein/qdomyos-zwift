#include "virtualtreadmill.h"
#include <QSettings>
#include <QtMath>
#include <chrono>

using namespace std::chrono_literals;

virtualtreadmill::virtualtreadmill(bluetoothdevice *t, bool noHeartService) {
    QSettings settings;
    treadMill = t;
    this->noHeartService = noHeartService;

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
        charData3.setProperties(QLowEnergyCharacteristic::Write);

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

        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
        serviceData.addCharacteristic(charData);
        serviceData.addCharacteristic(charData2);
        serviceData.addCharacteristic(charData3);
        serviceData.addCharacteristic(charDataFIT5);
        serviceData.addCharacteristic(charDataFIT6);
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
    emit debug(QStringLiteral("characteristicChanged ") + QString::number(characteristic.uuid().toUInt16()) +
               QStringLiteral(" ") + newValue);

    char a;
    char b;

    switch (characteristic.uuid().toUInt16()) {
    case 0x2AD9: // Fitness Machine Control Point
        if ((char)newValue.at(0) == 0x02) {
            // Set Target Speed
            a = newValue.at(1);
            b = newValue.at(2);

            uint16_t uspeed = a + (((uint16_t)b) << 8);
            double requestSpeed = (double)uspeed / 100.0;
            if (treadMill->deviceType() == bluetoothdevice::TREADMILL) {
                ((treadmill *)treadMill)->changeSpeed(requestSpeed);
            }
            emit debug(QStringLiteral("new requested speed ") + QString::number(requestSpeed));
        } else if ((char)newValue.at(0) == 0x03) // Set Target Inclination
        {
            a = newValue.at(1);
            b = newValue.at(2);

            int16_t sincline = a + (((int16_t)b) << 8);
            double requestIncline = (double)sincline / 10.0;
            if (requestIncline < 0)
                requestIncline = 0;
            // Resistance as incline on Sole E95s Elliptical #419
            if (treadMill->deviceType() == bluetoothdevice::TREADMILL ||
                treadMill->deviceType() == bluetoothdevice::ELLIPTICAL)
                ((treadmill *)treadMill)->changeInclination(requestIncline);
            emit debug("new requested incline " + QString::number(requestIncline));
        } else if ((char)newValue.at(0) == 0x07) // Start request
        {
            treadMill->start();
            emit debug(QStringLiteral("request to start"));
        } else if ((char)newValue.at(0) == 0x08) // Stop request
        {
            treadMill->stop();
            emit debug(QStringLiteral("request to stop"));
        }
        break;
    }
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
        emit debug(QStringLiteral("virtualtreadmill connection error"));
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
        value.append(0x08);       // Inclination avaiable
        value.append((char)0x01); // heart rate avaiable

        uint16_t normalizeSpeed = (uint16_t)qRound(treadMill->currentSpeed().value() * 100);
        char a = (normalizeSpeed >> 8) & 0XFF;
        char b = normalizeSpeed & 0XFF;
        QByteArray speedBytes;
        speedBytes.append(b);
        speedBytes.append(a);
        uint16_t normalizeIncline = 0;
        if (treadMill->deviceType() == bluetoothdevice::TREADMILL)
            normalizeIncline = (uint32_t)qRound(((treadmill *)treadMill)->currentInclination().value() * 10);
        a = (normalizeIncline >> 8) & 0XFF;
        b = normalizeIncline & 0XFF;
        QByteArray inclineBytes;
        inclineBytes.append(b);
        inclineBytes.append(a);
        double ramp = 0;
        if (treadMill->deviceType() == bluetoothdevice::TREADMILL)
            ramp = qRadiansToDegrees(qAtan(((treadmill *)treadMill)->currentInclination().value() / 100));
        int16_t normalizeRamp = (int32_t)qRound(ramp * 10);
        a = (normalizeRamp >> 8) & 0XFF;
        b = normalizeRamp & 0XFF;
        QByteArray rampBytes;
        rampBytes.append(b);
        rampBytes.append(a);

        value.append(speedBytes); // Actual value.

        value.append(inclineBytes); // incline

        value.append(rampBytes); // ramp angle

        value.append(treadMill->currentHeart().value()); // current heart rate

        if (!service) {
            qDebug() << QStringLiteral("service not available");
            return;
        }

        QLowEnergyCharacteristic characteristic =
            service->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD); // TreadmillDataCharacteristicUuid
        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            emit debug(QStringLiteral("virtualtreadmill connection error"));
            return;
        }
        try {
            service->writeCharacteristic(characteristic, value); // Potentially causes notification.
        } catch (...) {
            emit debug(QStringLiteral("virtualtreadmill error!"));
        }
    } else {
        if (!service) {
            qDebug() << QStringLiteral("service not available");
            return;
        }

        value.append(0x02); // total distance
        uint16_t speed = treadMill->currentSpeed().value() / 3.6;
        uint32_t distance = treadMill->odometer() * 1000.0;
        value.append((char)((speed & 0xFF)));
        value.append((char)((speed >> 8) & 0xFF));
        value.append((char)(90)); // cadence, i don't have any sensor to measure it
        value.append((char)((distance & 0xFF)));
        value.append((char)((distance >> 8) & 0xFF));
        value.append((char)((distance >> 16) & 0xFF));
        value.append((char)((distance >> 24) & 0xFF));

        QLowEnergyCharacteristic characteristic =
            service->characteristic(QBluetoothUuid::CharacteristicType::RSCMeasurement);
        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            emit debug(QStringLiteral("virtual bike not connected"));
            return;
        }
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    }

    // characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    // Q_ASSERT(characteristic.isValid());
    // service->readCharacteristic(characteristic);

    if (noHeartService == false) {
        if (!serviceHR) {
            qDebug() << QStringLiteral("serviceHR not available");
            return;
        }

        QByteArray valueHR;
        valueHR.append(char(0));                                 // Flags that specify the format of the value.
        valueHR.append(char(treadMill->currentHeart().value())); // Actual value.
        QLowEnergyCharacteristic characteristicHR = serviceHR->characteristic(QBluetoothUuid::HeartRateMeasurement);
        Q_ASSERT(characteristicHR.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            emit debug(QStringLiteral("virtualtreadmill connection error"));
            return;
        }
        try {
            serviceHR->writeCharacteristic(characteristicHR, valueHR); // Potentially causes notification.
        } catch (...) {
            emit debug(QStringLiteral("virtualtreadmill error!"));
        }
    }
}

bool virtualtreadmill::connected() {
    if (!leController) {
        return false;
    }
    return leController->state() == QLowEnergyController::ConnectedState;
}
