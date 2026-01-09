#include "virtualdevices/virtualrower.h"
#include "qsettings.h"
#include "rower.h"
#include "ftmsbike/ftmsbike.h"

#include <QDataStream>
#include <QMetaEnum>
#include <QSettings>
#include <QtMath>
#include <chrono>
#include <QThread>

#ifdef Q_OS_ANDROID
#include "androidactivityresultreceiver.h"
#include "keepawakehelper.h"
#include <QJniObject>
#endif

using namespace std::chrono_literals;

virtualrower::virtualrower(bluetoothdevice *t, bool noWriteResistance, bool noHeartService) {
    Rower = t;

    this->noHeartService = noHeartService;

    QSettings settings;
    bool heart_only =
        settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();

    Q_UNUSED(noWriteResistance)

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && !heart_only) {

        qDebug() << "ios_zwift_workaround activated!";
        h = new lockscreen();
        h->virtualrower_ios();
    } else

#endif
#endif
    {
        //! [Advertising Data]
        advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
        advertisingData.setIncludePowerLevel(true);
        advertisingData.setLocalName(QStringLiteral("PM5"));
        QList<QBluetoothUuid> services;

        if (!heart_only) {
            services << ((QBluetoothUuid::ServiceClassUuid)0x1826);
        }
        if (!this->noHeartService || heart_only) {
            services << QBluetoothUuid::ServiceClassUuid::HeartRate;
        }

        services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);

        advertisingData.setServices(services);
        //! [Advertising Data]

        if (!heart_only) {

            serviceDataFIT.setType(QLowEnergyServiceData::ServiceTypePrimary);
            QLowEnergyCharacteristicData charDataFIT;
            charDataFIT.setUuid((QBluetoothUuid::CharacteristicType)0x2ACC); // FitnessMachineFeatureCharacteristicUuid
            QByteArray valueFIT;

            valueFIT.append((char)0x83);
            valueFIT.append((char)0x14);
            valueFIT.append((char)0x00);
            valueFIT.append((char)0x00);
            valueFIT.append((char)0x0C);
            valueFIT.append((char)0xe0);
            valueFIT.append((char)0x00);
            valueFIT.append((char)0x00);
            charDataFIT.setValue(valueFIT);
            charDataFIT.setProperties(QLowEnergyCharacteristic::Read);

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

            QLowEnergyCharacteristicData charDataFIT3;
            charDataFIT3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
            charDataFIT3.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
            const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                          QByteArray(2, 0));
            charDataFIT3.addDescriptor(cpClientConfig);

            QLowEnergyCharacteristicData charDataFIT4;
            charDataFIT4.setUuid((QBluetoothUuid::CharacteristicType)0x2AD1); // rower
            charDataFIT4.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, descriptor);
            charDataFIT4.addDescriptor(clientConfig4);

            QLowEnergyCharacteristicData charDataFIT5;
            charDataFIT5.setUuid((QBluetoothUuid::CharacteristicType)0x2ADA); // Fitness Machine status
            charDataFIT5.setProperties(QLowEnergyCharacteristic::Notify);
            QByteArray descriptor5;
            descriptor5.append((char)0x01);
            descriptor5.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig5(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
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
            const QLowEnergyDescriptorData clientConfig6(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
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
        }

        if (!this->noHeartService || heart_only) {

            QLowEnergyCharacteristicData charDataHR;
            charDataHR.setUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
            charDataHR.setValue(QByteArray(2, 0));
            charDataHR.setProperties(QLowEnergyCharacteristic::Notify);
            const QLowEnergyDescriptorData clientConfigHR(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                          QByteArray(2, 0));
            charDataHR.addDescriptor(clientConfigHR);

            serviceDataHR.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceDataHR.setUuid(QBluetoothUuid::ServiceClassUuid::HeartRate);
            serviceDataHR.addCharacteristic(charDataHR);
        }

        //! [Start Advertising]
        leController = QLowEnergyController::createPeripheral();
        Q_ASSERT(leController);

        serviceFIT = leController->addService(serviceDataFIT);
        QThread::msleep(100); // give time to Android to add the service async.ly

        if (!this->noHeartService || heart_only) {
            serviceHR = leController->addService(serviceDataHR);
        }

        QObject::connect(serviceFIT, &QLowEnergyService::characteristicChanged, this,
                         &virtualrower::characteristicChanged);

        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
        if (!bluetooth_relaxed) {
            pars.setInterval(100, 100);
        }

#ifdef Q_OS_ANDROID
        QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/BleAdvertiser", "startAdvertisingRower",
                                                  "(Landroid/content/Context;)V", QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;").object());
#else
        leController->startAdvertising(pars, advertisingData, advertisingData);
#endif

        //! [Start Advertising]
    }

    //! [Provide Heartbeat]
    QObject::connect(&rowerTimer, &QTimer::timeout, this, &virtualrower::rowerProvider);
    if (settings.value(QZSettings::race_mode, QZSettings::default_race_mode).toBool())
        rowerTimer.start(100ms);
    else
        rowerTimer.start(1s);

    //! [Provide Heartbeat]
    QObject::connect(leController, &QLowEnergyController::disconnected, this, &virtualrower::reconnect);
    QObject::connect(
        leController,
        &QLowEnergyController::errorOccurred, this,
        &virtualrower::error);
}

void virtualrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QByteArray reply;
    QSettings settings;
    bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
    qDebug() << QStringLiteral("characteristicChanged ") + QString::number(characteristic.uuid().toUInt16()) +
                    QStringLiteral(" ") + newValue.toHex(' ');

    lastFTMSFrameReceived = QDateTime::currentMSecsSinceEpoch();

    switch (characteristic.uuid().toUInt16()) {

    case 0x2AD9: // Fitness Machine Control Point

        if ((char)newValue.at(0) == FTMS_SET_TARGET_RESISTANCE_LEVEL) {

            // Set Target Resistance
            resistance_t uresistance = newValue.at(1);
            uresistance = uresistance / 10;
            /*
            if (force_resistance && !erg_mode) {
                rower->changeResistance(uresistance);
            }*/
            qDebug() << QStringLiteral("new requested resistance ") + QString::number(uresistance);
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_RESISTANCE_LEVEL);
            reply.append((quint8)FTMS_SUCCESS);
        } else if ((char)newValue.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) // simulation parameter

        {
            qDebug() << QStringLiteral("indoor rower simulation parameters");
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS);
            reply.append((quint8)FTMS_SUCCESS);

            int16_t iresistance = (((uint8_t)newValue.at(3)) + (newValue.at(4) << 8));
            //slopeChanged(iresistance);
        } else if ((char)newValue.at(0) == FTMS_SET_TARGET_POWER) // erg mode

        {
            qDebug() << QStringLiteral("erg mode");
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_POWER);
            reply.append((quint8)FTMS_SUCCESS);

            uint16_t power = (((uint8_t)newValue.at(1)) + (newValue.at(2) << 8));
            //powerChanged(power);
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
            qDebug() << QStringLiteral("virtual rower not connected");

            return;
        }
        writeCharacteristic(serviceFIT, characteristic, reply);
        break;
    }
}

void virtualrower::writeCharacteristic(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic,
                                       const QByteArray &value) {
    try {
        qDebug() << QStringLiteral("virtualrower::writeCharacteristic ") + service->serviceName() +
                        QStringLiteral(" ") + characteristic.name() + QStringLiteral(" ") + value.toHex(' ');
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    } catch (...) {
        qDebug() << QStringLiteral("virtual rower error!");
    }
}

void virtualrower::reconnect() {

    QSettings settings;
    bool bluetooth_relaxed =
        settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();

    if (bluetooth_relaxed) {
        return;
    }

    bool heart_only =
        settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();

    qDebug() << QStringLiteral("virtualrower::reconnect");
    leController->disconnectFromDevice();

    serviceFIT = leController->addService(serviceDataFIT);
    QThread::msleep(100); // give time to Android to add the service async.ly
    if (!this->noHeartService || heart_only)
        serviceHR = leController->addService(serviceDataHR);

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/BleAdvertiser", "startAdvertisingRower",
                                              "(Landroid/content/Context;)V", QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;").object());
#else
    leController->startAdvertising(pars, advertisingData, advertisingData);
#endif

}

