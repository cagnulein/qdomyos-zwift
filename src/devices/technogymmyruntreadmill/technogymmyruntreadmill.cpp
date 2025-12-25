#include "technogymmyruntreadmill.h"

#include "devices/ftmsbike/ftmsbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif

#include <chrono>

using namespace std::chrono_literals;

technogymmyruntreadmill::technogymmyruntreadmill(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &technogymmyruntreadmill::update);
    refresh->start(200ms);
}

void technogymmyruntreadmill::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                                  uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                                  bool wait_for_response, QLowEnergyService::WriteMode writeMode) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &technogymmyruntreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(3000, &loop, SLOT(quit()));
    } else {
        connect(service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(3000, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    service->writeCharacteristic(characteristic, *writeBuffer, writeMode);

    if (!disable_log)
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;

    loop.exec();
}

void technogymmyruntreadmill::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &technogymmyruntreadmill::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

void technogymmyruntreadmill::btinit() {

    if (gattFTMSService) {
        uint8_t writeS[] = {0x00, 0x93, 0xf0, 0x51, 0xe8, 0x1b, 0x42, 0x92, 0x8e};

        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                            QStringLiteral("start"), false, true);
    }

    // disable pace
    uint8_t init1[] = {0x40, 0x44, 0x49, 0x53, 0x41, 0x42, 0x4c, 0x45, 0x5f, 0x50, 0x41, 0x43, 0x45, 0x23};
    if (gattCustomService) {
        for (uint i = 0; i < sizeof(init1); i++)
            writeCharacteristic(gattCustomService, gattWriteCustomCharacteristic, &init1[i], 1, QStringLiteral("init1"),
                                false, false);
    }

    // ssi units
    uint8_t init2[] = {0x40, 0x53, 0x53, 0x49, 0x55, 0x4e, 0x49, 0x54, 0x53, 0x23};
    if (gattCustomService) {
        for (uint i = 0; i < sizeof(init2); i++)
            writeCharacteristic(gattCustomService, gattWriteCustomCharacteristic, &init2[i], 1, QStringLiteral("init2"),
                                false, false);
    }

    // rjks en 1
    uint8_t init3[] = {0x40, 0x52, 0x4a, 0x53, 0x4b, 0x5f, 0x45, 0x4e, 0x20, 0x31, 0x23};
    if (gattCustomService) {
        for (uint i = 0; i < sizeof(init3); i++)
            writeCharacteristic(gattCustomService, gattWriteCustomCharacteristic, &init3[i], 1, QStringLiteral("init3"),
                                false, false);
    }

    // ljks en 1
    uint8_t init4[] = {0x40, 0x4c, 0x4a, 0x53, 0x4b, 0x5f, 0x45, 0x4e, 0x20, 0x31, 0x23};
    if (gattCustomService) {
        for (uint i = 0; i < sizeof(init4); i++)
            writeCharacteristic(gattCustomService, gattWriteCustomCharacteristic, &init4[i], 1, QStringLiteral("init4"),
                                false, false);
    }

    /*
    if (gattFTMSService) {
        uint8_t writeS[] = {FTMS_START_RESUME};

        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                            QStringLiteral("start"), false, false);
    }*/

    if (gattWeightService) {
        uint8_t writeS[] = {0x30, 0x43};

        writeCharacteristic(gattWeightService, gattWriteCharWeight, writeS, sizeof(writeS), QStringLiteral("weight"),
                            false, true);
    }

    // set date (maybe useless?)
    uint8_t init5[] = {0x40, 0x53, 0x45, 0x54, 0x44, 0x41, 0x54, 0x45, 0x20, 0x31, 0x33, 0x20, 0x30, 0x31, 0x20,
                       0x32, 0x30, 0x32, 0x32, 0x20, 0x31, 0x39, 0x20, 0x33, 0x31, 0x20, 0x34, 0x35, 0x23};
    if (gattCustomService) {
        for (uint i = 0; i < sizeof(init5); i++)
            writeCharacteristic(gattCustomService, gattWriteCustomCharacteristic, &init5[i], 1, QStringLiteral("init5"),
                                false, false);
    }

    initDone = true;
}

bool technogymmyruntreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

void technogymmyruntreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest && firstStateChanged) {
        btinit();
        initRequest = false;
    } else if (bluetoothDevice.isValid() //&&

               // m_control->state() == QLowEnergyController::DiscoveredState //&&
               // gattCommunicationChannelService &&
               // gattWriteCharacteristic.isValid() &&
               // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {

        QSettings settings;
        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            if (requestInclination < 0)
                requestInclination = 0;
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                forceIncline(requestInclination);
            }
            requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {

                lastSpeed = 0.5;
            }
            lastStart = QDateTime::currentMSecsSinceEpoch();
            if (gattFTMSService) {
                uint8_t writeS[] = {FTMS_START_RESUME};

                writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                                    QStringLiteral("start"), false, false);
            }
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

            if (gattFTMSService) {
                uint8_t writeS[] = {FTMS_STOP_PAUSE, 0x01};

                writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                                    QStringLiteral("stop"), false, true);
            }

            lastStop = QDateTime::currentMSecsSinceEpoch();

            requestStop = -1;
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

