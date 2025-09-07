#include "fitshowtreadmill.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualtreadmill.h"
#include "homeform.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

#define BLE_SERIALOUTPUT_MAXSIZE 25

fitshowtreadmill::fitshowtreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                   double forceInitInclination) {
    Q_UNUSED(noConsole)

    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0) {
        lastSpeed = forceInitSpeed;
    }

    if (forceInitInclination > 0) {
        lastInclination = forceInitInclination;
    }
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
    h = new lockscreen();
#endif

    refresh = new QTimer(this);
    initDone = false;
    QSettings settings;
    anyrun = settings.value(QZSettings::fitshow_anyrun, QZSettings::default_fitshow_anyrun).toBool();
    truetimer = settings.value(QZSettings::fitshow_truetimer, QZSettings::default_fitshow_truetimer).toBool();
    connect(refresh, &QTimer::timeout, this, &fitshowtreadmill::update);
    refresh->start(pollDeviceTime);
}

fitshowtreadmill::~fitshowtreadmill() {
    if (refresh) {
        refresh->stop();
        delete refresh;
    }
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
    if (h)
        delete h;
#endif
}

void fitshowtreadmill::scheduleWrite(const uint8_t *data, uint8_t data_len, const QString &info) {
    bufferWrite.append((char)data_len);
    bufferWrite.append(QByteArray((const char *)data, data_len));
    debugMsgs.append(info);
}

