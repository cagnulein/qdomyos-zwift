#include "virtualtreadmill.h"
#include "elliptical.h"
#include "ftmsbike.h"
#include <QSettings>
#include <QtMath>
#include <chrono>

using namespace std::chrono_literals;

bool virtualtreadmill::configureLockScreen() {

    this->lockscreenFunctions = QZLockscreenFunctions::create();
    if(!this->lockscreenFunctions)
        return false;

    this->lockscreenFunctions->setVirtualTreadmill(true);
    return this->lockscreenFunctions->isPelotonWorkaroundActive();

}

virtualtreadmill::virtualtreadmill(bluetoothdevice *t, bool noHeartService) {
    QSettings settings;
    treadMill = t;

    this->noHeartService = noHeartService;
    if (settings.value(QZSettings::dircon_yes, QZSettings::default_dircon_yes).toBool()) {
        dirconManager = new DirconManager(t, 0, 0, this);
        connect(dirconManager, SIGNAL(changeInclination(double, double)), this,
                SIGNAL(changeInclination(double, double)));
        connect(dirconManager, SIGNAL(changeInclination(double, double)), this,
                SLOT(dirconChangedInclination(double, double)));
        connect(dirconManager, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    }
    if (!settings.value(QZSettings::virtual_device_bluetooth, QZSettings::default_virtual_device_bluetooth).toBool())
        return;
    notif2AD9 = new CharacteristicNotifier2AD9(t, this);
    notif2AD2 = new CharacteristicNotifier2AD2(t, this);
    notif2ACD = new CharacteristicNotifier2ACD(t, this);
    notif2A53 = new CharacteristicNotifier2A53(t, this);
    notif2A37 = new CharacteristicNotifier2A37(t, this);
    writeP2AD9 = new CharacteristicWriteProcessor2AD9(0, 0, t, notif2AD9, this);
    connect(writeP2AD9, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
    connect(writeP2AD9, SIGNAL(slopeChanged()), this, SLOT(slopeChanged()));

    if(!this->configureLockScreen())
    {
        //! [Advertising Data]
        advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
        advertisingData.setIncludePowerLevel(true);
        advertisingData.setLocalName(QStringLiteral("DomyosBridge"));
        QList<QBluetoothUuid> services;

        if (ftmsServiceEnable()) {
            services << ((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
        }

        if (RSCEnable()) {
            services << (QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
        }

        if (noHeartService == false) {
            services << QBluetoothUuid::HeartRate;
        }

        services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);

        advertisingData.setServices(services);
        //! [Advertising Data]

        //! [Service Data]
        if (ftmsServiceEnable()) {
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
            if (ftmsTreadmillEnable()) {
                charData2.setUuid((QBluetoothUuid::CharacteristicType)0x2ACD); // TreadmillDataCharacteristicUuid
                charData2.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                QByteArray descriptor;
                descriptor.append((char)0x01);
                descriptor.append((char)0x00);
                const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                             descriptor);
                charData2.addDescriptor(clientConfig2);
            }

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

            // indoor bike in order to exploit a zwift bug that allows treadmill to get the incline values
            QLowEnergyCharacteristicData charDataFIT7;
            charDataFIT7.setUuid((QBluetoothUuid::CharacteristicType)0x2AD2); // indoor bike
            charDataFIT7.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
            QByteArray descriptor7;
            descriptor7.append((char)0x01);
            descriptor7.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig7(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                         descriptor7);
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

            serviceDataFTMS.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceDataFTMS.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
            serviceDataFTMS.addCharacteristic(charData);
            if (ftmsTreadmillEnable())
                serviceDataFTMS.addCharacteristic(charData2);
            serviceDataFTMS.addCharacteristic(charData3);
            serviceDataFTMS.addCharacteristic(charDataFIT5);
            serviceDataFTMS.addCharacteristic(charDataFIT6);
            serviceDataFTMS.addCharacteristic(charDataFIT7);
            serviceDataFTMS.addCharacteristic(charDataFIT2);
        }

        if (RSCEnable()) {
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

            serviceDataRSC.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceDataRSC.setUuid(QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
            serviceDataRSC.addCharacteristic(charData);
            serviceDataRSC.addCharacteristic(charData3);
            serviceDataRSC.addCharacteristic(charData2);
            serviceDataRSC.addCharacteristic(charData4);
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
        if (ftmsServiceEnable())
            serviceFTMS = leController->addService(serviceDataFTMS);
        if (RSCEnable())
            serviceRSC = leController->addService(serviceDataRSC);
        if (noHeartService == false) {
            serviceHR = leController->addService(serviceDataHR);
        }

        if (serviceFTMS)
            QObject::connect(serviceFTMS, &QLowEnergyService::characteristicChanged, this,
                             &virtualtreadmill::characteristicChanged);

        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
        if (!bluetooth_relaxed) {
            pars.setInterval(100, 100);
        }

        leController->startAdvertising(pars, advertisingData, advertisingData);
        //! [Start Advertising]

        //! [Provide Heartbeat]
        QObject::connect(leController, &QLowEnergyController::disconnected, this, &virtualtreadmill::reconnect);
    }
    //! [Provide Heartbeat]
    QObject::connect(&treadmillTimer, &QTimer::timeout, this, &virtualtreadmill::treadmillProvider);
    treadmillTimer.start(1s);
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
                serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                qDebug() << QStringLiteral("virtualtreadmill::writeCharacteristic ") + serviceFTMS->serviceName() +
                                QStringLiteral(" ") + characteristic.name() + QStringLiteral(" ") + reply.toHex(' ');
                serviceFTMS->writeCharacteristic(characteristic, reply); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtual treadmill error!");
            }
        }
        break;
    }
}

void virtualtreadmill::slopeChanged() {
    lastSlopeChanged = QDateTime::currentSecsSinceEpoch();

    if (treadMill && treadMill->autoResistance())
        m_autoInclinationEnabled = true;
    else
        m_autoInclinationEnabled = false;
}

void virtualtreadmill::reconnect() {
    QSettings settings;
    bool bluetooth_relaxed =
        settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
    bool cadence = settings.value(QZSettings::run_cadence_sensor, QZSettings::default_run_cadence_sensor).toBool();

    if (bluetooth_relaxed) {
        return;
    }

    qDebug() << QStringLiteral("virtualtreadmill reconnect ") << treadMill->connected();
    if (ftmsServiceEnable())
        serviceFTMS = leController->addService(serviceDataFTMS);
    if (RSCEnable())
        serviceRSC = leController->addService(serviceDataRSC);

    if (noHeartService == false) {
        serviceHR = leController->addService(serviceDataHR);
    }

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);

    if (serviceFTMS || serviceRSC) {
        leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData, advertisingData);
    }
}

