#include "virtualdevices/virtualrower.h"
#include "qsettings.h"
#include "qzsettings.h"
#include "rower.h"
#include "ftmsbike/ftmsbike.h"

#include <QDataStream>
#include <QMetaEnum>
#include <QSettings>
#include <QtMath>
#include <chrono>
#include <QThread>

// PM5 Concept2 BLE UUIDs
// Base UUID: CE06XXXX-43E5-11E4-916C-0800200C9A66
static const QBluetoothUuid PM5_ROWING_SERVICE_UUID(QStringLiteral("CE060030-43E5-11E4-916C-0800200C9A66"));
static const QBluetoothUuid PM5_DEVICE_INFO_SERVICE_UUID(QStringLiteral("CE060010-43E5-11E4-916C-0800200C9A66"));
static const QBluetoothUuid PM5_GENERAL_STATUS_UUID(QStringLiteral("CE060031-43E5-11E4-916C-0800200C9A66"));
static const QBluetoothUuid PM5_ADDITIONAL_STATUS_UUID(QStringLiteral("CE060032-43E5-11E4-916C-0800200C9A66"));
static const QBluetoothUuid PM5_ADDITIONAL_STATUS2_UUID(QStringLiteral("CE060033-43E5-11E4-916C-0800200C9A66"));
static const QBluetoothUuid PM5_SAMPLE_RATE_UUID(QStringLiteral("CE060034-43E5-11E4-916C-0800200C9A66"));
static const QBluetoothUuid PM5_MULTIPLEXED_INFO_UUID(QStringLiteral("CE060080-43E5-11E4-916C-0800200C9A66"));

// PM5 Workout states
enum PM5WorkoutState {
    PM5_WORKOUT_WAITING = 0,
    PM5_WORKOUT_WORKING = 1,
    PM5_WORKOUT_RESTING = 2,
    PM5_WORKOUT_FINISHED = 10
};

// PM5 Rowing states
enum PM5RowingState {
    PM5_ROWING_INACTIVE = 0,
    PM5_ROWING_ACTIVE = 1
};

// PM5 Stroke states
enum PM5StrokeState {
    PM5_STROKE_WAITING = 0,
    PM5_STROKE_DRIVE = 1,
    PM5_STROKE_DWELL = 2,
    PM5_STROKE_RECOVERY = 3
};

// PM5 Erg machine types
enum PM5ErgMachineType {
    PM5_ERG_ROWER = 0,
    PM5_ERG_SKIERG = 1,
    PM5_ERG_BIKEERG = 2
};

#ifdef Q_OS_ANDROID
#include "androidactivityresultreceiver.h"
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

using namespace std::chrono_literals;

virtualrower::virtualrower(bluetoothdevice *t, bool noWriteResistance, bool noHeartService) {
    Rower = t;

    this->noHeartService = noHeartService;

    QSettings settings;
    bool heart_only =
        settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();

    // Check if PM5 mode is enabled
    pm5Mode = settings.value(QZSettings::virtual_device_rower_pm5, QZSettings::default_virtual_device_rower_pm5).toBool();
    qDebug() << "virtualrower PM5 mode:" << pm5Mode;

    Q_UNUSED(noWriteResistance)

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && !heart_only && !pm5Mode) {

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

        if (pm5Mode) {
            // PM5 device name format: "PM5 XXXXXX" where XXXXXX is serial number
            advertisingData.setLocalName(QStringLiteral("PM5 430000000"));
            qDebug() << "Advertising as PM5 Concept2 rower";
        } else {
            advertisingData.setLocalName(QStringLiteral("PM5"));
        }

        QList<QBluetoothUuid> services;

        if (!heart_only) {
            if (pm5Mode) {
                // PM5 uses Concept2 proprietary services
                services << PM5_ROWING_SERVICE_UUID;
                services << PM5_DEVICE_INFO_SERVICE_UUID;
            } else {
                services << ((QBluetoothUuid::ServiceClassUuid)0x1826);
            }
        }
        if (!this->noHeartService || heart_only) {
            services << QBluetoothUuid::HeartRate;
        }

        if (!pm5Mode) {
            services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);
        }

        advertisingData.setServices(services);
        //! [Advertising Data]

        if (!heart_only) {
            if (pm5Mode) {
                // Setup PM5 Concept2 rowing service
                setupPM5Services();
            } else {
                // Setup FTMS service (original code)
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
                const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                              QByteArray(2, 0));
                charDataFIT3.addDescriptor(cpClientConfig);

                QLowEnergyCharacteristicData charDataFIT4;
                charDataFIT4.setUuid((QBluetoothUuid::CharacteristicType)0x2AD1); // rower
                charDataFIT4.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                QByteArray descriptor;
                descriptor.append((char)0x01);
                descriptor.append((char)0x00);
                const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor);
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
            }
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

        //! [Start Advertising]
        leController = QLowEnergyController::createPeripheral();
        Q_ASSERT(leController);

        if (pm5Mode) {
            // Add PM5 services
            servicePM5Rowing = leController->addService(serviceDataPM5Rowing);
            QThread::msleep(100);
            servicePM5DeviceInfo = leController->addService(serviceDataPM5DeviceInfo);
            QThread::msleep(100);
        } else {
            serviceFIT = leController->addService(serviceDataFIT);
            QThread::msleep(100); // give time to Android to add the service async.ly
        }

        if (!this->noHeartService || heart_only) {
            serviceHR = leController->addService(serviceDataHR);
        }

        if (!pm5Mode && serviceFIT) {
            QObject::connect(serviceFIT, &QLowEnergyService::characteristicChanged, this,
                             &virtualrower::characteristicChanged);
        }

        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
        if (!bluetooth_relaxed) {
            pars.setInterval(100, 100);
        }