void fitshowtreadmill::writeCharacteristic(const uint8_t *data, uint8_t data_len, const QString &info) {
    QEventLoop loop;
    QTimer timeout;

    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
    timeout.singleShot(300ms, &loop, &QEventLoop::quit);

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (!info.isEmpty()) {
        emit debug(QStringLiteral(" >>") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

bool fitshowtreadmill::checkIncomingPacket(const uint8_t *data, uint8_t data_len) const {
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
    } else
        return false;
}

bool fitshowtreadmill::writePayload(const uint8_t *array, uint8_t size, const QString &info) {
    if (size + 3 > BLE_SERIALOUTPUT_MAXSIZE) {
        return false;
    }
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
        QSettings settings;
        // the treadmill send the speed in miles always
        double miles = 1;
        if (settings.value(QZSettings::fitshow_treadmill_miles, QZSettings::default_fitshow_treadmill_miles).toBool())
            miles = 1.60934;
        requestSpeed /= miles;
        requestSpeed *= 10.0;
        if (requestSpeed >= MAX_SPEED) {
            requestSpeed = MAX_SPEED;
        } else if (requestSpeed <= MIN_SPEED) {
            requestSpeed = MIN_SPEED;
        }
        if (requestIncline >= MAX_INCLINE) {
            requestIncline = MAX_INCLINE;
        } else if (requestIncline <= MIN_INCLINE) {
            requestIncline = MIN_INCLINE;
        }

        uint8_t writeIncline[] = {FITSHOW_SYS_CONTROL, FITSHOW_CONTROL_TARGET_OR_RUN, (uint8_t)(requestSpeed + 0.5),
                                  (uint8_t)(requestIncline * (noblepro_connected ? 2.0 : 1.0))};
        scheduleWrite(writeIncline, sizeof(writeIncline),
                      QStringLiteral("forceSpeedOrIncline speed=") + QString::number(requestSpeed) +
                          QStringLiteral(" incline=") + QString::number(requestIncline));
    }
}

void fitshowtreadmill::update() {
    if (!m_control || m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        QSettings settings;
        initRequest = false;        
        btinit(settings.value(QZSettings::atletica_lightspeed_treadmill, QZSettings::default_atletica_lightspeed_treadmill).toBool());
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotifyCharacteristic.isValid() && initDone) {
        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && searchStopped && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &fitshowtreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &fitshowtreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        emit debug(QStringLiteral("fitshow Treadmill RSSI ") + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value()) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                double inc = rawInclination.value();
                if (requestInclination != -100) {
                    // only 0.5 or 1 changes otherwise it beeps forever
                    double a = 1.0 / minStepInclination();
                    inc = qRound(treadmillInclinationOverrideReverse(requestInclination) * a) / a;
                    requestInclination = -100;
                }
                forceSpeedOrIncline(requestSpeed, inc);
            }
            requestSpeed = -1;
        }

        if (requestInclination != -100) {
            double inc = rawInclination.value();
            // only 0.5 or 1 changes otherwise it beeps forever
            double a = 1.0 / minStepInclination();
            requestInclination = qRound(treadmillInclinationOverrideReverse(requestInclination) * a) / a;
            if (requestInclination != inc) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                inc = requestInclination;
                double speed = currentSpeed().valueRaw();
                if (requestSpeed != -1) {
                    speed = requestSpeed;
                    requestSpeed = -1;
                }
                forceSpeedOrIncline(speed, inc);
            }
            requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {
                lastSpeed = 0.5;
            }

            uint8_t startTape1[] = {
                FITSHOW_SYS_CONTROL,
                FITSHOW_CONTROL_READY_OR_START,
                (FITSHOW_TREADMILL_SPORT_ID >> 0) & 0xFF,
                (FITSHOW_TREADMILL_SPORT_ID >> 8) & 0xFF,
                (FITSHOW_TREADMILL_SPORT_ID >> 16) & 0xFF,
                (FITSHOW_TREADMILL_SPORT_ID >> 24) & 0xFF,
                FITSHOW_SYS_MODE_NORMAL,
                0x00, // number of blocks (u8)
                0x00,
                0x00 // mode-dependent value (u16le)
            };       // to verify
            scheduleWrite(startTape1, sizeof(startTape1), QStringLiteral("init_start"));
            forceSpeedOrIncline(lastSpeed, lastInclination);

            lastStart = QDateTime::currentMSecsSinceEpoch();
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            if (paused) {
                lastStop = QDateTime::currentMSecsSinceEpoch();
                uint8_t pauseTape[] = {FITSHOW_SYS_CONTROL, FITSHOW_CONTROL_PAUSE}; // to verify
                emit debug(QStringLiteral("pausing..."));
                scheduleWrite(pauseTape, sizeof(pauseTape), QStringLiteral("pause tape"));
            } else {
                uint8_t stopTape[] = {FITSHOW_SYS_CONTROL, FITSHOW_CONTROL_STOP};
                emit debug(QStringLiteral("stopping..."));
                lastStop = QDateTime::currentMSecsSinceEpoch();
                scheduleWrite(stopTape, sizeof(stopTape), QStringLiteral("stop tape"));
            }
            requestStop = -1;
        }

        if (retrySend >= 6) { // 3 retries
            emit debug(QStringLiteral("WARNING: answer not received for command "
                                      "%1 / %2 (%3)")
                           .arg(((uint8_t)bufferWrite.at(1)), 2, 16, QChar('0'))
                           .arg(((uint8_t)bufferWrite.at(2)), 2, 16, QChar('0'))
                           .arg(debugMsgs.at(0)));
            removeFromBuffer();
        }
        if (!bufferWrite.isEmpty()) {
            retrySend++;
            if (retrySend % 2) { // retry only on odd values: on even values wait some more time for response
                const uint8_t *write_pld = (const uint8_t *)bufferWrite.constData();
                writePayload(write_pld + 1, write_pld[0], debugMsgs.at(0));
            }
        } else {
            uint8_t status = FITSHOW_SYS_STATUS;
            writePayload(&status, 1);
        }
    }
}

void fitshowtreadmill::removeFromBuffer() {
    if (!bufferWrite.isEmpty()) {
        bufferWrite.remove(0, ((uint8_t)bufferWrite.at(0)) + 1);
    }
    if (!debugMsgs.isEmpty()) {
        debugMsgs.removeFirst();
    }
    retrySend = 0;
}

void fitshowtreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    uint32_t servRepr = gatt.toUInt32();
    QBluetoothUuid nobleproconnect(QStringLiteral("0000ae00-0000-1000-8000-00805f9b34fb"));
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString() + QStringLiteral(" ") +
               QString::number(servRepr));
    if ((gatt == nobleproconnect && serviceId.isNull()) || servRepr == 0xfff0 || (servRepr == 0xffe0 && serviceId.isNull())) {
        qDebug() << "adding" << gatt.toString() << "as the default service";
        serviceId = gatt; // NOTE: clazy-rule-of-tow
    }
    if(gatt == QBluetoothUuid((quint16)0x1826) && !fs_connected && !tunturi_t80_connected) {
        QSettings settings;
        settings.setValue(QZSettings::ftms_treadmill, bluetoothDevice.name());
        qDebug() << "forcing FTMS treadmill since it has FTMS";
        if(homeform::singleton())
            homeform::singleton()->setToastRequested("FTMS treadmill found, restart the app to apply the change");
    }
}