void virtualrower::rowerProvider() {

    QSettings settings;
    bool heart_only =
        settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();

    double normalizeWattage = Rower->wattsMetricforUI();
    if (normalizeWattage < 0)
        normalizeWattage = 0;

    uint16_t normalizeSpeed = (uint16_t)qRound(Rower->currentSpeed().value() * 100);

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    if (h) {
        // really connected to a device
        if (h->virtualrower_updateFTMS(
                normalizeSpeed, (char)Rower->currentResistance().value(), (uint16_t)Rower->currentCadence().value() * 2,
                (uint16_t)normalizeWattage, Rower->currentCrankRevolutions(), Rower->lastCrankEventTime(),
                ((rower *)Rower)->currentStrokesCount().value(), Rower->odometer() * 1000, Rower->calories().value(),
                QTime(0, 0, 0).secsTo(((rower *)Rower)->currentPace()), static_cast<uint8_t>(Rower->deviceType()))) {
            h->virtualrower_setHeartRate(Rower->currentHeart().value());

            uint8_t ftms_message[255];
            int ret = h->virtualrower_getLastFTMSMessage(ftms_message);
            if (ret > 0) {
                lastFTMSFrameReceived = QDateTime::currentMSecsSinceEpoch();
                qDebug() << "FTMS rcv << " << QByteArray::fromRawData((char *)ftms_message, ret).toHex(' ');
                emit ftmsCharacteristicChanged(QLowEnergyCharacteristic(),
                                               QByteArray::fromRawData((char *)ftms_message, ret));
            }
            qDebug() << "last FTMS rcv" << lastFTMSFrameReceived;
            if (lastFTMSFrameReceived > 0 && QDateTime::currentMSecsSinceEpoch() < (lastFTMSFrameReceived + 30000)) { /*
                 if (!erg_mode)
                     writeP2AD9->changeSlope(h->virtualbike_getCurrentSlope(), 0, 0);
                 else {
                     qDebug() << "ios workaround power changed request" << h->virtualbike_getPowerRequested();
                     writeP2AD9->changePower(h->virtualbike_getPowerRequested());
                 }*/
            }
        }
        return;
    }
#endif
#endif

    if (leController->state() != QLowEnergyController::ConnectedState) {
        qDebug() << QStringLiteral("virtual rower not connected");

        return;
    } else {
        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        bool bluetooth_30m_hangs =
            settings.value(QZSettings::bluetooth_30m_hangs, QZSettings::default_bluetooth_30m_hangs).toBool();
        if (bluetooth_relaxed) {

            leController->stopAdvertising();
        }

        if (lastFTMSFrameReceived > 0 && QDateTime::currentMSecsSinceEpoch() > (lastFTMSFrameReceived + 5000) &&
            bluetooth_30m_hangs) {
            lastFTMSFrameReceived = 0;
            qDebug() << QStringLiteral("virtual rower timeout, reconnecting...");

            reconnect();
            return;
        }

        qDebug() << QStringLiteral("virtual rower connected");
    }

    QByteArray value;

    if (!heart_only) {

        value.append((char)0x2C);
        value.append((char)0x03);

        value.append((char)((uint8_t)(Rower->currentCadence().value() * 2) & 0xFF)); // Stroke Rate

        value.append((char)((uint16_t)(((rower *)Rower)->currentStrokesCount().value()) & 0xFF));        // Stroke Count
        value.append((char)(((uint16_t)(((rower *)Rower)->currentStrokesCount().value()) >> 8) & 0xFF)); // Stroke Count

        value.append((char)(((uint16_t)(((rower *)Rower)->odometer() * 1000.0)) & 0xFF));       // Distance
        value.append((char)(((uint16_t)(((rower *)Rower)->odometer() * 1000.0) >> 8) & 0xFF));  // Distance
        value.append((char)(((uint16_t)(((rower *)Rower)->odometer() * 1000.0) >> 16) & 0xFF)); // Distance

        value.append((char)(((uint16_t)QTime(0, 0, 0).secsTo(((rower *)Rower)->currentPace())) & 0xFF));      // pace
        value.append((char)(((uint16_t)QTime(0, 0, 0).secsTo(((rower *)Rower)->currentPace())) >> 8) & 0xFF); // pace

        value.append((char)(((uint16_t)Rower->wattsMetricforUI()) & 0xFF));      // watts
        value.append((char)(((uint16_t)Rower->wattsMetricforUI()) >> 8) & 0xFF); // watts

        value.append((char)((uint16_t)(Rower->calories().value()) & 0xFF));        // calories
        value.append((char)(((uint16_t)(Rower->calories().value()) >> 8) & 0xFF)); // calories
        value.append((char)((uint16_t)(Rower->calories().value()) & 0xFF));        // calories
        value.append((char)(((uint16_t)(Rower->calories().value()) >> 8) & 0xFF)); // calories
        value.append((char)((uint16_t)(Rower->calories().value()) & 0xFF));        // calories

        value.append(char(Rower->currentHeart().value())); // Actual value.
        value.append((char)0);                             // Bkool FTMS protocol HRM offset 1280 fix

        if (!serviceFIT) {
            qDebug() << QStringLiteral("serviceFIT not available");

            return;
        }

        QLowEnergyCharacteristic characteristic =
            serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD1);
        if (!characteristic.isValid()) {
            qDebug() << QStringLiteral("virtual rower characteristic not valid!");

            return;
        }
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual rower not connected");

            return;
        }
        writeCharacteristic(serviceFIT, characteristic, value);
    }
    // characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    // Q_ASSERT(characteristic.isValid());
    // service->readCharacteristic(characteristic);

    if (!this->noHeartService || heart_only) {
        if (!serviceHR) {
            qDebug() << QStringLiteral("serviceHR not available");

            return;
        }

        QByteArray valueHR;
        valueHR.append(char(0));                                   // Flags that specify the format of the value.
        valueHR.append(char(Rower->metrics_override_heartrate())); // Actual value.
        QLowEnergyCharacteristic characteristicHR = serviceHR->characteristic(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);

        Q_ASSERT(characteristicHR.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual rower not connected");

            return;
        }
        writeCharacteristic(serviceHR, characteristicHR, valueHR);
    }
}

bool virtualrower::connected() {
    if (!leController) {

        return false;
    }
    return leController->state() == QLowEnergyController::ConnectedState;
}

void virtualrower::error(QLowEnergyController::Error newError) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("virtualrower::controller:ERROR ") +
                    QString::fromLocal8Bit(metaEnum.valueToKey(newError));

    if (newError != QLowEnergyController::RemoteHostClosedError) {
        reconnect();
    }
}
