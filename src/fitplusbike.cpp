#include "fitplusbike.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

fitplusbike::fitplusbike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &fitplusbike::update);
    refresh->start(200ms);
}

void fitplusbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                      bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
    // one for the resistance changed event (spontaneous), and one for the other ones.
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic,
                                                         QByteArray((const char *)data, data_len));

    if (!disable_log)
        qDebug() << QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
                        QStringLiteral(" // ") + info;

    loop.exec();
}

void fitplusbike::forceResistance(resistance_t requestResistance) {
    QSettings settings;
    bool virtufit_etappe = settings.value(QStringLiteral("virtufit_etappe"), false).toBool();
    if (virtufit_etappe) {
        if (requestResistance == 1) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x01, 0xf9, 0xb9, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 2) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x02, 0xf9, 0xba, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 3) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x03, 0xfa, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 4) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x04, 0xfb, 0xbe, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 5) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x05, 0xfc, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 6) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x06, 0xfd, 0xba, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 7) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x07, 0xfe, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 8) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x08, 0xff, 0xb6, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 9) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x09, 0x00, 0x48, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 10) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0a, 0x00, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 11) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0b, 0x01, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 12) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0c, 0x03, 0x4e, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 13) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0d, 0x03, 0x4f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 14) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0e, 0x05, 0x4a, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 15) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0f, 0x05, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 16) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x10, 0x06, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 17) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x11, 0x07, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 18) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x12, 0x08, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 19) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x13, 0x09, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 20) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x14, 0x0a, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 21) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x15, 0x0b, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 22) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x16, 0x0c, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 23) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x17, 0x0d, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 24) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x18, 0x0e, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        }
    }
}

void fitplusbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        QSettings settings;
        update_metrics(true, watts());
        bool virtufit_etappe = settings.value(QStringLiteral("virtufit_etappe"), false).toBool();

        if (virtufit_etappe) {

        } else {

            if (Heart.value() > 0) {
                int avgP = ((settings.value(QStringLiteral("power_hr_pwr1"), 200).toDouble() *
                             settings.value(QStringLiteral("power_hr_hr2"), 170).toDouble()) -
                            (settings.value(QStringLiteral("power_hr_pwr2"), 230).toDouble() *
                             settings.value(QStringLiteral("power_hr_hr1"), 150).toDouble())) /
                               (settings.value(QStringLiteral("power_hr_hr2"), 170).toDouble() -
                                settings.value(QStringLiteral("power_hr_hr1"), 150).toDouble()) +
                           (Heart.value() * ((settings.value(QStringLiteral("power_hr_pwr1"), 200).toDouble() -
                                              settings.value(QStringLiteral("power_hr_pwr2"), 230).toDouble()) /
                                             (settings.value(QStringLiteral("power_hr_hr1"), 150).toDouble() -
                                              settings.value(QStringLiteral("power_hr_hr2"), 170).toDouble())));
                if (avgP < 50) {
                    avgP = 50;
                }
                m_watt = avgP;
                qDebug() << QStringLiteral("Current Watt: ") + QString::number(m_watt.value());
            }
        }

        // sending poll every 2 seconds
        if (sec1Update++ >= (2000 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        uint8_t noOpData[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t noOpData1[] = {0x02, 0x43, 0x01, 0x42, 0x03};
        switch (counterPoll) {
        case 0:
            writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
            break;
        default:
            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
            break;
        }

        counterPoll++;
        if (counterPoll > 1) {
            counterPoll = 0;
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance) {
                requestResistance = max_resistance;
            } else if (requestResistance <= 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                qDebug() << QStringLiteral("writing resistance ") + QString::number(requestResistance);
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestStart != -1) {
            qDebug() << QStringLiteral("starting...");

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            qDebug() << QStringLiteral("stopping...");
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void fitplusbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void fitplusbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();

    qDebug() << QStringLiteral(" << ") + newValue.toHex(' ');

    lastPacket = newValue;

    bool virtufit_etappe = settings.value(QStringLiteral("virtufit_etappe"), false).toBool();
    if (virtufit_etappe) {
        if (newValue.length() != 15 && newValue.length() != 13)
            return;

        if (newValue.length() == 15) {
            Resistance = newValue.at(5);
            m_pelotonResistance = (100 * Resistance.value()) / max_resistance;

            if (settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled"))
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                Cadence = ((uint8_t)newValue.at(6));
            m_watt = (double)((((uint8_t)newValue.at(4)) << 8) | ((uint8_t)newValue.at(3))) / 10.0;

            /*if (!settings.value(QStringLiteral("speed_power_based"), false).toBool())
                Speed = (double)((((uint8_t)newValue.at(4)) << 10) | ((uint8_t)newValue.at(9))) / 100.0;
            else*/
            Speed = metric::calculateSpeedFromPower(m_watt.value(),  Inclination.value());

        } else if (newValue.length() == 13) {

            return;
        }

    } else {

        if (newValue.length() != 14) {
            return;
        }

        /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
            return;*/

        Resistance = 1;
        m_pelotonResistance = 1;
        emit resistanceRead(Resistance.value());
        if (settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled"))
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            Cadence = ((uint8_t)newValue.at(8));
        if (!settings.value(QStringLiteral("speed_power_based"), false).toBool())
            Speed = (double)((((uint8_t)newValue.at(7)) << 8) | ((uint8_t)newValue.at(6))) / 10.0;
        else
            Speed = metric::calculateSpeedFromPower(m_watt.value(),  Inclination.value());
    }

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) * settings.value(QStringLiteral("weight"), 75.0).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
    if (settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen h;
            long appleWatchHeartRate = h.heartRate();
            h.setKcal(KCal.value());
            h.setDistance(Distance.value());
            Heart = appleWatchHeartRate;
            qDebug() << "Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate);
#endif
#endif
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value("bike_cadence_sensor", false).toBool();
    bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void fitplusbike::btinit() {

    QSettings settings;
    bool virtufit_etappe = settings.value(QStringLiteral("virtufit_etappe"), false).toBool();

    if (virtufit_etappe) {
        uint8_t initData1[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t initData2[] = {0x02, 0x41, 0x02, 0x43, 0x03};
        uint8_t initData3[] = {0x02, 0x41, 0x03, 0x42, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData5[] = {0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xaa, 0x18, 0x00, 0xc0, 0x03};
        uint8_t initData6[] = {0x02, 0x44, 0x0b, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x03};
        uint8_t initData7[] = {0x02, 0x44, 0x02, 0x46, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);

        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);

    } else {

        uint8_t initData1[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData2[] = {0x02, 0x44, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x03};
        uint8_t initData3[] = {0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x46, 0xaa, 0x19, 0x00, 0xbb, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x02, 0x46, 0x03};
        uint8_t initData5[] = {0x02, 0x41, 0x02, 0x43, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
    }

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void fitplusbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &fitplusbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &fitplusbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &fitplusbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &fitplusbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!firstStateChanged && !virtualBike
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            && !h
#endif
#endif
        ) {
            QSettings settings;
            bool virtual_device_enabled = settings.value(QStringLiteral("virtual_device_enabled"), true).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence = settings.value("bike_cadence_sensor", false).toBool();
            bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
            if (ios_peloton_workaround && cadence) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                qDebug() << QStringLiteral("creating virtual bike interface...");
                virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&fitplusbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &fitplusbike::changeInclination);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void fitplusbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void fitplusbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void fitplusbike::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &fitplusbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void fitplusbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("fitplusbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void fitplusbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("fitplusbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void fitplusbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')';
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &fitplusbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &fitplusbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &fitplusbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &fitplusbike::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    qDebug() << QStringLiteral("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool fitplusbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *fitplusbike::VirtualBike() { return virtualBike; }

void *fitplusbike::VirtualDevice() { return VirtualBike(); }

uint16_t fitplusbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void fitplusbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