bool virtualtreadmill::doLockscreenUpdate() {

    if(!this->lockscreenFunctions && this->lockscreenFunctions->isPelotonWorkaroundActive())
        return false;

    QSettings settings;
    bool double_cadence = settings
                              .value(QZSettings::powr_sensor_running_cadence_double,
                                     QZSettings::default_powr_sensor_running_cadence_double).toBool();
    double cadence_multiplier = 2.0;
    if (double_cadence)
        cadence_multiplier = 1.0;

    QZLockscreen * lockscreen = this->lockscreenFunctions->getLockscreen();
    treadmill * t = static_cast<treadmill*>(this->treadMill);
    uint16_t normalizeSpeed = (uint16_t)qRound(t->currentSpeed().value() * 100);
    // really connected to a device
    if (lockscreen->virtualtreadmill_updateFTMS(
                normalizeSpeed, 0, (uint16_t)t->currentCadence().value() * cadence_multiplier,
                (uint16_t)t->wattsMetric().value(), t->currentInclination().value() * 10)) {
        lockscreen->virtualtreadmill_setHeartRate(t->currentHeart().value());
        lastSlopeChanged = lockscreen->virtualtreadmill_lastChangeCurrentSlope();
        if ((uint64_t)QDateTime::currentSecsSinceEpoch() < lastSlopeChanged + virtualtreadmill::slopeTimeoutSecs)
            writeP2AD9->changeSlope(lockscreen->virtualtreadmill_getCurrentSlope(), 0, 0);
    }
    return true;
}

