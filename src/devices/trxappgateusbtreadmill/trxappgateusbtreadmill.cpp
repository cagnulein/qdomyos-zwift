#include "trxappgateusbtreadmill.h"
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

trxappgateusbtreadmill::trxappgateusbtreadmill() {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &trxappgateusbtreadmill::update);
    refresh->start(200ms);
}

void trxappgateusbtreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                                 bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &trxappgateusbtreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse && (treadmill_type != TYPE::ADIDAS && treadmill_type != TYPE::DKN && treadmill_type != TYPE::DKN_2  && treadmill_type != TYPE::DKN_3)) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void trxappgateusbtreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void trxappgateusbtreadmill::forceIncline(double requestIncline) {
    QSettings settings;
    bool reebok_fr30_treadmill =
        settings.value(QZSettings::reebok_fr30_treadmill, QZSettings::default_reebok_fr30_treadmill).toBool();

    if (requestIncline < 0)
        requestIncline = 0;
    if (treadmill_type == TYPE::ADIDAS) {
        uint8_t write[] = {0xf0, 0xac, 0x5b, 0xd3, 0x08, 0x64, 0x64, 0x9a};
        write[4] = (requestIncline + 1);
        write[7] = write[4] + 0x92;

        writeCharacteristic(write, sizeof(write), QStringLiteral("forceIncline"), false, true);
    } else if (!reebok_fr30_treadmill) {
        uint8_t write[] = {0xf0, 0xac, 0x01, 0xd3, 0x03, 0x64, 0x64, 0x3b};
        write[4] = (requestIncline + 1);
        write[7] = write[4] + 0x38;

        writeCharacteristic(write, sizeof(write), QStringLiteral("forceIncline"), false, true);
    } else {
        uint8_t write[] = {0xf0, 0xac, 0x32, 0xd3, 0x01, 0x64, 0x64, 0x6a};
        write[4] = (requestIncline + 1);
        write[7] = write[4] + 0x69;

        writeCharacteristic(write, sizeof(write), QStringLiteral("forceIncline"), false, true);
    }
}

void trxappgateusbtreadmill::forceSpeed(double requestSpeed) {}

void trxappgateusbtreadmill::update() {
    QSettings settings;
    bool jtx_fitness_sprint_treadmill =
        settings.value(QZSettings::jtx_fitness_sprint_treadmill, QZSettings::default_jtx_fitness_sprint_treadmill)
            .toBool();
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
        }

        bool toorx30 = settings.value(QZSettings::toorx_3_0, QZSettings::default_toorx_3_0).toBool();
        if (treadmill_type == TYPE::REEBOK || treadmill_type == TYPE::REEBOK_2) {
            const uint8_t noOpData[] = {0xf0, 0xa2, 0x32, 0xd3, 0x97};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (treadmill_type == TYPE::DKN_2) {
            const uint8_t noOpData[] = {0xf0, 0xa2, 0x04, 0x01, 0x97};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (treadmill_type == TYPE::ADIDAS) {
            const uint8_t noOpData[] = {0xf0, 0xa2, 0x5b, 0xd3, 0xc0};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (treadmill_type == TYPE::DKN || treadmill_type == TYPE::DKN_2 || treadmill_type == TYPE::DKN_3 || toorx30 == false ||
                   jtx_fitness_sprint_treadmill) {
            const uint8_t noOpData[] = {0xf0, 0xa2, 0x01, 0xd3, 0x66};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else {
            const uint8_t noOpData[] = {0xf0, 0xa2, 0x23, 0xd3, 0x88};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        }

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value()) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            // only 0.5 changes otherwise it beeps forever
            if (jtx_fitness_sprint_treadmill)
                requestInclination = qRound(requestInclination * 2.0) / 2.0;

            if (requestInclination != currentInclination().value()) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                forceIncline(requestInclination);
            }
            requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            // btinit(true);
            if (treadmill_type == TYPE::REEBOK || treadmill_type == TYPE::REEBOK_2) {
                const uint8_t startTape[] = {0xf0, 0xa5, 0x32, 0xd3, 0x02, 0x9c};
                writeCharacteristic((uint8_t *)startTape, sizeof(startTape), QStringLiteral("startTape"), false, true);
            } else if (treadmill_type == TYPE::DKN || treadmill_type == TYPE::DKN_2 || toorx30 == false) {
                const uint8_t startTape[] = {0xf0, 0xa5, 0x01, 0xd3, 0x02, 0x6b};
                writeCharacteristic((uint8_t *)startTape, sizeof(startTape), QStringLiteral("startTape"), false, true);
            } else if (treadmill_type == TYPE::ADIDAS) {
                const uint8_t startTape[] = {0xf0, 0xa5, 0x5b, 0xd3, 0x02, 0xc5};
                writeCharacteristic((uint8_t *)startTape, sizeof(startTape), QStringLiteral("startTape"), false, true);
                writeCharacteristic((uint8_t *)startTape, sizeof(startTape), QStringLiteral("startTape"), false, true);
            } else {
                const uint8_t startTape[] = {0xf0, 0xa5, 0x23, 0xd3, 0x02, 0x8d};
                writeCharacteristic((uint8_t *)startTape, sizeof(startTape), QStringLiteral("startTape"), false, true);
            }

            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");

            if (treadmill_type == TYPE::ADIDAS) {
                const uint8_t stopTape[] = {0xf0, 0xa5, 0x5b, 0xd3, 0x04, 0xc7};
                writeCharacteristic((uint8_t *)stopTape, sizeof(stopTape), QStringLiteral("stopTape"), false, true);
            }

            requestStop = -1;
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));
            // changeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));
            // changeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

void trxappgateusbtreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void trxappgateusbtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                   const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket.append(newValue);

    qDebug() << "actual lastPacket" << lastPacket.toHex(' ');

    // Focus Fitness Senator 54 iplus #1790
    if((newValue.length() < 18 && lastPacket.length() > 2 && (((uint8_t)lastPacket.at(0)) != 0xf0 || ((uint8_t)lastPacket.at(1)) != 0xb2)) || lastPacket.length() > 19) {
        if(lastPacket.length() == 3 && ((uint8_t)lastPacket.at(1)) == 0xf0 && ((uint8_t)lastPacket.at(2)) == 0xb2) {
            lastPacket.clear();
            lastPacket.append(0xf0);
            lastPacket.append(0xb2);
            return;
        }
        lastPacket.clear();
        return;
    }

    if ((lastPacket.length() != 19 && treadmill_type != TYPE::DKN_2) ||
        (lastPacket.length() != 18 && treadmill_type == TYPE::DKN_2)) {
        return;
    }

    if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
        if (lastPacket.at(15) == 0x03 && lastPacket.at(16) == 0x02 && readyToStart == false) {
            readyToStart = true;
            requestStart = 1;
        }
    } else if (treadmill_type != TYPE::REEBOK && treadmill_type != TYPE::REEBOK_2 && treadmill_type != TYPE::DKN && treadmill_type != TYPE::DKN_2) {
        if (lastPacket.at(16) == 0x04 && lastPacket.at(17) == 0x03 && readyToStart == false) {
            readyToStart = true;
            requestStart = 1;
        }
    }

    double speed = GetSpeedFromPacket(lastPacket);
    double incline = GetInclinationFromPacket(lastPacket);
    double kcal = GetKcalFromPacket(lastPacket);
    double distance = GetDistanceFromPacket(lastPacket);

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            update_hr_from_external();
        }
    }
    FanSpeed = 0;

    QTime now = QTime::currentTime();
    if (!firstCharChanged) {
        DistanceCalculated += ((speed / 3600.0) / (1000.0 / (lastTimeCharChanged.msecsTo(now))));
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            kcal =
                KCal.value() + ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) + 1.19) *
                                    settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                                    200.0) /
                                (60000.0 / ((double)lastTimeCharChanged.msecsTo(
                                                now)))); //(( (0.048* Output in watts +1.19) * body
                                                                            // weight in kg * 3.5) / 200 ) / 60
        else
            kcal = KCal.value();
    }
    lastTimeCharChanged = now;

    cadenceFromAppleWatch();

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
    emit debug(QStringLiteral("Current Elapsed from the treadmill (not used): ") +
               QString::number(GetElapsedFromPacket(lastPacket)));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(DistanceCalculated));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    Speed = speed;
    Inclination = incline;
    KCal = kcal;
    Distance = distance;

    firstCharChanged = false;

    lastPacket.clear();
}

