#include "spirittreadmill.h"
#include "virtualdevices/virtualbike.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

spirittreadmill::spirittreadmill() {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &spirittreadmill::update);
    refresh->start(200ms);
}

void spirittreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &spirittreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void spirittreadmill::forceSpeed(double requestSpeed) {
    if (!XT385) {
        if (requestSpeed > Speed.value()) {
            uint8_t increaseSpeed[] = {0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d};
            writeCharacteristic(increaseSpeed, sizeof(increaseSpeed), QStringLiteral("increaseSpeed"), false, true);
        } else {
            uint8_t decreaseSpeed[] = {0x5b, 0x04, 0x00, 0x32, 0x4f, 0x4b, 0x5d};
            writeCharacteristic(decreaseSpeed, sizeof(decreaseSpeed), QStringLiteral("decreaseSpeed"), false, true);
        }
    } else {
        uint8_t increase[] = {0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d};
        if (requestSpeed > Speed.value()) {
            uint8_t increaseSpeed[] = {0x5b, 0x02, 0xF1, 0x02, 0x5d};
            writeCharacteristic(increaseSpeed, sizeof(increaseSpeed), QStringLiteral("increaseSpeed"), false, true);
            writeCharacteristic(increase, sizeof(increase), QStringLiteral("increaseSpeed"), false, true);
        } else {
            uint8_t decreaseSpeed[] = {0x5b, 0x02, 0xF1, 0x03, 0x5d};
            writeCharacteristic(decreaseSpeed, sizeof(decreaseSpeed), QStringLiteral("decreaseSpeed"), false, true);
            writeCharacteristic(increase, sizeof(increase), QStringLiteral("decreaseSpeed"), false, true);
        }
    }
}

void spirittreadmill::forceIncline(double requestIncline) {
    if (XT385) {
        uint8_t increase[] = {0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d};
        if (requestIncline > Inclination.value()) {
            if (requestInclinationState == IDLE)
                requestInclinationState = UP;
            else if (requestInclinationState == DOWN) {
                requestInclinationState = IDLE;
                this->requestInclination = -100;
                return;
            }
            uint8_t increaseSpeed[] = {0x5b, 0x02, 0xF1, 0x04, 0x5d};
            writeCharacteristic(increaseSpeed, sizeof(increaseSpeed), QStringLiteral("increaseIncline"), false, true);
            writeCharacteristic(increase, sizeof(increase), QStringLiteral("increaseIncline"), false, true);

        } else if (requestIncline < Inclination.value()) {
            if (requestInclinationState == IDLE)
                requestInclinationState = DOWN;
            else if (requestInclinationState == UP) {
                requestInclinationState = IDLE;
                this->requestInclination = -100;
                return;
            }
            uint8_t decreaseSpeed[] = {0x5b, 0x02, 0xF1, 0x05, 0x5d};
            writeCharacteristic(decreaseSpeed, sizeof(decreaseSpeed), QStringLiteral("decreaseIncline"), false, true);
            writeCharacteristic(increase, sizeof(increase), QStringLiteral("decreaseIncline"), false, true);

        } else {
            this->requestInclination = -100;
            requestInclinationState = IDLE;
        }
    }
}