void virtualtreadmill::treadmillProvider() {    

    if ((uint64_t)QDateTime::currentSecsSinceEpoch() > lastSlopeChanged + virtualtreadmill::slopeTimeoutSecs)
        m_autoInclinationEnabled = false;

    if(this->doLockscreenUpdate())
        return;

    if (leController->state() != QLowEnergyController::ConnectedState) {
        qDebug() << QStringLiteral("virtualtreadmill connection error");
        return;
    } else {
        QSettings settings;
        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        if (bluetooth_relaxed) {
            leController->stopAdvertising();
        }
    }

    QByteArray value;

    if (ftmsServiceEnable()) {
        if (ftmsTreadmillEnable()) {
            value.clear();
            if (notif2ACD->notify(value) == CN_OK) {
                if (!serviceFTMS) {
                    qDebug() << QStringLiteral("service not available");

                    return;
                }

                QLowEnergyCharacteristic characteristic =
                    serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD);
                Q_ASSERT(characteristic.isValid());
                if (leController->state() != QLowEnergyController::ConnectedState) {
                    qDebug() << QStringLiteral("virtual treadmill not connected");

                    return;
                }
                try {
                    serviceFTMS->writeCharacteristic(characteristic, value); // Potentially causes notification.
                } catch (...) {
                    qDebug() << QStringLiteral("virtualtreadmill error!");
                }
            }
        }
        value.clear();
        if (notif2AD2->notify(value) == CN_OK) {
            if (!serviceFTMS) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceFTMS->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }
    if (RSCEnable()) {
        value.clear();
        if (notif2A53->notify(value) == CN_OK) {
            if (!serviceRSC) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceRSC->characteristic(QBluetoothUuid::CharacteristicType::RSCMeasurement);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceRSC->writeCharacteristic(characteristic, value); // Potentially causes notification.
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
        value.clear();
        if (notif2A53->notify(value) == CN_OK) {
            if (!serviceHR) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceHR->characteristic(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
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

// Setup           |  FTMS Service | FTMS Treadmill Data | FTMS Bike | RSC | Heart
// -------------------------------------------------------------------------------
// iOS FTMS        |      X        |          X          |     X     |     |   X
// iOS RSC         |      X        |                     |     X     |  X  |   X
// Android<10 FTMS |      X        |          X          |     X     |     |   X
// Android<10 RSC  |      X        |                     |     X     |  X  |   X
// Android>9 FTMS  |      X        |          X          |     X     |     |
// Android>9 RSC   |               |                     |           |  X  |

bool virtualtreadmill::ftmsServiceEnable() {
    QSettings settings;
    bool cadence = settings.value(QZSettings::run_cadence_sensor, QZSettings::default_run_cadence_sensor).toBool();
    if (!cadence)
        return true;
    if (noHeartService == false)
        return true;
    return false;
}

bool virtualtreadmill::ftmsTreadmillEnable() {
    QSettings settings;
    bool cadence = settings.value(QZSettings::run_cadence_sensor, QZSettings::default_run_cadence_sensor).toBool();
    if (!cadence)
        return true;
    return false;
}

bool virtualtreadmill::RSCEnable() {
    QSettings settings;
    bool cadence = settings.value(QZSettings::run_cadence_sensor, QZSettings::default_run_cadence_sensor).toBool();
    if (cadence)
        return true;
    return false;
}

void virtualtreadmill::dirconChangedInclination(double grade, double percentage) {
    Q_UNUSED(grade);
    Q_UNUSED(percentage);
    slopeChanged();
}
