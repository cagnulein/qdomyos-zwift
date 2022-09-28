#include "eslinkertreadmill.h"
#include "keepawakehelper.h"
#include "virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

eslinkertreadmill::eslinkertreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService,
                                     double forceInitSpeed, double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0)
        lastSpeed = forceInitSpeed;

    if (forceInitInclination > 0)
        lastInclination = forceInitInclination;

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &eslinkertreadmill::update);
    refresh->start(500ms);
}

void eslinkertreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                            bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &eslinkertreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        // connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop,
        // &QEventLoop::quit); timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    gattCommunicationChannelService->writeCharacteristic(
        gattWriteCharacteristic, QByteArray((const char *)data, data_len), QLowEnergyService::WriteWithoutResponse);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    // packets sent from the characChanged event, i don't want to block everything
    if (wait_for_response) {
        loop.exec();

        if (timeout.isActive() == false)
            emit debug(QStringLiteral(" exit for timeout"));
    }
}

void eslinkertreadmill::updateDisplay(uint16_t elapsed) {
    if (treadmill_type == RHYTHM_FUN) {
        // trying to force a fixed value to keep the connection on
        uint8_t display[] = {0xa9, 0xa0, 0x03, 0x02, 0x23, 0x00, 0x2b};

        writeCharacteristic(display, sizeof(display),
                            QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, false);
    } else {
    }
}

void eslinkertreadmill::forceIncline(double requestIncline) {
    if (treadmill_type == CADENZA_FITNESS_T45) {
        uint8_t display[] = {0x04, 0x01, 0x00};
        display[2] = requestIncline;

        writeCharacteristic(display, sizeof(display),
                            QStringLiteral("forceIncline inclination=") + QString::number(requestIncline), false, true);
    }
}

double eslinkertreadmill::minStepInclination() { return 1.0; }

void eslinkertreadmill::forceSpeed(double requestSpeed) {
    if (treadmill_type == CADENZA_FITNESS_T45) {
        uint8_t display[] = {0x01, 0x01, 0x00};
        display[2] = requestSpeed * 10;

        writeCharacteristic(display, sizeof(display),
                            QStringLiteral("forceSpeed speed=") + QString::number(requestSpeed), false, true);
    }
}

void eslinkertreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    qDebug() << m_control->state() << bluetoothDevice.isValid() << gattCommunicationChannelService
             << gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone << treadmill_type
             << requestSpeed << requestInclination;

    if (initRequest) {
        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotifyCharacteristic.isValid() && initDone) {
        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !virtualTreadMill) {
            bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &eslinkertreadmill::debug);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        // it seems that stops the communication
        if (sec1Update++ >= (1000 / refresh->interval())) {
            updateDisplay(elapsed.value());
        }

        if (treadmill_type == TYPE::RHYTHM_FUN) {
            if (requestSpeed != -1) {
                if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                    emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                    // double inc = Inclination.value(); // NOTE: clang-analyzer-deadcode.DeadStores
                    if (requestInclination != -100) {
                        //                        inc = requestInclination;
                        requestInclination = -100;
                    }
                    forceSpeed(requestSpeed);
                }
                requestSpeed = -1;
            }
            if (requestInclination != -100) {
                if(requestInclination < 0)
                    requestInclination = 0;
                if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                    requestInclination <= 15) {
                    emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                    // double speed = currentSpeed().value(); // NOTE: clang-analyzer-deadcode.DeadStores
                    if (requestSpeed != -1) {
                        // speed = requestSpeed;
                        requestSpeed = -1;
                    }
                    forceIncline(requestInclination);
                }
                requestInclination = -100;
            }
        } else {
            if (requestVar2) {
                uint8_t display[] = {0x08, 0x04, 0x01, 0x00, 0x00, 0x01};
                writeCharacteristic(display, sizeof(display), QStringLiteral("var2"), false, true);
                requestHandshake = (1000 / refresh->interval());
                uint8_t display1[] = {0x09, 0x01, 0x01};
                writeCharacteristic(display1, sizeof(display1), QStringLiteral("speedslope"), false, true);
                uint8_t display2[] = {0x09, 0x01, 0x02};
                writeCharacteristic(display2, sizeof(display2), QStringLiteral("speedslope2"), false, true);
                requestVar2 = false;
            }
            if (requestHandshake) {
                requestHandshake--;
                if (!requestHandshake) {
                    uint8_t display[] = {0x08, 0x03, 0x00, 0x00, 0x00};
                    display[2] = 0;
                    display[3] = (uint8_t)(display[2] ^ 245);
                    display[4] = (uint8_t)(display[2] ^ 66);
                    writeCharacteristic(display, sizeof(display), QStringLiteral("var4"), false, true);
                }
            } else {
                // we need always to send values
                if (requestSpeed != -1 && requestInclination != -100) {
                    if (requestSpeed >= 0 && requestSpeed <= 20 && !toggleRequestSpeed) {
                        lastStart = 0;
                        forceSpeed(requestSpeed);
                        Speed = requestSpeed;
                        if (requestSpeed == 0)
                            lastStop = 0;
                    } else {
                        if (requestInclination > 18)
                            requestInclination = 18;
                        forceIncline(requestInclination);
                        Inclination = requestInclination;
                    }
                    toggleRequestSpeed = !toggleRequestSpeed;
                }
            }
        }

        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {
                lastSpeed = 0.5;
            }
            if (treadmill_type == TYPE::RHYTHM_FUN)
                btinit(true);
            requestSpeed = 1.0;
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            requestSpeed = 0;
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape", false, true);
            requestStop = -1;
        }
    }
}

void eslinkertreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void eslinkertreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    emit packetReceived();

    if (treadmill_type == CADENZA_FITNESS_T45) {
        if (newValue.length() == 6 && newValue.at(0) == 8 && newValue.at(1) == 4 && newValue.at(2) == 1 &&
            newValue.at(3) == 0 && newValue.at(4) == 0 && newValue.at(5) == 1) {
            if (requestSpeed == -1 || requestInclination == -100) {
                requestSpeed = 0;
                requestInclination = 0;
                qDebug() << QStringLiteral("we can start send force commands");
            }
            requestVar2 = true;
        } else if (newValue.length() == 3 && newValue.at(0) == 8 && newValue.at(1) == 1 && newValue.at(2) == -1) {
            uint8_t display[] = {0x08, 0x01, 0x01};

            writeCharacteristic(display, sizeof(display), QStringLiteral("var1"), false, false);
        } else if (newValue.length() == 3 && newValue.at(0) == 5 && newValue.at(1) == 1 && newValue.at(2) == 1) {
            if (requestSpeed <= 0)
                requestSpeed = 1;
            qDebug() << QStringLiteral("Start received!");
            lastStart = QDateTime::currentMSecsSinceEpoch();
        } else if (newValue.length() == 3 && newValue.at(0) == 5 && newValue.at(1) == 1 && newValue.at(2) == 2) {
            requestSpeed = 0;
            qDebug() << QStringLiteral("Stop received!");
            lastStop = QDateTime::currentMSecsSinceEpoch();
        } else if (newValue.length() == 3 && newValue.at(0) == 5 && newValue.at(1) == 1 && newValue.at(2) == 5) {
            requestInclination += 1;
            qDebug() << QStringLiteral("Elevation UP received!");
        } else if (newValue.length() == 3 && newValue.at(0) == 5 && newValue.at(1) == 1 && newValue.at(2) == 6) {
            if (requestInclination >= 1)
                requestInclination -= 1;
            else
                requestInclination = 0;
            qDebug() << QStringLiteral("Elevation DOWN received!");
        } else if (newValue.length() == 3 && newValue.at(0) == 5 && newValue.at(1) == 1 && newValue.at(2) == 3) {
            requestSpeed += 0.5;
            qDebug() << QStringLiteral("Speed UP received!");
        } else if (newValue.length() == 3 && newValue.at(0) == 5 && newValue.at(1) == 1 && newValue.at(2) == 4) {
            if (requestSpeed >= 0.5)
                requestSpeed -= 0.5;
            else
                requestSpeed = 0;
            qDebug() << QStringLiteral("Speed DOWN received!");
        } else if (newValue.length() == 3 && newValue.at(0) == 8 && newValue.at(1) == 1) {
            uint8_t display[] = {0x08, 0x01, 0x01};
            if (requestSpeed == -1 || requestInclination == -100) {
                requestSpeed = 0;
                requestInclination = 0;
                qDebug() << QStringLiteral("we can start send force commands");
            }
            writeCharacteristic(display, sizeof(display), QStringLiteral("var1"), false, false);
        } else if (newValue.length() == 5 && newValue.at(0) == 8 && newValue.at(1) == 3 &&
                   (uint8_t)newValue.at(3) == (uint8_t)(newValue.at(2) ^ 245) &&
                   (uint8_t)newValue.at(4) == (uint8_t)(newValue.at(2) ^ 222)) {
            uint8_t display[] = {0x08, 0x03, 0x00, 0x00, 0x00};
            display[2] = newValue.at(4);
            display[3] = (uint8_t)(newValue.at(4) ^ 245);
            display[4] = (uint8_t)(newValue.at(4) ^ 66);
            writeCharacteristic(display, sizeof(display), QStringLiteral("var4"), false, false);
        } else if (newValue.length() == 4 && newValue.at(0) == 8 && newValue.at(1) == 2) {
            if (requestSpeed == -1 || requestInclination == -100) {
                qDebug() << QStringLiteral("we can start send force commands");
                requestSpeed = 0;
                requestInclination = 0;
            }
        } else if (newValue.length() == 3 && newValue.at(0) == 2 && newValue.at(1) == 1) {
            uint8_t heart = newValue.at(2);
#ifdef Q_OS_ANDROID
            if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
                Heart = (uint8_t)KeepAwakeHelper::heart();
            else
#endif
            {
                if (heartRateBeltName.startsWith("Disabled"))
                    Heart = heart;
            }
            emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
        }
    }

    if ((newValue.length() != 17 && treadmill_type == RHYTHM_FUN))
        return;

    if (treadmill_type == RHYTHM_FUN) {
        double speed = GetSpeedFromPacket(value);
        double incline = GetInclinationFromPacket(value);
        double kcal = GetKcalFromPacket(value);
        // double distance = GetDistanceFromPacket(value);

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            /*if(heartRateBeltName.startsWith("Disabled"))
            Heart = value.at(18);*/
        }
        emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
        emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
        emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
        // debug("Current Distance: " + QString::number(distance));

        if (Speed.value() != speed) {
            emit speedChanged(speed);
        }
        Speed = speed;
        if (Inclination.value() != incline) {
            emit inclinationChanged(0.0, incline);
        }
        Inclination = incline;

        KCal = kcal;
        // Distance = distance;

        if (speed > 0) {
            lastSpeed = speed;
            lastInclination = incline;
        }
    }

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        Distance += ((Speed.value() / 3600.0) /
                     (1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }

    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
}