void spirittreadmill::update() {
    // qDebug() << treadmill.isValid() << m_control->state() << gattCommunicationChannelService <<
    // gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if (!m_control) {
        return;
    }

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit(false);
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotifyCharacteristic.isValid() && initDone) {
        QSettings settings;
        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1update++ == (1000 / refresh->interval())) {
            sec1update = 0;
            // updateDisplay(elapsed);
        } else {
            if (!XT385 && !XT485) {
                uint8_t noOpData[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
                uint8_t noOpData1[] = {0x5b, 0x04, 0x00, 0x40, 0x4f, 0x4b, 0x5d};

                switch (counterPoll) {
                case 0:
                    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
                    break;
                case 1:
                    writeCharacteristic(noOpData, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
                    break;
                }
            } else {
                uint8_t noOpData[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
                uint8_t noOpData1[] = {0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d};

                switch (counterPoll) {
                case 0:
                    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
                    break;
                case 1:
                    writeCharacteristic(noOpData, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
                    break;
                }
            }
            counterPoll++;
            if (counterPoll > 1) {
                counterPoll = 0;
            }
        }

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value()) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            if (requestInclination < 0)
                requestInclination = 0;
            double inc = qRound(requestInclination / 0.5) * 0.5;
            // this treadmill has 0.5% step inclination
            if (inc != currentInclination().value() && inc >= 0 && inc <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(inc));
                forceIncline(inc);
            } else if (inc == currentInclination().value()) {
                qDebug() << "int inclination match the current one" << inc << currentInclination().value();
                requestInclination = -100;
            }
            // i have to do the reset on when the inclination is equal to the current
            // requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            requestStart = -1;
            emit tapeStarted();
            if (XT385) {
                uint8_t start[] = {0x5b, 0x02, 0x03, 0x04, 0x5d};
                writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
                writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
            } else if (XT485) {
                uint8_t start[] = {0x5b, 0x02, 0x03, 0x03, 0x5d};
                writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
                writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
            }
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;

            if (XT385 || XT485) {
                uint8_t stop[] = {0x5b, 0x02, 0xf1, 0x06, 0x5d};
                writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
                writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
                writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
                writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
            }
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));
            changeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));
            changeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

void spirittreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void spirittreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;
    if (newValue.length() != 18) {
        return;
    }

    double speed = GetSpeedFromPacket(newValue);
    Inclination = GetInclinationFromPacket(newValue);
    double kcal = GetKcalFromPacket(newValue);
    // double distance = GetDistanceFromPacket(newValue) *
    // settings.value(QZSettings::domyos_elliptical_speed_ratio,
    // QZSettings::default_domyos_elliptical_speed_ratio).toDouble();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        /*if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            Heart = ((uint8_t)newValue.at(18));
        }*/
    }

    Distance += ((Speed.value() / 3600000.0) * ((double)lastTimeCharChanged.msecsTo(QTime::currentTime())));

    cadenceFromAppleWatch();

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current inclination: ") + QString::number(Inclination.value()));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current Watt: ") +
               QString::number(watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    lastTimeCharChanged = QTime::currentTime();

    Speed = speed;
    KCal = kcal;
}

uint16_t spirittreadmill::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    return convertedData;
}

double spirittreadmill::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (uint8_t)packet.at(10);
    double data = (double)convertedData / 10.0f;
    return data;
}

double spirittreadmill::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | ((uint8_t)packet.at(8));
    return (double)convertedData / 10.0;
}

double spirittreadmill::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

double spirittreadmill::GetInclinationFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(11);
    double data = (double)convertedData / 10.0f;
    return data;
}

void spirittreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)
    if (XT485) {
        // set speed and incline to 0
        uint8_t initData1[] = {0x5b, 0x01, 0xf0, 0x5d};
        uint8_t initData2[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
        uint8_t initData3[] = {0x5b, 0x02, 0x03, 0x01, 0x5d};
        uint8_t initData4[] = {0x5b, 0x04, 0x00, 0x09, 0x4f, 0x4b, 0x5d};
        uint8_t initData5[] = {0x5b, 0x06, 0x07, 0x01, 0x2c, 0x00, 0x61, 0x59, 0x5d};
        uint8_t initData6[] = {0x5b, 0x03, 0x08, 0x20, 0x04, 0x5d};
        uint8_t initData7[] = {0x5b, 0x05, 0x04, 0x0a, 0x00, 0x00, 0x00, 0x5d};
        uint8_t initData8[] = {0x5b, 0x02, 0x22, 0x09, 0x5d};
        uint8_t initData9[] = {0x5b, 0x02, 0x02, 0x02, 0x5d};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData4, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, true);
    } else if (!XT385) {
        // set speed and incline to 0
        uint8_t initData1[] = {0x5b, 0x01, 0xf0, 0x5d};
        uint8_t initData2[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
        uint8_t initData3[] = {0x5b, 0x02, 0x03, 0x01, 0x5d};
        uint8_t initData4[] = {0x5b, 0x04, 0x00, 0x09, 0x4f, 0x4b, 0x5d};
        uint8_t initData5[] = {0x5b, 0x06, 0x07, 0x00, 0x23, 0x00, 0x84, 0x40, 0x5d};
        uint8_t initData6[] = {0x5b, 0x03, 0x08, 0x10, 0x01, 0x5d};
        uint8_t initData7[] = {0x5b, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x5d};
        uint8_t initData8[] = {0x5b, 0x02, 0x22, 0x09, 0x5d};
        uint8_t initData9[] = {0x5b, 0x02, 0x02, 0x02, 0x5d};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, true);
    } else {
        uint8_t initData1[] = {0x5b, 0x01, 0xf0, 0x5d};
        uint8_t initData2[] = {0x5b, 0x02, 0x03, 0x01, 0x5d};
        uint8_t initData3[] = {0x5b, 0x04, 0x00, 0x09, 0x4f, 0x4b, 0x5d};
        uint8_t initData4[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
        uint8_t initData5[] = {0x5b, 0x06, 0x07, 0x01, 0x23, 0x00, 0x9b, 0xaa, 0x5d};
        uint8_t initData6[] = {0x5b, 0x03, 0x08, 0x10, 0x01, 0x5d};
        uint8_t initData7[] = {0x5b, 0x05, 0x04, 0x62, 0x00, 0x00, 0x00, 0x5d};
        uint8_t initData8[] = {0x5b, 0x02, 0x22, 0x09, 0x5d};
        uint8_t initData9[] = {0x5b, 0x02, 0x02, 0x02, 0x5d};
        uint8_t initData10[] = {0x5b, 0x02, 0x03, 0x04, 0x5d};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, true);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, true);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, true);
    }

    initDone = true;
}

void spirittreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            emit debug(QStringLiteral("characteristic ") + c.uuid().toString());
        }

        //        QString uuidWrite = "49535343-8841-43f4-a8d4-ecbe34729bb3";
        //        QString uuidNotify = "49535343-1E4D-4BD9-BA61-23C647249616";

        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
        QBluetoothUuid _gattNotifyCharacteristicId(QStringLiteral("49535343-1E4D-4BD9-BA61-23C647249616"));

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &spirittreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &spirittreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &spirittreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &spirittreadmill::descriptorWritten);

        // ******************************************* virtual treadmill init *************************************
        if (!firstVirtualTreadmill && !this->hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadMill = new virtualtreadmill(this, false);
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &spirittreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &spirittreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &spirittreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
            }
        }
        firstVirtualTreadmill = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void spirittreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    if (grade < 0)
        grade = 0;
    changeInclination(grade, percentage);
}

void spirittreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void spirittreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void spirittreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    // QString uuid = "49535343-FE7D-4AE5-8FA9-9FAFD205E455";

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("49535343-FE7D-4AE5-8FA9-9FAFD205E455"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << QStringLiteral("invalid service") << _gattCommunicationChannelServiceId.toString();
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &spirittreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void spirittreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("spirittreadmill::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void spirittreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("spirittreadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void spirittreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    {
        if (device.name().toUpper().startsWith(QStringLiteral("XT385"))) {
            XT385 = true;
            qDebug() << QStringLiteral("XT385 mod enabled");
        } else if (device.name().toUpper().startsWith(QStringLiteral("XT485"))) {
            XT485 = true;
            qDebug() << QStringLiteral("XT485 mod enabled");
        }
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &spirittreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &spirittreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &spirittreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &spirittreadmill::controllerStateChanged);

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

bool spirittreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void spirittreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