void technogymmyruntreadmill::forceSpeed(double requestSpeed) {
    if (gattFTMSService) {
        uint8_t writeS[] = {FTMS_SET_TARGET_SPEED, 0x00, 0x00};
        writeS[1] = (uint16_t)(requestSpeed * 100) & 0xFF;
        writeS[2] = (uint16_t)(requestSpeed * 100) >> 8;

        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                            QStringLiteral("forceSpeed"), false, true);
    }
}

void technogymmyruntreadmill::forceIncline(double requestIncline) {
    if (gattFTMSService) {
        uint8_t writeS[] = {FTMS_SET_TARGET_INCLINATION, 0x00, 0x00};
        writeS[1] = (int16_t)(requestIncline * 10) & 0xFF;
        writeS[2] = (int16_t)(requestIncline * 10) >> 8;

        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                            QStringLiteral("forceIncline"), false, true);
    }
}

void technogymmyruntreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void technogymmyruntreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                    const QByteArray &newValue) {
    double heart = 0; // NOTE : Should be initialized with a value to shut clang-analyzer's
                      // UndefinedBinaryOperatorResult
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2AD9))
        emit packetReceived();

    emit debug(QStringLiteral(" << ") + characteristic.uuid().toString() + " " + QString::number(newValue.length()) +
               " " + newValue.toHex(' ') + newValue);

    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACD)) {
        lastPacket = newValue;

        // default flags for this treadmill is 84 04

        union flags {
            struct {

                uint16_t moreData : 1;
                uint16_t avgSpeed : 1;
                uint16_t totalDistance : 1;
                uint16_t inclination : 1;
                uint16_t elevation : 1;
                uint16_t instantPace : 1;
                uint16_t averagePace : 1;
                uint16_t expEnergy : 1;
                uint16_t heartRate : 1;
                uint16_t metabolic : 1;
                uint16_t elapsedTime : 1;
                uint16_t remainingTime : 1;
                uint16_t forceBelt : 1;
                uint16_t spare : 3;
            };

            uint16_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {
            Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                    100.0;

            if (Speed.value() > 0)
                lastStart = 0;
            else
                lastStop = 0;

            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.totalDistance) {
            // ignoring the distance, because it's a total life odometer
            // Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
            // (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
            index += 3;
        }
        // else
        {
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.inclination) {
            Inclination = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          10.0;
            index += 4; // the ramo value is useless
            emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        }

        if (Flags.elevation) {
            index += 4; // TODO
        }

        if (Flags.instantPace) {
            index += 1; // TODO
        }

        if (Flags.averagePace) {
            index += 1; // TODO
        }

        if (Flags.expEnergy) {
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal +=
                    ((((0.048 *
                            ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged.msecsTo(
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate) {
                if (index < newValue.length()) {

                    heart = ((double)(((uint8_t)newValue.at(index))));
                    emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
                } else {
                    emit debug(QStringLiteral("Error on parsing heart!"));
                }
                // index += 1; //NOTE: clang-analyzer-deadcode.DeadStores
            }
        }

        if (Flags.metabolic) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }

        if (Flags.forceBelt) {
            // todo
        }

        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    } else if (characteristic.uuid() == QBluetoothUuid::CharacteristicType::RSCMeasurement) {
        uint8_t flags = (uint8_t)newValue.at(0);
        bool InstantaneousStrideLengthPresent = (flags & 0x01);
        bool TotalDistancePresent = (flags & 0x02) ? true : false;
        bool WalkingorRunningStatusbits = (flags & 0x04) ? true : false;
        bool double_cadence = settings
                                  .value(QZSettings::powr_sensor_running_cadence_double,
                                         QZSettings::default_powr_sensor_running_cadence_double)
                                  .toBool();
        double cadence_multiplier = 1.0;
        if (double_cadence)
            cadence_multiplier = 2.0;

        // Unit is in m/s with a resolution of 1/256
        uint16_t speedMs = (((uint16_t)((uint8_t)newValue.at(2)) << 8) | (uint16_t)((uint8_t)newValue.at(1)));
        double speed = (((double)speedMs) / 256.0) * 3.6; // km/h
        double cadence = (uint8_t)newValue.at(3) * cadence_multiplier;

        Cadence = cadence;
        emit cadenceChanged(cadence);
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(cadence));
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0 ||
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool()) {
            update_hr_from_external();
        } else {
            Heart = heart;
        }
    }

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void technogymmyruntreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::RemoteServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::RemoteServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this,
                    &technogymmyruntreadmill::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this,
                    &technogymmyruntreadmill::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &technogymmyruntreadmill::characteristicRead);
            connect(
                s, &QLowEnergyService::errorOccurred,
                this, &technogymmyruntreadmill::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &technogymmyruntreadmill::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &technogymmyruntreadmill::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("FTMS service and Control Point found");
                    gattWriteCharControlPointId = c;
                    gattFTMSService = s;
                }

                QBluetoothUuid _gattWriteCustomChar(QStringLiteral("df1eb8e4-1753-4bb9-a6a6-e018040af0a3"));
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCustomChar) {
                    qDebug() << QStringLiteral("Custom service found");
                    gattWriteCustomCharacteristic = c;
                    gattCustomService = s;
                }

                QBluetoothUuid _gattWriteWeigthChar(QStringLiteral("00002a98-0000-1000-8000-00805f9b34fb"));
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteWeigthChar) {
                    qDebug() << QStringLiteral("Weight service found");
                    gattWriteCharWeight = c;
                    gattWeightService = s;
                }
            }
        }
    }

    initRequest = false;
    emit connectedAndDiscovered();

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {

        QSettings settings;
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        bool virtual_device_force_bike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                .toBool();
        if (virtual_device_enabled) {
            if (!virtual_device_force_bike) {
                debug("creating virtual treadmill interface...");
                auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &technogymmyruntreadmill::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &technogymmyruntreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                debug("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &technogymmyruntreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************
    }
}

void technogymmyruntreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void technogymmyruntreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void technogymmyruntreadmill::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void technogymmyruntreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                    const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void technogymmyruntreadmill::characteristicRead(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void technogymmyruntreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &technogymmyruntreadmill::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void technogymmyruntreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("technogymmyruntreadmill::errorService") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void technogymmyruntreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("technogymmyruntreadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void technogymmyruntreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    // ***************************************************************************************************************
    // horizon treadmill and F80 treadmill, so if we want to add inclination support we have to separate the 2
    // devices
    // ***************************************************************************************************************
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &technogymmyruntreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &technogymmyruntreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &technogymmyruntreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &technogymmyruntreadmill::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("Controller connected. Search services..."));
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("LowEnergy controller disconnected"));
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool technogymmyruntreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void technogymmyruntreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool technogymmyruntreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