uint16_t trxappgateusbtreadmill::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(4) - 1);
    convertedData += ((packet.at(5) - 1) * 60);
    return convertedData;
}

double trxappgateusbtreadmill::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (uint16_t)((packet.at(13) - 1)) + (((uint16_t)((uint8_t)packet.at(12) - 1)) * 100);
    double data = (double)(convertedData) / 10.0f;
    return data;
}

double trxappgateusbtreadmill::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = ((packet.at(8) - 1) << 8) | (packet.at(9) - 1);
    return (double)(convertedData);
}

double trxappgateusbtreadmill::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = ((packet.at(6) - 1) * 100) + (packet.at(7) - 1);
    double data = ((double)(convertedData)) / 100.0f;
    return data;
}

double trxappgateusbtreadmill::GetInclinationFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(14);
    double data = (convertedData - 1);
    if (data < 0) {
        return 0;
    }
    return data;
}

void trxappgateusbtreadmill::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &trxappgateusbtreadmill::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

void trxappgateusbtreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape);
    QSettings settings;
    bool toorx30 = settings.value(QZSettings::toorx_3_0, QZSettings::default_toorx_3_0).toBool();
    bool jtx_fitness_sprint_treadmill =
        settings.value(QZSettings::jtx_fitness_sprint_treadmill, QZSettings::default_jtx_fitness_sprint_treadmill)
            .toBool();

    if (treadmill_type == TYPE::DKN || treadmill_type == TYPE::DKN_3) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x02, 0x02, 0x94};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x01, 0xd3, 0x64};
        const uint8_t initData3[] = {0xf0, 0xa5, 0x01, 0xd3, 0x04, 0x6d};
        const uint8_t initData4[] = {0xf0, 0xa1, 0x01, 0xd3, 0x65};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x01, 0xd3, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x47, 0xb7};
        const uint8_t initData6[] = {0xf0, 0xac, 0x01, 0xd3, 0x01, 0x64, 0x64, 0x39};
        const uint8_t initData7[] = {0xf0, 0xa4, 0x01, 0xd3, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x0b, 0x01, 0x01, 0x7d};
        const uint8_t initData8[] = {0xf0, 0xaf, 0x01, 0xd3, 0x02, 0x75};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(400);

    } else if (treadmill_type == TYPE::ADIDAS) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x02, 0x02, 0x94};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x5b, 0xd3, 0xbe};
        const uint8_t initData3[] = {0xf0, 0xa5, 0x5b, 0xd3, 0x04, 0xc7};
        const uint8_t initData4[] = {0xf0, 0xa1, 0x5b, 0xd3, 0xbf};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x5b, 0xd3, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x47, 0x11};
        const uint8_t initData6[] = {0xf0, 0xac, 0x5b, 0xd3, 0x01, 0x64, 0x64, 0x93};
        const uint8_t initData7[] = {0xf0, 0xa4, 0x5b, 0xd3, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x0b, 0x01, 0x01, 0xd7};
        const uint8_t initData8[] = {0xf0, 0xaf, 0x5b, 0xd3, 0x02, 0xcf};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(400);

    } else if (treadmill_type == TYPE::REEBOK || treadmill_type == TYPE::REEBOK_2) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x32, 0xd3, 0x95};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x32, 0xd3, 0x96};
        const uint8_t initData4[] = {0xf0, 0xa5, 0x32, 0xd3, 0x04, 0x9e};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x32, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x54};
        const uint8_t initData6[] = {0xf0, 0xa4, 0x32, 0xd3, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xa4};
        const uint8_t initData7[] = {0xf0, 0xaf, 0x32, 0xd3, 0x02, 0xa6};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (treadmill_type == TYPE::DKN_2) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x04, 0x01, 0x95};
        const uint8_t initData2[] = {0xf0, 0xa5, 0x04, 0x01, 0x04, 0x9e};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x04, 0x01, 0x96};
        const uint8_t initData4[] = {0xf0, 0xa3, 0x04, 0x01, 0x04, 0x9c};
        const uint8_t initData5[] = {0xf0, 0xac, 0x04, 0x01, 0x01, 0x64, 0x64, 0x6a};
        const uint8_t initData6[] = {0xf0, 0xa4, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0xa6};
        const uint8_t initData7[] = {0xf0, 0xa4, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xa4};
        const uint8_t initData8[] = {0xf0, 0xa4, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x06, 0x01, 0x01, 0xa9};
        const uint8_t initData9[] = {0xf0, 0xa2, 0x04, 0x01, 0x97};
        const uint8_t initData10[] = {0xf0, 0xa5, 0x04, 0x01, 0x02, 0x9c};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData9, sizeof(initData9), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData10, sizeof(initData10), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (toorx30 == false || jtx_fitness_sprint_treadmill) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa5, 0x01, 0xd3, 0x04, 0x6d};
        const uint8_t initData3[] = {0xf0, 0xa0, 0x01, 0xd3, 0x64};
        const uint8_t initData4[] = {0xf0, 0xa1, 0x01, 0xd3, 0x65};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x01, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x23};
        const uint8_t initData6[] = {0xf0, 0xa4, 0x01, 0xd3, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x73};
        const uint8_t initData7[] = {0xf0, 0xaf, 0x01, 0xd3, 0x02, 0x75};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
            QThread::msleep(400);
        }
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2 || jtx_fitness_sprint_treadmill) {
            writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
            QThread::msleep(400);
            writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
            QThread::msleep(400);
            writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::IRUNNING_2) {
            QThread::msleep(400);
        }
    } else {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa5, 0x23, 0xd3, 0x04, 0x8f};
        const uint8_t initData3[] = {0xf0, 0xa0, 0x23, 0xd3, 0x86};
        const uint8_t initData4[] = {0xf0, 0xa1, 0x23, 0xd3, 0x87};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x23, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x45};
        const uint8_t initData6[] = {0xf0, 0xa4, 0x23, 0xd3, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x95};
        const uint8_t initData7[] = {0xf0, 0xaf, 0x23, 0xd3, 0x02, 0x97};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    }
    initDone = true;
}

void trxappgateusbtreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            emit debug("characteristic " + c.uuid().toString());
        }

        QString uuidWrite = QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb");
        QString uuidNotify = QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb");
        QString uuidNotify2 = QStringLiteral("49535343-ACA3-481C-91EC-D85E28A60318");

        if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::REEBOK || treadmill_type == TYPE::DKN_2 || treadmill_type == TYPE::DKN_3) {
            uuidWrite = QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3");
            uuidNotify = QStringLiteral("49535343-1E4D-4BD9-BA61-23C647249616");
        }

        QBluetoothUuid _gattWriteCharacteristicId((QString)uuidWrite);
        QBluetoothUuid _gattNotifyCharacteristicId((QString)uuidNotify);
        QBluetoothUuid _gattNotify2CharacteristicId((QString)uuidNotify2);

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        if (treadmill_type == TYPE::DKN_2)
            gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &trxappgateusbtreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &trxappgateusbtreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &trxappgateusbtreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &trxappgateusbtreadmill::descriptorWritten);

        // ******************************************* virtual treadmill init *************************************
        if (!firstVirtualTreadmill && !this->hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadMill = new virtualtreadmill(this, false);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &trxappgateusbtreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &trxappgateusbtreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstVirtualTreadmill = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

        if (treadmill_type == TYPE::DKN_2 && gattNotify2Characteristic.isValid())
            gattCommunicationChannelService->writeDescriptor(
                gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void trxappgateusbtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void trxappgateusbtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                   const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void trxappgateusbtreadmill::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone") << treadmill_type;

    QString uuid = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");
    if (treadmill_type == TYPE::IRUNNING || treadmill_type == TYPE::REEBOK || treadmill_type == TYPE::DKN_2) {
        uuid = QStringLiteral("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
    }

    QBluetoothUuid _gattCommunicationChannelServiceId((QString)uuid);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        if (treadmill_type == TYPE::IRUNNING) {
            uuid = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");
            QBluetoothUuid _gattCommunicationChannelServiceId2((QString)uuid);
            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId2);
            if (gattCommunicationChannelService == nullptr) {
                qDebug() << QStringLiteral("invalid service") << uuid;
                return;
            } else {
                treadmill_type = TYPE::IRUNNING_2;
                qDebug() << QStringLiteral("treadmill_type IRUNNING_2");
            }
        } else if (treadmill_type == TYPE::REEBOK) {
            uuid = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");
            QBluetoothUuid _gattCommunicationChannelServiceId2((QString)uuid);
            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId2);
            if (gattCommunicationChannelService == nullptr) {
                qDebug() << QStringLiteral("invalid service") << uuid;
                return;
            } else {
                treadmill_type = TYPE::REEBOK_2;
                qDebug() << QStringLiteral("treadmill_type REEBOK_2");
            }
        } else if (treadmill_type == TYPE::DKN) {
            uuid = QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455");
            QBluetoothUuid _gattCommunicationChannelServiceId2((QString)uuid);
            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId2);
            if (gattCommunicationChannelService == nullptr) {
                qDebug() << QStringLiteral("invalid service") << uuid;
                return;
            } else {
                treadmill_type = TYPE::DKN_3;
                qDebug() << QStringLiteral("treadmill_type DKN_3");
            }            
        } else {
            qDebug() << QStringLiteral("invalid service") << uuid;
            return;
        }
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &trxappgateusbtreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void trxappgateusbtreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("trxappgateusbtreadmill::errorService") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void trxappgateusbtreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("trxappgateusbtreadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void trxappgateusbtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    bool dkn_endurun_treadmill =
        settings.value(QZSettings::dkn_endurun_treadmill, QZSettings::default_dkn_endurun_treadmill).toBool();

    if (device.name().startsWith(QStringLiteral("TOORX")) || device.name().startsWith(QStringLiteral("V-RUN")) ||
        device.name().startsWith(QStringLiteral("FS-")) ||
        device.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+")) ||
        device.name().toUpper().startsWith(QStringLiteral("I-RUNNING")) ||
        device.name().startsWith(QStringLiteral("F63")) ||
        device.name().toUpper().startsWith(QStringLiteral("REEBOK")) ||
        device.name().toUpper().startsWith(QStringLiteral("ICONSOLE+")) ||
        device.name().toUpper().startsWith(QStringLiteral("DKN RUN")) ||
        device.name().toUpper().startsWith(QStringLiteral("K80_")) ||
        device.name().toUpper().startsWith(QStringLiteral("XT900")) ||
        device.name().toUpper().startsWith(QStringLiteral("ADIDAS ")) ||
        device.name().toUpper().startsWith(QStringLiteral("XT485"))) {
        if (dkn_endurun_treadmill) {
            treadmill_type = TYPE::DKN;
        } else if (device.name().toUpper().startsWith(QStringLiteral("DKN RUN"))) {
            treadmill_type = TYPE::DKN_2;
        } else if (device.name().toUpper().startsWith(QStringLiteral("I-RUNNING")) ||
                   device.name().toUpper().startsWith(QStringLiteral("ICONSOLE+")) ||
                   device.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+")) ||
                   device.name().startsWith(QStringLiteral("F63")) ||
                   device.name().toUpper().startsWith(QStringLiteral("DKN RUN")) ||
                   device.name().toUpper().startsWith(QStringLiteral("XT900")) ||
                   device.name().toUpper().startsWith(QStringLiteral("XT485"))) {
            treadmill_type = TYPE::IRUNNING;
        } else if (device.name().toUpper().startsWith(QStringLiteral("REEBOK"))) {
            treadmill_type = TYPE::REEBOK;
        } else if (device.name().toUpper().startsWith(QStringLiteral("ADIDAS "))) {
            treadmill_type = TYPE::ADIDAS;
        } else {
            treadmill_type = TYPE::TRXAPPGATE;
        }

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &trxappgateusbtreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &trxappgateusbtreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &trxappgateusbtreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &trxappgateusbtreadmill::controllerStateChanged);

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

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool trxappgateusbtreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void trxappgateusbtreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

double trxappgateusbtreadmill::minStepInclination() { return 1.0; }
