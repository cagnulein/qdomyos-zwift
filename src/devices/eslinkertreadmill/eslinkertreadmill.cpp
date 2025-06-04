#include "eslinkertreadmill.h"
#include "homeform.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <QRandomGenerator>

using namespace std::chrono_literals;

class CRC8
{
  public:
    CRC8(quint8 polynomial = 0x07, quint8 init = 0x00, bool refIn = false, bool refOut = false, quint8 xorOut = 0x00)
        : m_polynomial(polynomial), m_init(init), m_refIn(refIn), m_refOut(refOut), m_xorOut(xorOut)
    {
        generateTable();
    }

    quint8 calculate(const QByteArray &data)
    {
        quint8 crc = m_init;
        for (char c : data) {
            if (m_refIn)
                c = reflect8(c);
            crc = m_table[crc ^ static_cast<quint8>(c)];
        }
        if (m_refOut)
            crc = reflect8(crc);
        return crc ^ m_xorOut;
    }

  private:
    quint8 m_polynomial;
    quint8 m_init;
    bool m_refIn;
    bool m_refOut;
    quint8 m_xorOut;
    quint8 m_table[256];

    void generateTable()
    {
        for (int i = 0; i < 256; ++i) {
            quint8 crc = static_cast<quint8>(i);
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x80)
                    crc = (crc << 1) ^ m_polynomial;
                else
                    crc <<= 1;
            }
            m_table[i] = crc;
        }
    }

    quint8 reflect8(quint8 value)
    {
        quint8 reflected = 0;
        for (int i = 0; i < 8; ++i) {
            if (value & 0x01)
                reflected |= (1 << (7 - i));
            value >>= 1;
        }
        return reflected;
    }
};


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

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                            QLowEnergyService::WriteWithoutResponse);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
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
    if (treadmill_type == RHYTHM_FUN || treadmill_type == YPOO_MINI_CHANGE) {
        // trying to force a fixed value to keep the connection on
        uint8_t display[] = {0xa9, 0xa0, 0x03, 0x02, 0x23, 0x00, 0x2b};

        writeCharacteristic(display, sizeof(display),
                            QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, false);
    } else if (treadmill_type == ESANGLINKER){

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
    } else if (treadmill_type == YPOO_MINI_CHANGE) {
        // 0x0d: a901010da4
        // 0x0e: a901010ea7
        // CheckSum 8 Xor
        uint8_t display[] = {0xa9, 0x01, 0x01, 0x0e, 0x00};
        display[3] = requestSpeed * 10;
        for (int i = 0; i < 4; i++) {
            display[4] = display[4] ^ display[i];
        }

        writeCharacteristic(display, sizeof(display),
                            QStringLiteral("forceSpeed speed=") + QString::number(requestSpeed), false, true);
    } else if (treadmill_type == COSTAWAY) {
        // CheckSum 8 Xor
        uint8_t display[] = {0xa9, 0xa0, 0x03, 0x02, 0x00, 0x00, 0x00};
        display[4] = requestSpeed * 10;
        for (int i = 0; i < 6; i++) {
            display[6] = display[6] ^ display[i];
        }

        writeCharacteristic(display, sizeof(display),
                            QStringLiteral("forceSpeed speed=") + QString::number(requestSpeed), false, true);
    } else if(treadmill_type == ESANGLINKER) {
        uint8_t display[] = {0xa9, 0x01, 0x01, 0x0b, 0x00};
        display[3] = (int)qRound(requestSpeed * 10 * 0.621371);
        for (int i = 0; i < 4; i++) {
            display[4] = display[4] ^ display[i];
        }

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
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &eslinkertreadmill::debug);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
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

        if (treadmill_type == TYPE::RHYTHM_FUN || treadmill_type == TYPE::YPOO_MINI_CHANGE ||
            treadmill_type == TYPE::COSTAWAY || treadmill_type == TYPE::ESANGLINKER) {

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
                if (requestInclination < 0)
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
        } else if (treadmill_type == COSTAWAY) {
            uint8_t initData11[] = {0xa9, 0xa0, 0x03, 0x02, 0x06, 0x00, 0x0e};
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("noop"), false, true);
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
            uint8_t startTape[] = {0xa9, 0xa3, 0x01, 0x01, 0x0a};
            writeCharacteristic(startTape, sizeof(startTape), QStringLiteral("startTape"), false, true);
            requestSpeed = 1.0;
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            requestSpeed = 0;
            emit debug(QStringLiteral("stopping..."));
            uint8_t startTape[] = {0xa9, 0xa3, 0x01, 0x00, 0x0b};
            writeCharacteristic(startTape, sizeof(startTape), QStringLiteral("stopTape"), false, true);
            requestStop = -1;
        }
    }
}

void eslinkertreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());

    if(gatt == QBluetoothUuid((quint16)0x1826)) {
        QSettings settings;
        settings.setValue(QZSettings::ftms_treadmill, bluetoothDevice.name());
        qDebug() << "forcing FTMS treadmill since it has FTMS";
        if(homeform::singleton())
            homeform::singleton()->setToastRequested("FTMS treadmill found, restart the app to apply the change");
    }
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

    if(treadmill_type == TYPE::ESANGLINKER) {
        if((uint8_t)newValue.at(0) == 0xa9 && (uint8_t)newValue.at(1) == 0x08 && (uint8_t)newValue.at(2) == 0x04) { // pair request
            lastPairFrame = newValue;
            qDebug() << "lastPairFrame" << lastPairFrame;

            uint8_t initData6[] = {0xa9, 0x08, 0x04, 0x0c, 0x06, 0x48, 0x12, 0xf5};

            if(lastPairFrame.length() < 3) {
                qDebug() << "ERROR! Pair code!";
                return;
            }
            QByteArray crypto = cryptographicArray(lastPairFrame.at(3));
            initData6[3] = crypto.at(0);
            initData6[4] = crypto.at(1);
            initData6[5] = crypto.at(2);
            initData6[6] = crypto.at(3);
            CRC8 crc8;
            initData6[7] = crc8.calculate(QByteArray((const char*)&initData6[3], 4));

            writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);

            emit pairPacketReceived();
        } else if((uint8_t)newValue.at(0) == 0xa9 && (uint8_t)newValue.at(1) == 0x08 &&
                   (uint8_t)newValue.at(2) == 0x01 && (uint8_t)newValue.at(3) == 0xff && (uint8_t)newValue.at(4) == 0x5f) { // handshake request
            qDebug() << "handshake";

            uint8_t initData5[] = {0xa9, 0x08, 0x01, 0xad, 0x0d};
            initData5[3] = QRandomGenerator::global()->bounded(256);
            CRC8 crc8;
            initData5[4] = crc8.calculate(QByteArray((const char*)&initData5[3], 1));

            writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);

            emit handshakePacketReceived();
        }
    }

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

    if ((newValue.length() != 17 && (treadmill_type == RHYTHM_FUN || treadmill_type == YPOO_MINI_CHANGE))) {

    } else if (newValue.length() != 5 && (treadmill_type == COSTAWAY || treadmill_type == TYPE::ESANGLINKER)) {

    } else if (treadmill_type == RHYTHM_FUN || treadmill_type == YPOO_MINI_CHANGE) {
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
    } else if (treadmill_type == COSTAWAY || (treadmill_type == TYPE::ESANGLINKER && (uint8_t)newValue.at(1) == 0xe0)) {
        const double miles = 1.60934;
        if(((uint8_t)newValue.at(3)) == 0xFF && treadmill_type == COSTAWAY)
            Speed = 0;
        else
            Speed = (double)((uint8_t)newValue.at(3)) / 10.0 * miles;
        Inclination = 0; // this treadmill doesn't have inclination
        emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
    }

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        Distance += ((Speed.value() / 3600.0) /
                     (1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }

    cadenceFromAppleWatch();

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
    if (treadmill_type == YPOO_MINI_CHANGE && data < 1.0) {
        data = 0.0;
    }
    return data;
}

double eslinkertreadmill::GetKcalFromPacket(const QByteArray &packet) {
    double data;
    if (treadmill_type == YPOO_MINI_CHANGE) {
        uint16_t convertedData = (((uint8_t)packet.at(5)) << 8) | (uint8_t)packet.at(6);
        ;
        data = (double)convertedData / 100.0f;
    } else {
        uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
        data = (double)convertedData;
    }
    return data;
}

double eslinkertreadmill::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

double eslinkertreadmill::GetInclinationFromPacket(const QByteArray &packet) {
    double data = 0.0;
    if (treadmill_type != YPOO_MINI_CHANGE) {
        uint16_t convertedData = packet.at(11);
        data = convertedData;
    }
    return data;
}

void eslinkertreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)

    if (treadmill_type == ESANGLINKER) {
        uint8_t initData1[] = {0xa9, 0xf2, 0x01, 0x2f, 0x75};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);

        uint8_t initData2[] = {0xa9, 0x0a, 0x01, 0xc6, 0x64};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);

        uint8_t initData3[] = {0xa9, 0xae, 0x01, 0xfe, 0xf8};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);

        uint8_t initData4[] = {0xa9, 0xa0, 0x03, 0x00, 0x00, 0x00, 0x0a};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);

        waitForHandshakePacket();

        QThread::sleep(2);

        waitForPairPacket();

        QThread::sleep(1);

        uint8_t initData7[] = {0xa9, 0x1e, 0x01, 0xfe, 0x48};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);

        uint8_t initData8[] = {0xa9, 0xae, 0x01, 0xfe, 0xf8};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, true);

        QThread::sleep(2);

        uint8_t initData9[] = {0xa9, 0xa3, 0x01, 0x01, 0x0a};
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, true);

        uint8_t initData10[] = {0xa9, 0x8e, 0x01, 0x09, 0x2f};
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);

        uint8_t initData11[] = {0xa9, 0xb2, 0x01, 0xfe, 0xe4};
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, true);

        QThread::sleep(3);

        uint8_t initData12[] = {0xa9, 0x8e, 0x01, 0x09, 0x2f};
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);

        uint8_t initData13[] = {0xa9, 0xae, 0x01, 0xfe, 0xf8};
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, true);

        if(homeform::singleton())
            homeform::singleton()->setToastRequested("Init completed, you can use the treadmill now!");

    } else if (treadmill_type == COSTAWAY) {
        uint8_t initData1[] = {0xa9, 0xf2, 0x01, 0x2f, 0x75};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

        uint8_t initData2[] = {0xa9, 0x08, 0x01, 0x79, 0xd9};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);

        uint8_t initData3[] = {0xa9, 0x08, 0x04, 0x05, 0x04, 0x04, 0x01, 0xa1};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);

        uint8_t initData4[] = {0xa9, 0x1e, 0x01, 0xfe, 0x48};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);

        uint8_t initData5[] = {0xa9, 0xa0, 0x03, 0x00, 0x00, 0x00, 0x0a};
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);

        uint8_t initData6[] = {0xa9, 0x0a, 0x01, 0x48, 0xea};
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);

        uint8_t initData7[] = {0xa9, 0xae, 0x01, 0xfe, 0xf8};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);

        uint8_t initData8[] = {0xa9, 0xa0, 0x03, 0x00, 0x00, 0x00, 0x0a};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, true);
    } else if (treadmill_type == RHYTHM_FUN || treadmill_type == YPOO_MINI_CHANGE) {
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
        uint8_t initData2_CADENZA[] = {0x08, 0x01, 0x01};
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
        bool eslinker_cadenza =
            settings.value(QZSettings::eslinker_cadenza, QZSettings::default_eslinker_cadenza).toBool();
        bool eslinker_ypoo = settings.value(QZSettings::eslinker_ypoo, QZSettings::default_eslinker_ypoo).toBool();
        bool eslinker_costaway =
            settings.value(QZSettings::eslinker_costaway, QZSettings::default_eslinker_costaway).toBool();
        if(device.name().toUpper().startsWith(QStringLiteral("ESANGLINKER"))) {
            treadmill_type = ESANGLINKER;
            qDebug() << "ESANGLINKER workaround ENABLED!";
        } else if (eslinker_cadenza) {
            treadmill_type = CADENZA_FITNESS_T45;
        } else if (eslinker_ypoo) {
            treadmill_type = YPOO_MINI_CHANGE;
        } else if (eslinker_costaway) {
            treadmill_type = COSTAWAY;
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

QByteArray eslinkertreadmill::cryptographicArray(quint8 b2) {
    int i = b2 % 6;
    QRandomGenerator *random = QRandomGenerator::global();
    int nextInt = random->bounded(1, 16);  // 1 to 15
    int nextInt2 = random->bounded(1, 16); // 1 to 15

    QByteArray bArr(4, 0);

    switch (i) {
    case 0:
        bArr[0] = static_cast<char>(nextInt);
        bArr[1] = static_cast<char>(nextInt2);
        bArr[2] = static_cast<char>(nextInt + nextInt2);
        bArr[3] = static_cast<char>(nextInt * nextInt2);
        break;
    case 1:
        bArr[0] = static_cast<char>(nextInt);
        bArr[1] = static_cast<char>(nextInt2);
        bArr[2] = static_cast<char>(nextInt * nextInt2);
        bArr[3] = static_cast<char>(nextInt + nextInt2);
        break;
    case 2:
        bArr[0] = static_cast<char>(nextInt + nextInt2);
        bArr[1] = static_cast<char>(nextInt);
        bArr[2] = static_cast<char>(nextInt2);
        bArr[3] = static_cast<char>(nextInt * nextInt2);
        break;
    case 3:
        bArr[0] = static_cast<char>(nextInt * nextInt2);
        bArr[1] = static_cast<char>(nextInt);
        bArr[2] = static_cast<char>(nextInt2);
        bArr[3] = static_cast<char>(nextInt + nextInt2);
        break;
    case 4:
        bArr[0] = static_cast<char>(nextInt + nextInt2);
        bArr[1] = static_cast<char>(nextInt * nextInt2);
        bArr[2] = static_cast<char>(nextInt);
        bArr[3] = static_cast<char>(nextInt2);
        break;
    case 5:
        bArr[0] = static_cast<char>(nextInt * nextInt2);
        bArr[1] = static_cast<char>(nextInt + nextInt2);
        bArr[2] = static_cast<char>(nextInt);
        bArr[3] = static_cast<char>(nextInt2);
        break;
    }

    return bArr;
}

void eslinkertreadmill::waitForPairPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &eslinkertreadmill::pairPacketReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

void eslinkertreadmill::waitForHandshakePacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &eslinkertreadmill::handshakePacketReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

double eslinkertreadmill::minStepSpeed() {
    if(treadmill_type == ESANGLINKER)
        return 0.160934;  // 0.1 mi
    else
        return 0.5;
}
