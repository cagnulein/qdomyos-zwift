#include "kingsmithr2treadmill.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

kingsmithr2treadmill::kingsmithr2treadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService,
                                           double forceInitSpeed, double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0) {
        lastSpeed = forceInitSpeed;
    }

    if (forceInitInclination > 0) {
        lastInclination = forceInitInclination;
    }
    if (lastControlMode != UNKNOWN_CONTROL_MODE) {
        lastControlMode = UNKNOWN_CONTROL_MODE;
    }
    if (lastRunState != UNKNOWN_RUN_STATE) {
        lastRunState = UNKNOWN_RUN_STATE;
    }

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &kingsmithr2treadmill::update);
    refresh->start(pollDeviceTime);
}

void kingsmithr2treadmill::writeCharacteristic(const QString &data, const QString &info, bool disable_log,
                                               bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &kingsmithr2treadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("writeCharacteristic error because the connection is closed"));

        return;
    }

    QByteArray input = data.toUtf8().toBase64();
    QByteArray encrypted;
    for (int i = 0; i < input.length(); i++) {
        int idx = PLAINTEXT_TABLE.indexOf(input.at(i));
        QSettings settings;
        if(KS_NACH_MXG)
            encrypted.append(ENCRYPT_TABLE_v7[idx]);
        else if (settings.value(QZSettings::kingsmith_encrypt_v2, QZSettings::default_kingsmith_encrypt_v2).toBool())
            encrypted.append(ENCRYPT_TABLE_v2[idx]);
        else if (settings.value(QZSettings::kingsmith_encrypt_v3, QZSettings::default_kingsmith_encrypt_v3).toBool())
            encrypted.append(ENCRYPT_TABLE_v3[idx]);
        else if (settings.value(QZSettings::kingsmith_encrypt_v4, QZSettings::default_kingsmith_encrypt_v4).toBool())
            encrypted.append(ENCRYPT_TABLE_v4[idx]);
        else if (settings.value(QZSettings::kingsmith_encrypt_v5, QZSettings::default_kingsmith_encrypt_v5).toBool())
            encrypted.append(ENCRYPT_TABLE_v5[idx]);
        else if (settings.value(QZSettings::kingsmith_encrypt_g1_walking_pad, QZSettings::default_kingsmith_encrypt_g1_walking_pad).toBool())
            encrypted.append(ENCRYPT_TABLE_v6[idx]);
        else
            encrypted.append(ENCRYPT_TABLE[idx]);
    }
    if (!disable_log) {
        emit debug(QStringLiteral(" >> plain: ") + data + QStringLiteral(" // ") + info);
        emit debug(QStringLiteral(" >> base64: ") + QString(input) + QStringLiteral(" // ") + info);
        emit debug(QStringLiteral(" >> encrypted: ") + QString(encrypted) + QStringLiteral(" // ") + info);
    }
    encrypted.append('\x0d');
    for (int i = 0; i < encrypted.length(); i += 16) {
        // it's missing the writeBuffer here, it could lead to crash on iOS
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, encrypted.mid(i, 16),
                                                             QLowEnergyService::WriteWithoutResponse);
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + encrypted.toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void kingsmithr2treadmill::updateDisplay(uint16_t elapsed) {}

void kingsmithr2treadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline) {
    Q_UNUSED(requestIncline)
    QString speed = QString::number(requestSpeed);
    writeCharacteristic(QStringLiteral("props CurrentSpeed ") + speed, QStringLiteral("forceSpeed") + speed, false,
                        true);
}

