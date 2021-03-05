#include "fitshowtreadmill.h"
#include "virtualtreadmill.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>
#include "ios/lockscreen.h"
#include "keepawakehelper.h"

#define BLE_SERIALOUTPUT_MAXSIZE 25

fitshowtreadmill::fitshowtreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed, double forceInitInclination) {
    Q_UNUSED(noConsole)
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0)
        lastSpeed = forceInitSpeed;

    if (forceInitInclination > 0)
        lastInclination = forceInitInclination;
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
    h = new lockscreen();
#endif
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(pollDeviceTime);
}

fitshowtreadmill::~fitshowtreadmill() {
    if (refresh) {
        refresh->stop();
        delete refresh;
    }
    if (virtualTreadMill)
        delete virtualTreadMill;
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
    if (h)
        delete h;
#endif
}

void fitshowtreadmill::scheduleWrite(const uint8_t* data, uint8_t data_len, const QString& info) {
    bufferWrite.append((char)data_len);
    bufferWrite.append(QByteArray((const char*)data, data_len));
    debugMsgs.append(info);
}

void fitshowtreadmill::writeCharacteristic(const uint8_t* data, uint8_t data_len, const QString& info) {
    QEventLoop loop;
    QTimer timeout;
    QByteArray qba((const char*)data, data_len);
    if (!info.isEmpty())
        debug(" >>" + qba.toHex(' ') + " // " + info);

    connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
            &loop, SLOT(quit()));
    timeout.singleShot(300, &loop, SLOT(quit()));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, qba);

    loop.exec();

    if (timeout.isActive() == false)
        debug(" exit for timeout");
}

bool fitshowtreadmill::checkIncomingPacket(const uint8_t* data, uint8_t data_len) const {
    if (data_len >= 4 && data[0] == FITSHOW_PKT_HEADER && data[data_len - 1] == FITSHOW_PKT_FOOTER) {
        int n4 = 0;
        int n5 = 1;
        int n6 = data_len - 2;
        while (true) {
            if (n5 >= n6) {
                break;
            }
            n4 ^= data[n5];
            ++n5;
        }
        return n4 == data[n6];
    }
    else
        return false;
}

bool fitshowtreadmill::writePayload(const uint8_t * array, uint8_t size, const QString& info) {
    if (size + 3 > BLE_SERIALOUTPUT_MAXSIZE)
        return false;
    uint8_t array2[BLE_SERIALOUTPUT_MAXSIZE];
    array2[0] = FITSHOW_PKT_HEADER;
    uint8_t n = 0, i = 0;
    while (i < size) {
        array2[i + 1] = array[i];
        n ^= array[i++];
    }
    array2[size + 1] = n;
    array2[size + 2] = FITSHOW_PKT_FOOTER;
    writeCharacteristic(array2, size + 3, info);
    return true;
}


void fitshowtreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline) {
    if (MAX_SPEED > 0) {
        requestSpeed *= 10.0;
        if (requestSpeed >= MAX_SPEED)
            requestSpeed = MAX_SPEED;
        else if (requestSpeed <= MIN_SPEED)
            requestSpeed = MIN_SPEED;
        if (requestIncline >= MAX_INCLINE)
            requestIncline = MAX_INCLINE;
        else if (requestIncline <= MIN_INCLINE)
            requestIncline = MIN_INCLINE;
        uint8_t writeIncline[] = { FITSHOW_SYS_CONTROL, FITSHOW_CONTROL_TARGET_OR_RUN, (uint8_t)(requestSpeed + 0.5), (uint8_t)requestIncline };
        scheduleWrite(writeIncline, sizeof(writeIncline), "forceSpeedOrIncline speed=" + QString::number(requestSpeed) + " incline=" + QString::number(requestIncline));
    }
}