void fitshowtreadmill::sendSportData() {
    if (!anyrun) {
        uint8_t writeSport[] = {FITSHOW_SYS_DATA, FITSHOW_DATA_SPORT};
        scheduleWrite(writeSport, sizeof(writeSport), QStringLiteral("SendSportsData"));
    }
}

void fitshowtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    emit debug(QStringLiteral("packetReceived!"));
    emit packetReceived();

    lastPacket = value;

    if(characteristic.uuid() == QBluetoothUuid((quint16)0x2a53) && value.length() >= 4) {
        Cadence = value.at(3);
        emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
        return;
    }

    const uint8_t *full_array = (uint8_t *)value.constData();
    uint8_t full_len = value.length();
    if (!checkIncomingPacket(full_array, full_len)) {
        emit debug(QStringLiteral("Invalid packet"));
        return;
    }
    const uint8_t *array = full_array + 1;
    const uint8_t cmd = array[0];
    const uint8_t par = array[1];

    const uint8_t *array_expected = (const uint8_t *)bufferWrite.constData() + 1;
    uint8_t len = full_len - 3;
    if (cmd != FITSHOW_SYS_STATUS && !bufferWrite.isEmpty() && *array_expected == cmd && *(array_expected + 1) == par) {
        removeFromBuffer();
    }
    if (cmd == FITSHOW_SYS_INFO) {
        if (par == FITSHOW_INFO_SPEED) {
            if (full_len > 6) {
                MAX_SPEED = full_array[3];
                MIN_SPEED = full_array[4];
                emit debug(QStringLiteral("Speed between ") + QString::number(MIN_SPEED) + QStringLiteral(" and ") +
                           QString::number(MAX_SPEED));
                if (full_len > 7) {
                    UNIT = full_array[5];
                }
            }
        } else if (par == FITSHOW_INFO_UNKNOWN) {
            if (full_len >= 9) {
                MAX_SPEED = full_array[3];
                MIN_SPEED = full_array[4];
                MAX_INCLINE = full_array[5];
                MIN_INCLINE = full_array[6];
                IS_HRC = (bool)full_array[7];
                COUNTDOWN_VALUE = full_array[8];
            }
        } else if (par == FITSHOW_INFO_INCLINE) {
            if (full_len < 7) {
                MAX_INCLINE = 0;
                emit debug(QStringLiteral("Incline not supported"));
            } else {
                MAX_INCLINE = full_array[3];
                MIN_INCLINE = full_array[4];
                if (full_len > 7 && (full_array[5] & 0x2) != 0x0) {
                    IS_PAUSE = true;
                }
                emit debug(QStringLiteral("Incline between ") + QString::number(MIN_INCLINE) + QStringLiteral(" and ") +
                           QString::number(MAX_INCLINE));
            }
        } else if (par == FITSHOW_INFO_MODEL) {
            if (full_len > 7) {
                uint16_t second = (full_array[5] << 8) | full_array[4];
                DEVICE_ID_NAME = QStringLiteral("%1-%2")
                                     .arg(full_array[3], 2, 16, QLatin1Char('0'))
                                     .arg(second, 4, 16, QLatin1Char('0'));
                emit debug(QStringLiteral("DEVICE ") + DEVICE_ID_NAME);
            }
        } else if (par == FITSHOW_INFO_TOTAL) {
            if (full_len > 8) {
                TOTAL = (full_array[6] << 24 | full_array[5] << 16 | full_array[4] << 8 | full_array[3]);
                emit debug(QStringLiteral("TOTAL ") + QString::number(TOTAL));
            } else {
                TOTAL = -1;
            }
        } else if (par == FITSHOW_INFO_DATE) {
            if (full_len > 7) {
                FACTORY_DATE = QDate(full_array[3] + 2000, full_array[4], full_array[5]);
                emit debug(QStringLiteral("DATE ") + FACTORY_DATE.toString());
            } else {
                FACTORY_DATE = QDate();
            }
        }
    } else if (cmd == FITSHOW_SYS_CONTROL) {
        SYS_CONTROL_CMD = par;
        emit debug(QStringLiteral("SYS_CONTROL received ok: par ") + QString::number(par));
        if (par == FITSHOW_CONTROL_TARGET_OR_RUN) {
            QString dbg;
            if (full_len > 5) {
                dbg = QStringLiteral("Actual speed ") + QString::number(full_array[3] / 10.0);
                if (full_len > 6) {
                    dbg += QStringLiteral("; actual incline: ") + QString::number(full_array[4]);
                }
            }
            emit debug(dbg);
        }
    }
    if (cmd == FITSHOW_SYS_STATUS) {
        CURRENT_STATUS = par;
        emit debug(QStringLiteral("STATUS ") + QString::number(par));
        if (par == FITSHOW_STATUS_START) {
            if (len > 2) {
                COUNTDOWN_VALUE = array[2];
                emit debug(QStringLiteral("CONTDOWN ") + QString::number(COUNTDOWN_VALUE));
            }
        } else if (par == FITSHOW_STATUS_RUNNING || par == FITSHOW_STATUS_STOP || par == FITSHOW_STATUS_PAUSED ||
                   par == FITSHOW_STATUS_END) {
            if (full_len >= 16) {
                if (par == FITSHOW_STATUS_RUNNING)
                    IS_RUNNING = true;
                else {
                    IS_STATUS_STUDY = false;
                    IS_STATUS_ERRO = false;
                    IS_STATUS_SAFETY = false;
                    IS_RUNNING = false;
                }

                double speed = array[2] / 10.0;
                double incline = (int8_t)array[3];
                uint16_t seconds_elapsed = anyrun ? array[4] * 60 + array[5] : array[4] | array[5] << 8;
                double distance = (anyrun ? (array[7] | array[6] << 8) : (array[6] | array[7] << 8)) / 10.0;
                double kcal = anyrun ? (array[9] | array[8] << 8) : (array[8] | array[9] << 8);
                uint16_t step_count = anyrun ? (array[11] | array[10] << 8) : (array[10] | array[11] << 8);
                // final byte b2 = array[13]; Mark_zuli???
                double heart = array[12];

                if (MAX_INCLINE == 0) {
                    qDebug() << QStringLiteral("inclination out of range, resetting it to 0...") << incline;
                    incline = 0;
                }

                if (!firstCharacteristicChanged) {
                    if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                        KCal +=
                            ((((0.048 * ((double)watts(
                                            settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                                1.19) *
                               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                              200.0) /
                             (60000.0 /
                              ((double)lastTimeCharacteristicChanged.msecsTo(
                                  QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                    // kg * 3.5) / 200 ) / 60
                    DistanceCalculated +=
                        ((speed / 3600.0) /
                         (1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
                    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
                }

                StepCount = step_count;

                emit debug(QStringLiteral("Current elapsed from treadmill: ") + QString::number(seconds_elapsed));
                emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
                emit debug(QStringLiteral("Current heart: ") + QString::number(heart));
                emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
                emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(DistanceCalculated));
                emit debug(QStringLiteral("Current KCal from the Machine: ") + QString::number(kcal));
                emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
                emit debug(QStringLiteral("Current step countl: ") + QString::number(step_count));

                if (m_control->error() != QLowEnergyController::NoError) {
                    qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
                }

                if (speed > 0) {
                    lastStart =
                        0; // telling to the UI that it could be autostoppable when the speed it will reach again 0
                } else {
                    lastStop = 0;
                }

                // the treadmill send the speed in miles always
                double miles = 1;
                if (settings.value(QZSettings::fitshow_treadmill_miles, QZSettings::default_fitshow_treadmill_miles)
                        .toBool())
                    miles = 1.60934;

                if(IS_RUNNING)
                    Speed = speed * miles;
                else
                    Speed = 0;

                if (Speed.value() != speed) {
                    emit speedChanged(speed);
                }

                if (noblepro_connected)
                    incline /= 2;

                rawInclination = incline;
                Inclination = treadmillInclinationOverride(incline);
                if (Inclination.value() != incline) {
                    emit inclinationChanged(0, incline);
                }
                emit debug(QStringLiteral("Current incline: ") + QString::number(Inclination.value()));

                if (truetimer)
                    elapsed = seconds_elapsed;
                Distance = DistanceCalculated;
#ifdef Q_OS_ANDROID
                if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
                    Heart = (uint8_t)KeepAwakeHelper::heart();
                else
#endif
                {
                    if (settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString().startsWith(QStringLiteral("Disabled"))) {
                        if (heart == 0 || disable_hr_frommachinery)
                            update_hr_from_external();
                        else 
                            Heart = heart;
                    }
                }

                if (speed > 0) {
                    lastSpeed = speed;
                    lastInclination = incline;
                }

                firstCharacteristicChanged = false;
                if (par != FITSHOW_STATUS_RUNNING) {
                    sendSportData();
                }
            }
        } else {
            if (par == FITSHOW_STATUS_NORMAL) {
                sendSportData();
                IS_STATUS_STUDY = false;
                IS_STATUS_ERRO = false;
                IS_STATUS_SAFETY = false;
                IS_RUNNING = false;
            } else if (par == FITSHOW_STATUS_STUDY) {
                IS_STATUS_STUDY = true;
            } else if (par == FITSHOW_STATUS_ERROR) {
                if (len > 2) {
                    IS_STATUS_ERRO = true;
                    ERRNO = array[2];
                    sendSportData();
                }
            } else if (par == FITSHOW_STATUS_SAFETY) {
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
                emit inclinationChanged(0.0, 0.0);
            }
        }
    } else if (cmd == FITSHOW_SYS_DATA) {
        if (par == FITSHOW_DATA_INFO) {
            if (len > 13) {
                SPORT_ID = array[6] | array[7] << 8 | array[8] << 16 | array[9] << 24;
                USER_ID = array[2] | array[3] << 8 | array[4] << 16 | array[5] << 24;
                RUN_WAY = array[10];
                int indoorrun_TIME_DATA = array[12] | array[13] << 8;
                if (RUN_WAY == FITSHOW_SYS_MODE_TIMER) {
                    INDOORRUN_MODE = 2;
                    INDOORRUN_TIME_DATA = indoorrun_TIME_DATA;
                } else if (RUN_WAY == FITSHOW_SYS_MODE_DISTANCE) {
                    INDOORRUN_MODE = 1;
                    INDOORRUN_DISTANCE_DATA = indoorrun_TIME_DATA;
                } else if (RUN_WAY == FITSHOW_SYS_MODE_CALORIE) {
                    INDOORRUN_MODE = 3;
                    INDOORRUN_CALORIE_DATA = indoorrun_TIME_DATA / 10;
                } else if (RUN_WAY == FITSHOW_SYS_MODE_PROGRAMS) {
                    INDOORRUN_MODE = 4;
                    INDOORRUN_TIME_DATA = indoorrun_TIME_DATA;
                    INDOORRUN_PARAM_NUM = array[11];
                } else {
                    INDOORRUN_MODE = 0;
                }
                emit debug(QStringLiteral("USER_ID = %1").arg(USER_ID));
                emit debug(QStringLiteral("SPORT_ID = %1").arg(SPORT_ID));
                emit debug(QStringLiteral("RUN_WAY = %1").arg(RUN_WAY));
                emit debug(QStringLiteral("INDOORRUN_MODE = %1").arg(INDOORRUN_MODE));
                emit debug(QStringLiteral("INDOORRUN_TIME_DATA = %1").arg(INDOORRUN_TIME_DATA));
                emit debug(QStringLiteral("INDOORRUN_PARAM_NUM = %1").arg(INDOORRUN_PARAM_NUM));
                emit debug(QStringLiteral("INDOORRUN_CALORIE_DATA = %1").arg(INDOORRUN_CALORIE_DATA));
                emit debug(QStringLiteral("INDOORRUN_DISTANCE_DATA = %1").arg(INDOORRUN_DISTANCE_DATA));
            }
        } else if (par == FITSHOW_DATA_SPORT) {
            if (len > 9) {
                double kcal = array[6] | array[7] << 8;
                uint16_t seconds_elapsed = array[2] | array[3] << 8;
                double distance = array[4] | array[5] << 8;
                uint16_t step_count = array[8] | array[9] << 8;

                StepCount = step_count;

                emit debug(QStringLiteral("Current elapsed from treadmill: ") + QString::number(seconds_elapsed));
                emit debug(QStringLiteral("Current step countl: ") + QString::number(step_count));
                emit debug(QStringLiteral("Current KCal from the machine: ") + QString::number(kcal));
                emit debug(QStringLiteral("Current Distance from the machine: ") + QString::number(distance));
                // KCal = kcal;
                if (truetimer)
                    elapsed = seconds_elapsed;
                // Distance = distance;
            }
        }
    }
}

void fitshowtreadmill::btinit(bool startTape) {
    uint8_t initInfos[] = {FITSHOW_INFO_SPEED, FITSHOW_INFO_INCLINE, FITSHOW_INFO_TOTAL, FITSHOW_INFO_DATE};
    uint8_t initDataStart1[] = {FITSHOW_SYS_INFO, FITSHOW_INFO_UNKNOWN};

    QDateTime now = QDateTime::currentDateTime();
    uint8_t initDataStart0[] = {FITSHOW_SYS_INFO,
                                FITSHOW_INFO_MODEL,
                                (uint8_t)(now.date().year() - 2000),
                                (uint8_t)(now.date().month()),
                                (uint8_t)(now.date().day()),
                                (uint8_t)(now.time().hour()),
                                (uint8_t)(now.time().minute()),
                                (uint8_t)(now.time().second())};

    uint8_t startTape1[] = {
        FITSHOW_SYS_CONTROL,
        FITSHOW_CONTROL_READY_OR_START,
        (FITSHOW_TREADMILL_SPORT_ID >> 0) & 0xFF,
        (FITSHOW_TREADMILL_SPORT_ID >> 8) & 0xFF,
        (FITSHOW_TREADMILL_SPORT_ID >> 16) & 0xFF,
        (FITSHOW_TREADMILL_SPORT_ID >> 24) & 0xFF,
        FITSHOW_SYS_MODE_NORMAL,
        0x00, // number of blocks (u8)
        0x00,
        0x00 // mode-dependent value (u16le)
    };       // to verify
    QSettings settings;
    int user_id = settings.value(QZSettings::fitshow_user_id, QZSettings::default_fitshow_user_id).toInt();
    uint8_t weight = (uint8_t)(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() + 0.5);
    uint8_t initUserData[] = {FITSHOW_SYS_CONTROL, FITSHOW_CONTROL_USER, 0, 0, 0, 0, 0, 0};
    initUserData[2] = (user_id >> 0) & 0xFF;
    initUserData[3] = (user_id >> 8) & 0xFF;
    initUserData[4] = 110;
    initUserData[5] = 30; // age
    initUserData[6] = weight;
    initUserData[7] = 170; // height
    scheduleWrite(initUserData, sizeof(initUserData) - (!anyrun ? 1 : 0), QStringLiteral("init_user"));
    if (!anyrun)
        scheduleWrite(initDataStart0, sizeof(initDataStart0),
                      QStringLiteral("init ") + QString::number(initDataStart0[1]));
    for (uint8_t i = 0; i < sizeof(initInfos); i++) {
        if (!anyrun)
            initDataStart1[1] = initInfos[i];
        scheduleWrite(initDataStart1, sizeof(initDataStart1),
                      QStringLiteral("init ") + QString::number(initDataStart1[1]));
        if (anyrun)
            break;
    }

    if (startTape) {
        scheduleWrite(startTape1, sizeof(startTape1), QStringLiteral("init_start"));
        forceSpeedOrIncline(lastSpeed, lastInclination);
    }

    // initUserData[0] = FITSHOW_SYS_DATA;
    // initUserData[1] = FITSHOW_DATA_INFO;
    // scheduleWrite(initUserData, sizeof(initUserData) - 2, QStringLiteral("check what is going on for given user"));

    initDone = true;
}

void fitshowtreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(gattCommunicationRSCService != nullptr && (gattCommunicationRSCService->state() != QLowEnergyService::ServiceDiscovered ||
        gattCommunicationChannelService->state() != QLowEnergyService::ServiceDiscovered)) {
        qDebug() << QStringLiteral("not all services discovered");
        return;        
    }

    if (state == QLowEnergyService::ServiceDiscovered) {
        uint32_t id32;
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("c -> ") << c.uuid() << c.properties();
            id32 = c.uuid().toUInt32();
            auto descriptors_list = c.descriptors();
            for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                qDebug() << QStringLiteral("d -> ") << d.uuid();
            }
            if (id32 == 0xffe1 || id32 == 0xfff2 || id32 == 0xae01) {
                gattWriteCharacteristic = c;
            } else if (id32 == 0xffe4 || id32 == 0xfff1 || id32 == 0xae02) {
                gattNotifyCharacteristic = c;
            }
        }

        if (!gattWriteCharacteristic.isValid()) {
            qDebug() << QStringLiteral("gattWriteCharacteristic not valid");
            return;
        }
        if (!gattNotifyCharacteristic.isValid()) {
            qDebug() << QStringLiteral("gattNotifyCharacteristic not valid");
            return;
        }

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &fitshowtreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &fitshowtreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &fitshowtreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &fitshowtreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }

    if(gattCommunicationRSCService != nullptr) {
        connect(gattCommunicationRSCService, &QLowEnergyService::characteristicChanged, this, &fitshowtreadmill::characteristicChanged);

        qDebug() << gattCommunicationRSCService->serviceUuid() << QStringLiteral("connected!");

        auto characteristics_list = gattCommunicationRSCService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            auto descriptors_list = c.descriptors();
            for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
            }

            if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                QByteArray descriptor;
                descriptor.append((char)0x01);
                descriptor.append((char)0x00);
                if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                    gattCommunicationRSCService->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                } else {
                    qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                << QStringLiteral(" is not valid");
                }

                qDebug() << gattCommunicationRSCService->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
            }
        }        
    }
}

void fitshowtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void fitshowtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void fitshowtreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    gattCommunicationChannelService = m_control->createServiceObject(serviceId);
    if (!gattCommunicationChannelService) {
        qDebug() << "service not valid";
        return;
    }    
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &fitshowtreadmill::stateChanged);
#ifdef _MSC_VER
    // QTBluetooth bug on Win10 (https://bugreports.qt.io/browse/QTBUG-78488)
    QTimer::singleShot(0, [=]() { gattCommunicationChannelService->discoverDetails(); });
#else
    gattCommunicationChannelService->discoverDetails();
#endif

    // useful for the cadence
    gattCommunicationRSCService = m_control->createServiceObject(QBluetoothUuid((quint16)0x1814));
    if (!gattCommunicationRSCService) {
        qDebug() << "gattCommunicationFTMSService not valid";
        return;
    } 

#ifdef _MSC_VER
    // QTBluetooth bug on Win10 (https://bugreports.qt.io/browse/QTBUG-78488)
    QTimer::singleShot(0, [=]() { gattCommunicationRSCService->discoverDetails(); });
#else
    gattCommunicationRSCService->discoverDetails();
#endif       
}

void fitshowtreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("fitshowtreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void fitshowtreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("fitshowtreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void fitshowtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    if (device.name().toUpper().startsWith(QStringLiteral("FS-"))) {
        qDebug() << "FS FIX!";
        fs_connected = true;
    } else if (device.name().toUpper().startsWith(QStringLiteral("NOBLEPRO CONNECT"))) {
        qDebug() << "NOBLEPRO FIX!";
        minStepInclinationValue = 0.5;
        noblepro_connected = true;
    } else if (device.name().toUpper().startsWith(QStringLiteral("TUNTURI T80-"))) {
        qDebug() << "TUNTURI T80 detected - ignoring FTMS forcing";
        tunturi_t80_connected = true;
    }

    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &fitshowtreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &fitshowtreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &fitshowtreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &fitshowtreadmill::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    searchStopped = false;
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
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool fitshowtreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void fitshowtreadmill::searchingStop() { searchStopped = true; }

void fitshowtreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool fitshowtreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool fitshowtreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}

double fitshowtreadmill::minStepInclination() { return minStepInclinationValue; }

void fitshowtreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(qRound(grade), qRound(percentage));
}