#ifdef Q_OS_ANDROID
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/BleAdvertiser", "startAdvertisingRower",
                                                  "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
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
        static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error), this,
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

    qDebug() << QStringLiteral("virtualrower::reconnect") << "pm5Mode:" << pm5Mode;
    leController->disconnectFromDevice();

    if (pm5Mode) {
        servicePM5Rowing = leController->addService(serviceDataPM5Rowing);
        QThread::msleep(100);
        servicePM5DeviceInfo = leController->addService(serviceDataPM5DeviceInfo);
        QThread::msleep(100);
    } else {
        serviceFIT = leController->addService(serviceDataFIT);
        QThread::msleep(100); // give time to Android to add the service async.ly
    }

    if (!this->noHeartService || heart_only)
        serviceHR = leController->addService(serviceDataHR);

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/BleAdvertiser", "startAdvertisingRower",
                                              "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
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

    // Get stroke count based on device type
    uint32_t strokeCount = 0;
    if (Rower->deviceType() == ROWING) {
        strokeCount = ((rower *)Rower)->currentStrokesCount().value();
    } else {
        // For bikes/other devices, estimate strokes from cadence
        strokeCount = (uint32_t)(Rower->currentCadence().value() * 2 * Rower->movingTime().hour() * 3600 +
                                 Rower->movingTime().minute() * 60 + Rower->movingTime().second());
    }

    // Get pace based on device type
    uint16_t paceSecs = 0;
    if (Rower->deviceType() == ROWING) {
        paceSecs = QTime(0, 0, 0).secsTo(((rower *)Rower)->currentPace());
    } else {
        // For bikes, pace = odometer / moving_time in seconds
        if (Rower->movingTime().hour() > 0 || Rower->movingTime().minute() > 0 || Rower->movingTime().second() > 0) {
            double totalSecs = Rower->movingTime().hour() * 3600 + Rower->movingTime().minute() * 60 + Rower->movingTime().second();
            if (totalSecs > 0)
                paceSecs = (uint16_t)(Rower->odometer() * 1000.0 / totalSecs);
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    if (h) {
        // really connected to a device
        if (h->virtualrower_updateFTMS(
                normalizeSpeed, (char)Rower->currentResistance().value(), (uint16_t)Rower->currentCadence().value() * 2,
                (uint16_t)normalizeWattage, Rower->currentCrankRevolutions(), Rower->lastCrankEventTime(),
                strokeCount, Rower->odometer() * 1000, Rower->calories().value(),
                paceSecs, static_cast<uint8_t>(Rower->deviceType()))) {
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
        if (pm5Mode) {
            // PM5 Concept2 protocol
            if (!servicePM5Rowing) {
                qDebug() << QStringLiteral("servicePM5Rowing not available");
                return;
            }

            // Send General Status (CE060031)
            QByteArray generalStatus = buildPM5GeneralStatus();
            QLowEnergyCharacteristic charGeneralStatus = servicePM5Rowing->characteristic(PM5_GENERAL_STATUS_UUID);
            if (charGeneralStatus.isValid()) {
                writeCharacteristic(servicePM5Rowing, charGeneralStatus, generalStatus);
                qDebug() << "PM5 General Status:" << generalStatus.toHex(' ');
            }

            // Send Additional Status (CE060032)
            QByteArray additionalStatus = buildPM5AdditionalStatus();
            QLowEnergyCharacteristic charAdditionalStatus = servicePM5Rowing->characteristic(PM5_ADDITIONAL_STATUS_UUID);
            if (charAdditionalStatus.isValid()) {
                writeCharacteristic(servicePM5Rowing, charAdditionalStatus, additionalStatus);
                qDebug() << "PM5 Additional Status:" << additionalStatus.toHex(' ');
            }

            // Send Additional Status 2 (CE060033)
            QByteArray additionalStatus2 = buildPM5AdditionalStatus2();
            QLowEnergyCharacteristic charAdditionalStatus2 = servicePM5Rowing->characteristic(PM5_ADDITIONAL_STATUS2_UUID);
            if (charAdditionalStatus2.isValid()) {
                writeCharacteristic(servicePM5Rowing, charAdditionalStatus2, additionalStatus2);
                qDebug() << "PM5 Additional Status 2:" << additionalStatus2.toHex(' ');
            }
        } else {
            // FTMS protocol (original code)
            value.append((char)0x2C);
            value.append((char)0x03);

            value.append((char)((uint8_t)(Rower->currentCadence().value() * 2) & 0xFF)); // Stroke Rate

            value.append((char)((uint16_t)(strokeCount & 0xFF)));        // Stroke Count
            value.append((char)(((uint16_t)(strokeCount >> 8) & 0xFF))); // Stroke Count

            value.append((char)(((uint16_t)(Rower->odometer() * 1000.0)) & 0xFF));       // Distance
            value.append((char)(((uint16_t)(Rower->odometer() * 1000.0) >> 8) & 0xFF));  // Distance
            value.append((char)(((uint16_t)(Rower->odometer() * 1000.0) >> 16) & 0xFF)); // Distance

            value.append((char)((uint16_t)(paceSecs & 0xFF)));      // pace
            value.append((char)(((uint16_t)(paceSecs >> 8) & 0xFF))); // pace

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
        QLowEnergyCharacteristic characteristicHR = serviceHR->characteristic(QBluetoothUuid::HeartRateMeasurement);

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

void virtualrower::setupPM5Services() {
    qDebug() << "Setting up PM5 Concept2 services";

    // PM5 Device Information Service (CE060010)
    serviceDataPM5DeviceInfo.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceDataPM5DeviceInfo.setUuid(PM5_DEVICE_INFO_SERVICE_UUID);

    // PM5 Rowing Service (CE060030)
    serviceDataPM5Rowing.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceDataPM5Rowing.setUuid(PM5_ROWING_SERVICE_UUID);

    // General Status Characteristic (CE060031) - 19 bytes
    QLowEnergyCharacteristicData charGeneralStatus;
    charGeneralStatus.setUuid(PM5_GENERAL_STATUS_UUID);
    charGeneralStatus.setProperties(QLowEnergyCharacteristic::Notify);
    QByteArray descriptorGS;
    descriptorGS.append((char)0x01);
    descriptorGS.append((char)0x00);
    const QLowEnergyDescriptorData clientConfigGS(QBluetoothUuid::ClientCharacteristicConfiguration, descriptorGS);
    charGeneralStatus.addDescriptor(clientConfigGS);
    charGeneralStatus.setValue(QByteArray(19, 0));

    // Additional Status Characteristic (CE060032) - 19 bytes
    QLowEnergyCharacteristicData charAdditionalStatus;
    charAdditionalStatus.setUuid(PM5_ADDITIONAL_STATUS_UUID);
    charAdditionalStatus.setProperties(QLowEnergyCharacteristic::Notify);
    QByteArray descriptorAS;
    descriptorAS.append((char)0x01);
    descriptorAS.append((char)0x00);
    const QLowEnergyDescriptorData clientConfigAS(QBluetoothUuid::ClientCharacteristicConfiguration, descriptorAS);
    charAdditionalStatus.addDescriptor(clientConfigAS);
    charAdditionalStatus.setValue(QByteArray(19, 0));

    // Additional Status 2 Characteristic (CE060033) - 20 bytes
    QLowEnergyCharacteristicData charAdditionalStatus2;
    charAdditionalStatus2.setUuid(PM5_ADDITIONAL_STATUS2_UUID);
    charAdditionalStatus2.setProperties(QLowEnergyCharacteristic::Notify);
    QByteArray descriptorAS2;
    descriptorAS2.append((char)0x01);
    descriptorAS2.append((char)0x00);
    const QLowEnergyDescriptorData clientConfigAS2(QBluetoothUuid::ClientCharacteristicConfiguration, descriptorAS2);
    charAdditionalStatus2.addDescriptor(clientConfigAS2);
    charAdditionalStatus2.setValue(QByteArray(20, 0));

    // Multiplexed Info Characteristic (CE060080) - variable size
    QLowEnergyCharacteristicData charMultiplexedInfo;
    charMultiplexedInfo.setUuid(PM5_MULTIPLEXED_INFO_UUID);
    charMultiplexedInfo.setProperties(QLowEnergyCharacteristic::Notify);
    QByteArray descriptorMI;
    descriptorMI.append((char)0x01);
    descriptorMI.append((char)0x00);
    const QLowEnergyDescriptorData clientConfigMI(QBluetoothUuid::ClientCharacteristicConfiguration, descriptorMI);
    charMultiplexedInfo.addDescriptor(clientConfigMI);
    charMultiplexedInfo.setValue(QByteArray(20, 0));

    // Add characteristics to rowing service
    serviceDataPM5Rowing.addCharacteristic(charGeneralStatus);
    serviceDataPM5Rowing.addCharacteristic(charAdditionalStatus);
    serviceDataPM5Rowing.addCharacteristic(charAdditionalStatus2);
    serviceDataPM5Rowing.addCharacteristic(charMultiplexedInfo);

    qDebug() << "PM5 services setup complete";
}

QByteArray virtualrower::buildPM5GeneralStatus() {
    // PM5 General Status - 19 bytes
    // Bytes 0-2: Elapsed Time (UInt24LE, 0.01 sec)
    // Bytes 3-5: Distance (UInt24LE, 0.1 m)
    // Byte 6: Workout Type (0 = Just Row Free)
    // Byte 7: Interval Type (0 = None)
    // Byte 8: Workout State (1 = Working)
    // Byte 9: Rowing State (1 = Active)
    // Byte 10: Stroke State (1 = Driving, 3 = Recovery)
    // Bytes 11-13: Total Work Distance (UInt24LE, 1 m)
    // Bytes 14-16: Workout Duration (target distance in m or time in 0.01s)
    // Byte 17: Workout Duration Type (0 = time, 128 = distance)
    // Byte 18: Drag Factor

    QByteArray value(19, 0);

    // Calculate elapsed time in centiseconds (0.01 sec units)
    uint32_t elapsedCentiseconds = (uint32_t)(
        (Rower->movingTime().hour() * 3600 +
         Rower->movingTime().minute() * 60 +
         Rower->movingTime().second()) * 100 +
        Rower->movingTime().msec() / 10);

    // Elapsed time (24-bit LE)
    value[0] = (char)(elapsedCentiseconds & 0xFF);
    value[1] = (char)((elapsedCentiseconds >> 8) & 0xFF);
    value[2] = (char)((elapsedCentiseconds >> 16) & 0xFF);

    // Distance in 0.1m units (24-bit LE)
    uint32_t distanceDecimeters = (uint32_t)(Rower->odometer() * 10000.0); // odometer is in km, convert to 0.1m
    value[3] = (char)(distanceDecimeters & 0xFF);
    value[4] = (char)((distanceDecimeters >> 8) & 0xFF);
    value[5] = (char)((distanceDecimeters >> 16) & 0xFF);

    // Workout Type - 0 = Just Row Free
    value[6] = 0x00;

    // Interval Type - 0 = None
    value[7] = 0x00;

    // Workout State - 1 = Working (if moving), 0 = Waiting (if not)
    value[8] = (Rower->currentSpeed().value() > 0 || Rower->wattsMetricforUI() > 0) ? PM5_WORKOUT_WORKING : PM5_WORKOUT_WAITING;

    // Rowing State - 1 = Active (if rowing), 0 = Inactive
    value[9] = (Rower->currentSpeed().value() > 0 || Rower->wattsMetricforUI() > 0) ? PM5_ROWING_ACTIVE : PM5_ROWING_INACTIVE;

    // Stroke State - Alternate between drive and recovery based on stroke
    static uint8_t strokeState = PM5_STROKE_DRIVE;
    if (Rower->currentSpeed().value() > 0) {
        strokeState = (strokeState == PM5_STROKE_DRIVE) ? PM5_STROKE_RECOVERY : PM5_STROKE_DRIVE;
    } else {
        strokeState = PM5_STROKE_WAITING;
    }
    value[10] = strokeState;

    // Total Work Distance in meters (24-bit LE)
    uint32_t totalDistanceMeters = (uint32_t)(Rower->odometer() * 1000.0); // odometer in km -> m
    value[11] = (char)(totalDistanceMeters & 0xFF);
    value[12] = (char)((totalDistanceMeters >> 8) & 0xFF);
    value[13] = (char)((totalDistanceMeters >> 16) & 0xFF);

    // Workout Duration (target) - set to 0 for free row
    value[14] = 0x00;
    value[15] = 0x00;
    value[16] = 0x00;

    // Workout Duration Type - 0 = time based
    value[17] = 0x00;

    // Drag Factor (typical range 90-130 for rowers)
    value[18] = 110;

    return value;
}

QByteArray virtualrower::buildPM5AdditionalStatus() {
    // PM5 Additional Status - 19 bytes
    // Bytes 0-2: Elapsed Time (UInt24LE, 0.01 sec)
    // Bytes 3-4: Speed (UInt16LE, 0.001 m/sec)
    // Byte 5: Stroke Rate (strokes/min)
    // Byte 6: Heart Rate (bpm)
    // Bytes 7-8: Current Pace (UInt16LE, 0.01 sec/500m)
    // Bytes 9-10: Average Pace (UInt16LE, 0.01 sec/500m)
    // Bytes 11-12: Rest Distance
    // Bytes 13-15: Rest Time (UInt24LE, 0.01 sec)
    // Bytes 16-17: Average Power (watts)
    // Byte 18: Erg Machine Type (0 = Rower)

    QByteArray value(19, 0);

    // Calculate elapsed time in centiseconds
    uint32_t elapsedCentiseconds = (uint32_t)(
        (Rower->movingTime().hour() * 3600 +
         Rower->movingTime().minute() * 60 +
         Rower->movingTime().second()) * 100 +
        Rower->movingTime().msec() / 10);

    // Elapsed time (24-bit LE)
    value[0] = (char)(elapsedCentiseconds & 0xFF);
    value[1] = (char)((elapsedCentiseconds >> 8) & 0xFF);
    value[2] = (char)((elapsedCentiseconds >> 16) & 0xFF);

    // Speed in 0.001 m/s units (16-bit LE)
    // currentSpeed is in km/h, convert to m/s * 1000
    uint16_t speedMillimetersPerSec = (uint16_t)(Rower->currentSpeed().value() * 1000.0 / 3.6);
    value[3] = (char)(speedMillimetersPerSec & 0xFF);
    value[4] = (char)((speedMillimetersPerSec >> 8) & 0xFF);

    // Stroke Rate (strokes per minute)
    value[5] = (uint8_t)Rower->currentCadence().value();

    // Heart Rate
    value[6] = (uint8_t)Rower->currentHeart().value();

    // Current Pace in 0.01 sec/500m units (16-bit LE)
    uint16_t paceCentiseconds = 0;
    if (Rower->deviceType() == ROWING) {
        QTime pace = ((rower *)Rower)->currentPace();
        paceCentiseconds = (uint16_t)((pace.minute() * 60 + pace.second()) * 100 + pace.msec() / 10);
    } else if (Rower->currentSpeed().value() > 0) {
        // Calculate pace from speed: pace (s/500m) = 500 / speed (m/s)
        double speedMs = Rower->currentSpeed().value() / 3.6;
        if (speedMs > 0) {
            paceCentiseconds = (uint16_t)(500.0 / speedMs * 100.0);
        }
    }
    value[7] = (char)(paceCentiseconds & 0xFF);
    value[8] = (char)((paceCentiseconds >> 8) & 0xFF);

    // Average Pace (same as current for now)
    value[9] = value[7];
    value[10] = value[8];

    // Rest Distance - 0
    value[11] = 0x00;
    value[12] = 0x00;

    // Rest Time - 0
    value[13] = 0x00;
    value[14] = 0x00;
    value[15] = 0x00;

    // Average Power (16-bit LE)
    uint16_t avgPower = (uint16_t)Rower->wattsMetricforUI();
    value[16] = (char)(avgPower & 0xFF);
    value[17] = (char)((avgPower >> 8) & 0xFF);

    // Erg Machine Type - 0 = Rower
    value[18] = PM5_ERG_ROWER;

    return value;
}

QByteArray virtualrower::buildPM5AdditionalStatus2() {
    // PM5 Additional Status 2 - 20 bytes
    // Bytes 0-2: Elapsed Time (UInt24LE, 0.01 sec)
    // Byte 3: Interval Count
    // Bytes 4-5: Average Power (watts)
    // Bytes 6-7: Total Calories (kCal)
    // Bytes 8-9: Split Average Pace (0.01 sec/500m)
    // Bytes 10-11: Split Average Power (watts)
    // Bytes 12-13: Split Average Calories (kCal/hour)
    // Bytes 14-16: Last Split Time (UInt24LE, 0.01 sec)
    // Bytes 17-19: Last Split Distance (UInt24LE, meters)

    QByteArray value(20, 0);

    // Calculate elapsed time in centiseconds
    uint32_t elapsedCentiseconds = (uint32_t)(
        (Rower->movingTime().hour() * 3600 +
         Rower->movingTime().minute() * 60 +
         Rower->movingTime().second()) * 100 +
        Rower->movingTime().msec() / 10);

    // Elapsed time (24-bit LE)
    value[0] = (char)(elapsedCentiseconds & 0xFF);
    value[1] = (char)((elapsedCentiseconds >> 8) & 0xFF);
    value[2] = (char)((elapsedCentiseconds >> 16) & 0xFF);

    // Interval Count
    value[3] = 0x01;

    // Average Power (16-bit LE)
    uint16_t avgPower = (uint16_t)Rower->wattsMetricforUI();
    value[4] = (char)(avgPower & 0xFF);
    value[5] = (char)((avgPower >> 8) & 0xFF);

    // Total Calories (16-bit LE)
    uint16_t calories = (uint16_t)Rower->calories().value();
    value[6] = (char)(calories & 0xFF);
    value[7] = (char)((calories >> 8) & 0xFF);

    // Split Average Pace (16-bit LE) - same as current pace
    uint16_t paceCentiseconds = 0;
    if (Rower->deviceType() == ROWING) {
        QTime pace = ((rower *)Rower)->currentPace();
        paceCentiseconds = (uint16_t)((pace.minute() * 60 + pace.second()) * 100 + pace.msec() / 10);
    } else if (Rower->currentSpeed().value() > 0) {
        double speedMs = Rower->currentSpeed().value() / 3.6;
        if (speedMs > 0) {
            paceCentiseconds = (uint16_t)(500.0 / speedMs * 100.0);
        }
    }
    value[8] = (char)(paceCentiseconds & 0xFF);
    value[9] = (char)((paceCentiseconds >> 8) & 0xFF);

    // Split Average Power (16-bit LE)
    value[10] = value[4];
    value[11] = value[5];

    // Split Average Calories (kCal/hour) (16-bit LE)
    uint16_t calPerHour = 0;
    double totalSeconds = Rower->movingTime().hour() * 3600 + Rower->movingTime().minute() * 60 + Rower->movingTime().second();
    if (totalSeconds > 0) {
        calPerHour = (uint16_t)(calories * 3600.0 / totalSeconds);
    }
    value[12] = (char)(calPerHour & 0xFF);
    value[13] = (char)((calPerHour >> 8) & 0xFF);

    // Last Split Time - 0 (no splits)
    value[14] = 0x00;
    value[15] = 0x00;
    value[16] = 0x00;

    // Last Split Distance - 0 (no splits)
    value[17] = 0x00;
    value[18] = 0x00;
    value[19] = 0x00;

    return value;
}