double eslinkertreadmill::GetSpeedFromPacket(const QByteArray &packet) {
    uint8_t convertedData = (uint8_t)packet.at(14);
    double data = (double)convertedData / 10.0f;
    return data;
}

double eslinkertreadmill::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return (double)convertedData;
}

double eslinkertreadmill::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

double eslinkertreadmill::GetInclinationFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(11);
    double data = convertedData;

    return data;
}

void eslinkertreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)
    // set speed and incline to 0
    uint8_t initData1[] = {0x08, 0x01, 0x86};
    uint8_t initData2[] = {0xa9, 0x08, 0x01, 0x86, 0x26};
    uint8_t initData3[] = {0xa9, 0x80, 0x05, 0x05, 0xb0, 0x04, 0x52, 0xa9, 0x66};
    uint8_t initData4[] = {0xa9, 0x08, 0x04, 0xb2, 0x51, 0x03, 0x52, 0x17};
    uint8_t initData5[] = {0xa9, 0x1e, 0x01, 0xfe, 0x48};
    uint8_t initData6[] = {0xa9, 0x0a, 0x01, 0x01, 0xa3};
    uint8_t initData7[] = {0xa9, 0xf0, 0x01, 0x01, 0x59};
    uint8_t initData8[] = {0xa9, 0xa0, 0x03, 0xff, 0x00, 0x00, 0xf5};
    uint8_t initData9[] = {0xa9, 0xa0, 0x03, 0x00, 0x00, 0x00, 0x0a};
    uint8_t initData10[] = {0xa9, 0xa0, 0x03, 0x01, 0x00, 0x00, 0x0b};
    uint8_t initData11[] = {0xa9, 0x01, 0x01, 0x08, 0xa1};
    uint8_t initData12[] = {0xa9, 0xa0, 0x03, 0x02, 0x08, 0x00, 0x00};

    uint8_t initData2_CADENZA[] = {0x08, 0x01, 0x01};

    if (treadmill_type == RHYTHM_FUN) {
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, true);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, true);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, true);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, true);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, true);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, true);
    } else {
        writeCharacteristic(initData2_CADENZA, sizeof(initData2_CADENZA), QStringLiteral("init"), false, true);
    }

    initDone = true;
}

void eslinkertreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::ServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
        QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xfff1);

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &eslinkertreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &eslinkertreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &eslinkertreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &eslinkertreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void eslinkertreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void eslinkertreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void eslinkertreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &eslinkertreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void eslinkertreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("eslinkertreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void eslinkertreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("eslinkertreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void eslinkertreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &eslinkertreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &eslinkertreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &eslinkertreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &eslinkertreadmill::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
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

        QSettings settings;
        bool eslinker_cadenza = settings.value(QZSettings::eslinker_cadenza, QZSettings::default_eslinker_cadenza).toBool();
        if (eslinker_cadenza) {
            treadmill_type = CADENZA_FITNESS_T45;
        } else
            treadmill_type = RHYTHM_FUN;

        // Connect
        m_control->connectToDevice();
        return;
    }
}

void eslinkertreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool eslinkertreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *eslinkertreadmill::VirtualTreadMill() { return virtualTreadMill; }

void *eslinkertreadmill::VirtualDevice() { return VirtualTreadMill(); }

bool eslinkertreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool eslinkertreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