void fitshowtreadmill::update() {
    if (!m_control || m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    }
    else if (bluetoothDevice.isValid() &&
             m_control->state() == QLowEnergyController::DiscoveredState &&
             gattCommunicationChannelService &&
             gattWriteCharacteristic.isValid() &&
             gattNotifyCharacteristic.isValid() &&
             initDone) {
        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && searchStopped && !virtualTreadMill) {
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            if (virtual_device_enabled) {
                debug("creating virtual treadmill interface...");
                virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &fitshowtreadmill::debug);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        debug("fitshow Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        QDateTime current = QDateTime::currentDateTime();
        double deltaTime = (((double)lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
        if (currentSpeed().value() > 0.0 && !firstUpdate && !paused) {
            m_watt = (double)watts(settings.value("weight", 75.0).toFloat());
            m_jouls += (m_watt.value() * deltaTime);
        }
        lastTimeUpdate = current;

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value()) {
                debug("writing speed " + QString::number(requestSpeed));
                double inc = currentInclination().value();
                if (requestInclination != -1) {
                    int diffInc = (int)(requestInclination - inc);
                    if (!diffInc) {
                        if (requestInclination > inc)
                            inc += 1.0;
                        else if (requestInclination < inc)
                            inc -= 1.0;
                    }
                    else
                        inc = (int)requestInclination;
                    requestInclination = -1;
                }
                forceSpeedOrIncline(requestSpeed, inc);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -1) {
            double inc = currentInclination().value();
            if (requestInclination != inc) {
                debug("writing incline " + QString::number(requestInclination));
                int diffInc = (int)(requestInclination - inc);
                if (!diffInc) {
                    if (requestInclination > inc)
                        inc += 1.0;
                    else if (requestInclination < inc)
                        inc -= 1.0;
                }
                else
                    inc = (int)requestInclination;
                double speed = currentSpeed().value();
                if (requestSpeed != -1) {
                    speed = requestSpeed;
                    requestSpeed = -1;
                }
                forceSpeedOrIncline(speed, inc);
            }
            requestInclination = -1;
        }
        if (requestStart != -1) {
            debug("starting...");
            if (lastSpeed == 0.0)
                lastSpeed = 0.5;
            btinit(true);
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            uint8_t stopTape[] = { FITSHOW_SYS_CONTROL, FITSHOW_CONTROL_STOP }; // to verify
            debug("stopping...");
            scheduleWrite(stopTape, sizeof(stopTape), "stop tape");
            requestStop = -1;
        }

        elevationAcc += (currentSpeed().value() / 3600.0) * 1000.0 * (currentInclination().value() / 100.0) * deltaTime;
        if (retrySend >= 6) {//3 retries
            debug("WARNING: answer not received for command " + QString("%1 / %2 (%3)").arg(((uint8_t)bufferWrite.at(1)), 2, 16, QChar('0')).arg(((uint8_t)bufferWrite.at(2)), 2, 16, QChar('0')).arg(debugMsgs.at(0)));
            removeFromBuffer();
        }
        if (!bufferWrite.isEmpty()) {
            retrySend++;
            if (retrySend % 2) {//retry only on odd values: on even values wait some more time for response
                const uint8_t* write_pld = (const uint8_t*)bufferWrite.constData();
                writePayload(write_pld + 1, write_pld[0], debugMsgs.at(0));
            }
        }
        else {
            uint8_t status = FITSHOW_SYS_STATUS;
            writePayload(&status, 1);
        }

        firstUpdate = false;
    }
}

void fitshowtreadmill::removeFromBuffer() {
    if (!bufferWrite.isEmpty())
        bufferWrite.remove(0, ((uint8_t)bufferWrite.at(0)) + 1);
    if (!debugMsgs.isEmpty())
        debugMsgs.removeFirst();
    retrySend = 0;
}

void fitshowtreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    uint32_t servRepr = gatt.toUInt32();
    debug("serviceDiscovered " + gatt.toString() + " " + QString::number(servRepr));
    if (servRepr == 0xfff0 || (servRepr == 0xffe0 && serviceId.isNull())) {
        serviceId = gatt;
    }
}

void fitshowtreadmill::sendSportData() {
    uint8_t writeSport[] = { FITSHOW_SYS_DATA, FITSHOW_DATA_SPORT };
    scheduleWrite(writeSport, sizeof(writeSport), "SendSportsData");
}

void fitshowtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    //qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    debug(" << " + QString::number(value.length()) + " " + value.toHex(' '));

    debug("packetReceived!");
    emit packetReceived();

    lastPacket = value;
    const uint8_t * full_array = (uint8_t*)value.constData();
    uint8_t full_len = value.length();
    if (!checkIncomingPacket(full_array, full_len)) {
        debug("Invalid packet");
        return;
    }
    const uint8_t * array = full_array + 1;
    const uint8_t cmd = array[0];
    const uint8_t par = array[1];
    const uint8_t * array_expected = (const uint8_t*)bufferWrite.constData() + 1;
    uint8_t len = full_len - 3;
    if (cmd != FITSHOW_SYS_STATUS && bufferWrite.length() && *array_expected == cmd && *(array_expected + 1) == par) {
        removeFromBuffer();
    }
    if (cmd == FITSHOW_SYS_INFO) {
        if (par == FITSHOW_INFO_SPEED) {
            if (full_len > 6) {
                MAX_SPEED = full_array[3];
                MIN_SPEED = full_array[4];
                debug("Speed between " + QString::number(MIN_SPEED) + " and " + QString::number(MAX_SPEED));
                if (full_len > 7) {
                    UNIT = full_array[5];
                }
            }
        }
        else if (par == FITSHOW_INFO_INCLINE) {
            if (full_len < 7) {
                MAX_INCLINE = 0;
                debug("Incline not supported");
            }
            else {
                MAX_INCLINE = full_array[3];
                MIN_INCLINE = full_array[4];
                if (full_len > 7 && (full_array[5] & 0x2) != 0x0) {
                    IS_PAUSE = true;
                }
                debug("Incline between " + QString::number(MIN_INCLINE) + " and " + QString::number(MAX_INCLINE));
            }
        }
        else if (par == FITSHOW_INFO_MODEL) {
            if (full_len > 7) {
                uint16_t second = (full_array[5] << 8) | full_array[4];
                DEVICE_ID_NAME = QString("%1-%2")
                                 .arg(full_array[3], 2, 16, QLatin1Char('0'))
                                 .arg(second, 4, 16, QLatin1Char('0'));
                debug("DEVICE " + DEVICE_ID_NAME);
            }
        }
        else if (par == FITSHOW_INFO_TOTAL) {
            if (full_len > 8) {
                TOTAL = (full_array[6] << 24 | full_array[5] << 16 | full_array[4] << 8 | full_array[3]);
                debug("TOTAL " + QString::number(TOTAL));
            }
            else {
                TOTAL = -1;
            }
        }
        else if (par == FITSHOW_INFO_DATE) {
            if (full_len > 7) {
                FACTORY_DATE = QDate(full_array[3] + 2000, full_array[4], full_array[5]);
                debug("DATE " + FACTORY_DATE.toString());
            }
            else {
                FACTORY_DATE = QDate();
            }
        }
    }
    else if (cmd == FITSHOW_SYS_CONTROL) {
        SYS_CONTROL_CMD = par;
        debug("SYS_CONTROL received ok: par " + QString::number(par));
        if (par == FITSHOW_CONTROL_TARGET_OR_RUN) {
            QString dbg;
            if (full_len > 5) {
                dbg = "Actual speed " + QString::number(full_array[3] / 10.0);
                if (full_len > 6) {
                    dbg += "; actual incline: " + QString::number(full_array[4]);
                }
            }
            debug(dbg);
        }
    }
    if (cmd == FITSHOW_SYS_STATUS) {
        CURRENT_STATUS = par;
        debug("STATUS " + QString::number(par));
        if (par == FITSHOW_STATUS_START) {
            if (len > 2) {
                COUNTDOWN_VALUE = array[2];
                debug("CONTDOWN " + QString::number(COUNTDOWN_VALUE));
            }
        }
        else if (par == FITSHOW_STATUS_RUNNING ||
                 par == FITSHOW_STATUS_STOP ||
                 par == FITSHOW_STATUS_PAUSED ||
                 par == FITSHOW_STATUS_END
                 ) {
            if (full_len >= 17) {
                if (par == FITSHOW_STATUS_RUNNING)
                    IS_RUNNING = true;
                else {
                    IS_STATUS_STUDY = false;
                    IS_STATUS_ERRO = false;
                    IS_STATUS_SAFETY = false;
                    IS_RUNNING = false;
                }

                double speed = array[2] / 10.0;
                double incline = array[3];
                uint16_t seconds_elapsed = array[4] | array[5] << 8;
                double distance = (array[6] | array[7] << 8) / 10.0;
                double kcal = array[8] | array[9] << 8;
                uint16_t step_count = array[10] | array[11] << 8;
                //final byte b2 = array[13]; Mark_zuli???
                double heart = array[12];

                if (MAX_INCLINE == 0) {
                    qDebug() << "inclination out of range, resetting it to 0..." << incline;
                    incline = 0;
                }

                if (!firstCharacteristicChanged)
                    DistanceCalculated += ((speed / 3600.0) / (1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));

                debug("Current elapsed from treadmill: " + QString::number(seconds_elapsed));
                debug("Current speed: " + QString::number(speed));
                debug("Current incline: " + QString::number(incline));
                debug("Current heart: " + QString::number(heart));
                debug("Current Distance: " + QString::number(distance));
                debug("Current Distance Calculated: " + QString::number(DistanceCalculated));
                debug("Current KCal: " + QString::number(kcal));
                debug("Current step countl: " + QString::number(step_count));

                if (m_control->error() != QLowEnergyController::NoError)
                    qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();

                if (Speed.value() != speed) {
                    Speed = speed;
                    emit speedChanged(speed);
                }
                if (Inclination.value() != incline) {
                    Inclination = incline;
                    emit inclinationChanged(incline);
                }

                KCal = kcal;
                elapsed = seconds_elapsed;
                Distance = distance;
#ifdef Q_OS_ANDROID
                if (settings.value("ant_heart", false).toBool())
                    Heart = (uint8_t)KeepAwakeHelper::heart();
                else
#endif
                {
                    if (heartRateBeltName.startsWith("Disabled")) {
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
                        long appleWatchHeartRate = h->heartRate();
                        Heart = appleWatchHeartRate;
                        debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
#else
                        Heart = heart;
#endif
                    }
                }

                if (speed > 0) {
                    lastSpeed = speed;
                    lastInclination = incline;
                }

                lastTimeCharacteristicChanged = QDateTime::currentDateTime();
                firstCharacteristicChanged = false;
                if (par != FITSHOW_STATUS_RUNNING)
                    sendSportData();
            }
        }
        else {
            if (par == FITSHOW_STATUS_NORMAL) {
                sendSportData();
                IS_STATUS_STUDY = false;
                IS_STATUS_ERRO = false;
                IS_STATUS_SAFETY = false;
                IS_RUNNING = false;
            }
            else if (par == FITSHOW_STATUS_STUDY) {
                IS_STATUS_STUDY = true;
            }
            else if (par == FITSHOW_STATUS_ERROR) {
                if (len > 2) {
                    IS_STATUS_ERRO = true;
                    ERRNO = array[2];
                    sendSportData();
                }
            }
            else if (par == FITSHOW_STATUS_SAFETY) {
                ERRNO = 100;
                IS_STATUS_SAFETY = true;
                sendSportData();
            }
            if (Speed.value() != 0.0) {
                Speed = 0.0;
                emit speedChanged(0.0);
            }
            if (Inclination.value() != 0.0) {
                Inclination = 0.0;
                emit inclinationChanged(0.0);
            }
        }
    }
    else if (cmd == FITSHOW_SYS_DATA) {
        if (par == FITSHOW_DATA_INFO) {
            if (len > 13) {
                SPORT_ID = array[6] | array[7] << 8 | array[8] << 16 | array[9] << 24;
                USER_ID = array[2] | array[3] << 8 | array[4] << 16 | array[5] << 24;
                RUN_WAY = array[10];
                int indoorrun_TIME_DATA = array[12] | array[13] << 8;
                if (RUN_WAY == FITSHOW_SYS_MODE_TIMER) {
                    INDOORRUN_MODE = 2;
                    INDOORRUN_TIME_DATA = indoorrun_TIME_DATA;
                }
                else if (RUN_WAY == FITSHOW_SYS_MODE_DISTANCE) {
                    INDOORRUN_MODE = 1;
                    INDOORRUN_DISTANCE_DATA = indoorrun_TIME_DATA;
                }
                else if (RUN_WAY == FITSHOW_SYS_MODE_CALORIE) {
                    INDOORRUN_MODE = 3;
                    INDOORRUN_CALORIE_DATA = indoorrun_TIME_DATA / 10;
                }
                else if (RUN_WAY == FITSHOW_SYS_MODE_PROGRAMS) {
                    INDOORRUN_MODE = 4;
                    INDOORRUN_TIME_DATA = indoorrun_TIME_DATA;
                    INDOORRUN_PARAM_NUM = array[11];
                }
                else {
                    INDOORRUN_MODE = 0;
                }
                debug(QString("USER_ID = %1").arg(USER_ID));
                debug(QString("SPORT_ID = %1").arg(SPORT_ID));
                debug(QString("RUN_WAY = %1").arg(RUN_WAY));
                debug(QString("INDOORRUN_MODE = %1").arg(INDOORRUN_MODE));
                debug(QString("INDOORRUN_TIME_DATA = %1").arg(INDOORRUN_TIME_DATA));
                debug(QString("INDOORRUN_PARAM_NUM = %1").arg(INDOORRUN_PARAM_NUM));
                debug(QString("INDOORRUN_CALORIE_DATA = %1").arg(INDOORRUN_CALORIE_DATA));
                debug(QString("INDOORRUN_DISTANCE_DATA = %1").arg(INDOORRUN_DISTANCE_DATA));
            }
        }
        else if (par == FITSHOW_DATA_SPORT) {
            if (len > 9) {
                double kcal = array[6] | array[7] << 8;
                uint16_t seconds_elapsed = array[2] | array[3] << 8;
                double distance = array[4] | array[5] << 8;
                uint16_t step_count = array[8] | array[9] << 8;

                debug("Current elapsed from treadmill: " + QString::number(seconds_elapsed));
                debug("Current step countl: " + QString::number(step_count));
                debug("Current KCal: " + QString::number(kcal));
                debug("Current Distance: " + QString::number(distance));
                KCal = kcal;
                elapsed = seconds_elapsed;
                Distance = distance;
            }
        }
    }
}