void kingsmithr2treadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void kingsmithr2treadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    } else if (/*bluetoothDevice.isValid() &&*/
               m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() && gattNotifyCharacteristic.isValid() && initDone) {

        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();

            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &kingsmithr2treadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &kingsmithr2treadmill::changeInclinationRequested);                    
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &kingsmithr2treadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        // debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ >= (1000 / refresh->interval())) {

            sec1Update = 0;
            updateDisplay(elapsed.value());
        } else {
            // writeCharacteristic(QStringLiteral(""), QStringLiteral("noOp"), false, true);
        }

        // byte 3 - 4 = elapsed time
        // byte 17    = inclination
        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));

                double inc = Inclination.value();
                if (requestInclination != -100) {

                    // only 0.5 steps ara available
                    requestInclination = qRound(requestInclination * 2.0) / 2.0;
                    inc = requestInclination;
                    requestInclination = -100;
                }
                forceSpeedOrIncline(requestSpeed, inc);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            if (requestInclination < 0)
                requestInclination = 0;
            // only 0.5 steps ara available
            requestInclination = qRound(requestInclination * 2.0) / 2.0;
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));

                double speed = currentSpeed().value();
                if (requestSpeed != -1) {

                    speed = requestSpeed;
                    requestSpeed = -1;
                }
                forceSpeedOrIncline(speed, requestInclination);
            }
            requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastControlMode != MANUAL) {
                writeCharacteristic(QStringLiteral("props ControlMode 1"), QStringLiteral("turn on treadmill to manual mode"),
                                    false, true);
            }
            if (lastRunState != START) {
                writeCharacteristic(QStringLiteral("props runState 1"), QStringLiteral("starting"), false, true);
            }
            if (lastSpeed == 0.0) {
                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            if (lastRunState != STOP) {
                writeCharacteristic(QStringLiteral("props runState 0"), QStringLiteral("stopping"), false, true);
            }
            // don't go to standby mode automatically
            requestStop = -1;
        }
        if (requestFanSpeed != -1) {
            emit debug(QStringLiteral("changing fan speed..."));

            // sendChangeFanSpeed(requestFanSpeed);
            requestFanSpeed = -1;
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

void kingsmithr2treadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void kingsmithr2treadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    buffer.append(value);
    if (value.back() != '\x0d') {
        emit debug(QStringLiteral("packet not finished"));
        return;
    }
    QByteArray decrypted;
    for (int i = 0; i < buffer.length(); i++) {
        char ch = buffer.at(i);
        if (ch == '\x0d') {
            continue;
        }
        int idx;
        QSettings settings;
        if(KS_NACH_MXG)
            idx = ENCRYPT_TABLE_v7.indexOf(ch);
        else if (settings.value(QZSettings::kingsmith_encrypt_v2, QZSettings::default_kingsmith_encrypt_v2).toBool())
            idx = ENCRYPT_TABLE_v2.indexOf(ch);
        else if (settings.value(QZSettings::kingsmith_encrypt_v3, QZSettings::default_kingsmith_encrypt_v3).toBool())
            idx = ENCRYPT_TABLE_v3.indexOf(ch);
        else if (settings.value(QZSettings::kingsmith_encrypt_v4, QZSettings::default_kingsmith_encrypt_v4).toBool())
            idx = ENCRYPT_TABLE_v4.indexOf(ch);
        else if (settings.value(QZSettings::kingsmith_encrypt_v5, QZSettings::default_kingsmith_encrypt_v5).toBool())
            idx = ENCRYPT_TABLE_v5.indexOf(ch);
        else if (settings.value(QZSettings::kingsmith_encrypt_g1_walking_pad, QZSettings::default_kingsmith_encrypt_g1_walking_pad).toBool())
            idx = ENCRYPT_TABLE_v6.indexOf(ch);
        else
            idx = ENCRYPT_TABLE.indexOf(ch);
        decrypted.append(PLAINTEXT_TABLE[idx]);
    }
    buffer.clear();
    lastValue = QByteArray::fromBase64(decrypted);

    emit packetReceived();

    QString data = QString(lastValue);
    emit debug(QStringLiteral(" << decrypted: ") + QString(data));

    if (!data.startsWith("props")) {
        // TODO
        return;
    }

    QStringList _props = data.split(QStringLiteral(" "), QString::SkipEmptyParts);
    for (int i = 1; i < _props.size(); i += 2) {
        QString key = _props.at(i);
        // Error key only can have error code
        // props Error "ErrorCode" -5000
        if (!key.compare(QStringLiteral("Error"))) {
            break;
        }
        // skip string params
        if (!key.compare(QStringLiteral("mcu_version")) || !key.compare(QStringLiteral("goal"))) {
            continue;
        }
        if (i + 1 >= _props.count()) {
            qDebug() << "error decoding" << i;
            return;
        }
        QString value = _props.at(i + 1);
        emit debug(key + ": " + value);
        props[key] = value.toDouble();
    }

    double speed = props.value("CurrentSpeed", 0);
    Cadence = props.value("spm", 0);
    KINGSMITH_R2_CONTROL_MODE controlMode = (KINGSMITH_R2_CONTROL_MODE)(int)props.value("ControlMode", (double)UNKNOWN_CONTROL_MODE);
    KINGSMITH_R2_RUN_STATE runState = (KINGSMITH_R2_RUN_STATE)(int)props.value("runState", (double)UNKNOWN_RUN_STATE);

    // TODO:
    // - RunningDistance (int; meter) : update each 10miters / 0.01 mile
    // - RunningSteps (int) : update 2 steps
    // - BurnCalories (int) : KCal * 1000
    // - RunningTotalTime (int; sec)
    // - spm (int) : steps per minute

    // TODO: check 'ControlMode' and 'runState' of treadmill side
    // then update current running status of application.

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {

            uint8_t heart = 0;
            if (heart == 0) {
                update_hr_from_external();
            } else {
                Heart = heart;
            }
        }
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

        Distance += ((speed / (double)3600.0) /
                     ((double)1000.0 / (double)(lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
        lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    // emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    // emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    if (Speed.value() != speed) {

        emit speedChanged(speed);
    }
    Speed = speed;

    if (speed > 0) {

        lastSpeed = speed;
        // lastInclination = incline;
    }

    if (lastControlMode != controlMode) {
        lastControlMode = controlMode;
        if (controlMode != UNKNOWN_CONTROL_MODE) {
            emit debug(QStringLiteral("kingsmith r2 is ready"));
            initDone = true;
        }
    }
    if (lastRunState != runState) {
        lastRunState = runState;
        if (runState == START) {
            emit debug(QStringLiteral("start button pressed on treadmill!"));
            emit buttonHWStart();  // Signal for physical hardware button press, it starts the treadmill automatically
        } else if (runState == STOP) {
            emit debug(QStringLiteral("stop button pressed on treadmill!"));
            emit buttonHWStop();   // Signal for physical hardware button press, it stops the treadmill automatically
        }
    }
    firstCharacteristicChanged = false;
}

void kingsmithr2treadmill::btinit(bool startTape) {
    emit debug(QStringLiteral("btinit"));

    writeCharacteristic(QStringLiteral(""), QStringLiteral("init"), false, true);
    // format error
    writeCharacteristic(QStringLiteral("shake"), QStringLiteral("init"), false, true);
    // shake 00
    writeCharacteristic(QStringLiteral("net"), QStringLiteral("init"), false, true);
    // net cloud
    writeCharacteristic(QStringLiteral("get_dn"), QStringLiteral("init"), false, true);
    // get_dn XXXX...
    writeCharacteristic(QStringLiteral("get_pk"), QStringLiteral("init"), false, true);
    // get_pk XXXX...
    uint64_t timestamp = QDateTime::currentSecsSinceEpoch();
    writeCharacteristic(QStringLiteral("time_posix %1").arg(timestamp), QStringLiteral("init"), false, true);
    // time_posix 0
    writeCharacteristic(QStringLiteral("version"), QStringLiteral("init"), false, true);
    // version 0014

    // read current properties
    // writeCharacteristic(
    //    QStringLiteral("servers getProp 1 3 7 8 9 16 17 18 19 21 22 23 24 31"), QStringLiteral("init"), false, true);
    writeCharacteristic(QStringLiteral("servers getProp 1 2 7 12 23 24 31"), QStringLiteral("init"), false, true);

    initDone = true;
}

void kingsmithr2treadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xFED7);
    QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xFED8);

    if (KS_NACH_X21C || KS_NGCH_G1C_2 || KS_HDSY_X21C_2) {
        _gattWriteCharacteristicId = QBluetoothUuid(QStringLiteral("0002FED7-0000-1000-8000-00805f9b34fb"));
        _gattNotifyCharacteristicId = QBluetoothUuid(QStringLiteral("0002FED8-0000-1000-8000-00805f9b34fb"));
    } else if (KS_NGCH_G1C || KS_NACH_MXG || KS_NACH_X21C_2) {
        _gattWriteCharacteristicId = QBluetoothUuid(QStringLiteral("0001FED7-0000-1000-8000-00805f9b34fb"));
        _gattNotifyCharacteristicId = QBluetoothUuid(QStringLiteral("0001FED8-0000-1000-8000-00805f9b34fb"));
    }

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::DiscoveringServices) {
    }
    if (state == QLowEnergyService::ServiceDiscovered) {

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &kingsmithr2treadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &kingsmithr2treadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &kingsmithr2treadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &kingsmithr2treadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void kingsmithr2treadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void kingsmithr2treadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void kingsmithr2treadmill::serviceScanDone(void) {
    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0x1234);
    emit debug(QStringLiteral("serviceScanDone"));

    if (KS_NACH_X21C)
        _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("00021234-0000-1000-8000-00805f9b34fb"));
    else if(KS_NGCH_G1C || KS_NACH_MXG)
        _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("00011234-0000-1000-8000-00805f9b34fb"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if(gattCommunicationChannelService == nullptr && KS_NACH_X21C) {
        KS_NACH_X21C_2 = true;
        KS_NACH_X21C = false;
        qDebug() << "KS_NACH_X21C default service id not found";
        _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("00011234-0000-1000-8000-00805f9b34fb"));
        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    } else if(gattCommunicationChannelService == nullptr && KS_HDSY_X21C) {
        KS_HDSY_X21C_2 = true;
        KS_HDSY_X21C = false;
        qDebug() << "KS_HDSY_X21C default service id not found";
        _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("00021234-0000-1000-8000-00805f9b34fb"));
        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    } else if(gattCommunicationChannelService == nullptr && KS_NGCH_G1C) {
        KS_NGCH_G1C_2 = true;
        KS_NGCH_G1C = false;
        qDebug() << "KS_NGCH_G1C default service id not found";
        _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("00021234-0000-1000-8000-00805f9b34fb"));
        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    }
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &kingsmithr2treadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void kingsmithr2treadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("kingsmithr2treadmill::errorService ") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void kingsmithr2treadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("kingsmithr2treadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void kingsmithr2treadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {

        bluetoothDevice = device;
        if (device.name().toUpper().startsWith(QStringLiteral("KS-NACH-X21C"))) {
            qDebug() << "KS-NACH-X21C workaround!";
            KS_NACH_X21C = true;
        } else if (device.name().toUpper().startsWith(QStringLiteral("KS-HDSY-X21C"))) {
            qDebug() << "KS-HDSY-X21C workaround!";
            KS_HDSY_X21C = true;
        } else if (device.name().toUpper().startsWith(QStringLiteral("KS-NGCH-G1C"))) {
            qDebug() << "KS-NGCH-G1C workaround!";
            KS_NGCH_G1C = true;
        } else if (device.name().toUpper().startsWith(QStringLiteral("KS-NACH-MXG"))) {
            qDebug() << "KS-NACH-MXG workaround!";
            KS_NACH_MXG = true;
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &kingsmithr2treadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &kingsmithr2treadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &kingsmithr2treadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &kingsmithr2treadmill::controllerStateChanged);

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

void kingsmithr2treadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool kingsmithr2treadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void kingsmithr2treadmill::searchingStop() { searchStopped = true; }