void fitshowtreadmill::btinit(bool startTape) {
    uint8_t initInfos[] = { FITSHOW_INFO_SPEED,
                            FITSHOW_INFO_INCLINE,
                            FITSHOW_INFO_TOTAL,
                            FITSHOW_INFO_DATE };
    uint8_t initDataStart1[] = { FITSHOW_SYS_INFO, 0 };
    QDateTime now = QDateTime::currentDateTime();
    uint8_t initDataStart0[] = { FITSHOW_SYS_INFO,
                                 FITSHOW_INFO_MODEL,
                                 (uint8_t)(now.date().year() - 2000),
                                 (uint8_t)(now.date().month()),
                                 (uint8_t)(now.date().day()),
                                 (uint8_t)(now.time().hour()),
                                 (uint8_t)(now.time().minute()),
                                 (uint8_t)(now.time().second()) };

    uint8_t startTape1[] = { FITSHOW_SYS_CONTROL,
                             FITSHOW_CONTROL_READY_OR_START,
                             (FITSHOW_TREADMILL_SPORT_ID >> 0) & 0xFF,
                             (FITSHOW_TREADMILL_SPORT_ID >> 8) & 0xFF,
                             (FITSHOW_TREADMILL_SPORT_ID >> 16) & 0xFF,
                             (FITSHOW_TREADMILL_SPORT_ID >> 24) & 0xFF,
                             FITSHOW_SYS_MODE_NORMAL,
                             0x00, //number of blocks (u8)
                             0x00, 0x00 //mode-dependent value (u16le)
    };                        // to verify
    QSettings settings;
    int user_id = settings.value("fitshow_user_id", 0x006E13AA).toInt();
    uint8_t weight = (uint8_t)(settings.value("weight", 75.0).toFloat() + 0.5);
    uint8_t initUserData[] = { FITSHOW_SYS_CONTROL,
                               FITSHOW_CONTROL_USER,
                               0,
                               0,
                               0,
                               0,
                               0, };
    initUserData[2] = (user_id >> 0) & 0xFF;
    initUserData[3] = (user_id >> 8) & 0xFF;
    initUserData[4] = (user_id >> 16) & 0xFF;
    initUserData[5] = (user_id >> 24) & 0xFF;
    initUserData[6] = weight;
    scheduleWrite(initUserData, sizeof(initUserData), "init_user");
    scheduleWrite(initDataStart0, sizeof(initDataStart0), "init " + QString::number(initDataStart0[1]));
    for (uint8_t i = 0; i < sizeof(initInfos); i++) {
        initDataStart1[1] = initInfos[i];
        scheduleWrite(initDataStart1, sizeof(initDataStart1), "init " + QString::number(initDataStart1[1]));
    }
    initUserData[0] = FITSHOW_SYS_DATA;
    initUserData[1] = FITSHOW_DATA_INFO;

    if (startTape) {
        scheduleWrite(startTape1, sizeof(startTape1), "init_start");
        forceSpeedOrIncline(lastSpeed, lastInclination);
    }
    scheduleWrite(initUserData, sizeof(initUserData) - 1, "check what is going on for given user");

    initDone = true;
}

void fitshowtreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::ServiceDiscovered) {
        uint32_t id32;
        foreach(QLowEnergyCharacteristic c, gattCommunicationChannelService->characteristics()) {
            qDebug() << "c -> " << c.uuid();
            id32 = c.uuid().toUInt32();
            foreach(QLowEnergyDescriptor d, c.descriptors())
            qDebug() << "d -> " << d.uuid();
            if (id32 == 0xffe1 || id32 == 0xfff2)
                gattWriteCharacteristic = c;
            else if (id32 == 0xffe4 || id32 == 0xfff1)
                gattNotifyCharacteristic = c;
        }

        if (!gattWriteCharacteristic.isValid()) {
            qDebug() << "gattWriteCharacteristic not valid";
            return;
        }
        if (!gattNotifyCharacteristic.isValid()) {
            qDebug() << "gattNotifyCharacteristic not valid";
            return;
        }

        // establish hook into notifications
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(const QLowEnergyCharacteristic,const QByteArray)),
                this, SLOT(characteristicWritten(const QLowEnergyCharacteristic,const QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)),
                this, SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, SIGNAL(descriptorWritten(const QLowEnergyDescriptor,const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor,const QByteArray)));

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void fitshowtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void fitshowtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void fitshowtreadmill::serviceScanDone(void) {
    debug("serviceScanDone");

    gattCommunicationChannelService = m_control->createServiceObject(serviceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
#ifdef _MSC_VER
    //QTBluetooth bug on Win10 (https://bugreports.qt.io/browse/QTBUG-78488)
    QTimer::singleShot(0, [ = ] () {
        gattCommunicationChannelService->discoverDetails();
    });
#else
    gattCommunicationChannelService->discoverDetails();
#endif
}

void fitshowtreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("fitshowtreadmill::errorService " + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void fitshowtreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("fitshowtreadmill::error " + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void fitshowtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if (device.name().startsWith("FS-") || (device.name().startsWith("SW") && device.name().length() == 14)) {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, SIGNAL(serviceDiscovered(const QBluetoothUuid&)),
                this, SLOT(serviceDiscovered(const QBluetoothUuid&)));
        connect(m_control, SIGNAL(discoveryFinished()),
                this, SLOT(serviceScanDone()));
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)),
                this, SLOT(error(QLowEnergyController::Error)));
        connect(m_control, SIGNAL(stateChanged(QLowEnergyController::ControllerState)), this, SLOT(controllerStateChanged(QLowEnergyController::ControllerState)));

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            Q_UNUSED(this);
            debug("Cannot connect to remote device.");
            searchStopped = false;
            emit disconnected();
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            debug("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            debug("LowEnergy controller disconnected");
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool fitshowtreadmill::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void* fitshowtreadmill::VirtualTreadMill() {
    return virtualTreadMill;
}

void* fitshowtreadmill::VirtualDevice() {
    return VirtualTreadMill();
}

double fitshowtreadmill::odometer() {
    return DistanceCalculated;
}

void fitshowtreadmill::setLastSpeed(double speed) {
    lastSpeed = speed;
}

void fitshowtreadmill::setLastInclination(double inclination) {
    lastInclination = inclination;
}

void fitshowtreadmill::searchingStop() {
    searchStopped = true;
}

void fitshowtreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << "controllerStateChanged" << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
