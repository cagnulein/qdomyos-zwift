#include "kingsmithr1protreadmill.h"

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

kingsmithr1protreadmill::kingsmithr1protreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService,
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

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &kingsmithr1protreadmill::update);
    refresh->start(pollDeviceTime);
}

void kingsmithr1protreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info,
                                                  bool disable_log, bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &kingsmithr1protreadmill::packetReceived, &loop, &QEventLoop::quit);
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

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::Write)
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    else
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") << writeBuffer->toHex(' ') <<
                   QStringLiteral(" // ") << info << " " << gattWriteCharacteristic.properties();
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void kingsmithr1protreadmill::updateDisplay(uint16_t elapsed) {}

void kingsmithr1protreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline) {
    Q_UNUSED(requestIncline)
    uint8_t speed[] = {0xf7, 0xa2, 0x01, 0x19, 0xbc, 0xfd};
    speed[3] = qRound(requestSpeed * 10);
    speed[4] = speed[1] + speed[2] + speed[3];
    writeCharacteristic(speed, sizeof(speed), QStringLiteral("forceSpeed") + QString::number(requestSpeed), false,
                        true);
}

void kingsmithr1protreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void kingsmithr1protreadmill::update() {

    if (!m_control)
        return;

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
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &kingsmithr1protreadmill::debug);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &kingsmithr1protreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        // debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        uint8_t start_pause[] = {0xf7, 0xa2, 0x04, 0x01, 0xa7, 0xfd};

        // updating the treadmill console every second
        if (sec1Update++ >= (1000 / refresh->interval())) {

            sec1Update = 0;
            updateDisplay(elapsed.value());
        } else {
            uint8_t noOpData[] = {0xf7, 0xa2, 0x00, 0x00, 0xa2, 0xfd};
            writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
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
        if (requestUnlock) {
            uint8_t unlock[] = {0xf7, 0xa2, 0x02, 0x01, 0xa5, 0xfd};
            writeCharacteristic(unlock, sizeof(unlock), QStringLiteral("unlocking..."), false, true);
            requestUnlock = false;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {

                lastSpeed = 0.5;
            }
            writeCharacteristic(start_pause, sizeof(start_pause), QStringLiteral("start"), false, true);

            requestStart = -1;
            emit tapeStarted();
            lastStart = QDateTime::currentMSecsSinceEpoch();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

            if (Speed.value() != 0)
                writeCharacteristic(start_pause, sizeof(start_pause), QStringLiteral("pause"), false, true);

            lastStop = QDateTime::currentMSecsSinceEpoch();

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

void kingsmithr1protreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void kingsmithr1protreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                    const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));
    emit packetReceived();

    lastPacket = value;

    if (newValue.length() != 20 || ignoreFirstPackage) {
        ignoreFirstPackage = false;
        emit debug(QStringLiteral("packet ignored"));
        return;

    }

    if (newValue.at(2) == 0x05) {
        emit debug(QStringLiteral("lock byte found, asking to unlock the treadmill"));
        requestUnlock = true;
    }

    double speed = GetSpeedFromPacket(value);
    if (speed > 0) {
        lastStart = 0; // telling to the UI that it could be autostoppable when the speed it will reach again 0

        // sometimes, the treadmill, in the startup phase, send a bugged target speed
        // https://github.com/cagnulein/qdomyos-zwift/issues/470#issuecomment-963025129
        double t = GetTargetSpeedFromPacket(value);
        if (t == lastTargetSpeed) {
            targetSpeed = speed;
            emit debug(QStringLiteral("ignoring target speed"));
        } else {
            // if the user changes the speed from the remote, the treadmill doesn't update the target speed
            // https://github.com/cagnulein/qdomyos-zwift/issues/470#issuecomment-965525224
            if (speed == t) {
                targetSpeedMatchesSpeed = true;
                lastTargetSpeedMatchesSpeed = speed;
                emit debug(QStringLiteral("target speed matches the current speed"));
            }
            if (targetSpeedMatchesSpeed && t != lastTargetSpeedMatchesSpeed) {
                targetSpeedMatchesSpeed = false;
                lastTargetSpeedMatchesSpeed = -1;
                emit debug(QStringLiteral("target speed changes"));
            }

            if (targetSpeedMatchesSpeed == false) {
                targetSpeed = t;
                emit debug(QStringLiteral("using target speed from TM"));
            } else {
                emit debug(QStringLiteral("using current speed as target speed"));
                targetSpeed = speed;
            }

            lastTargetSpeed = -1;
        }
    } else {
        Cadence = 0;
        lastStop = 0;
        targetSpeed = 0;
        lastTargetSpeed = GetTargetSpeedFromPacket(value);
        targetSpeedMatchesSpeed = false;
        lastTargetSpeedMatchesSpeed = -1;
    }

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

    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    // it's not the perfect setting for this, but it's better than nothing...
    if(disable_hr_frommachinery)
        cadenceFromAppleWatch();
    else {
        double sc = GetStepsFromPacket(value);
        StepCount = sc;
        if(lastStepCount < StepCount.value()) {
            double c = (StepCount.value() - lastStepCount) / (lastTimeStepCountChanged.msecsTo(QDateTime::currentDateTime()) / 60000.0);
            if(c < 255)
                cadenceRaw = c;
            Cadence = cadenceRaw.average5s();
            lastTimeStepCountChanged = QDateTime::currentDateTime();
        }
        lastStepCount = sc;
    }    

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current target speed: ") + QString::number(targetSpeed));
    emit debug(QStringLiteral("Current StepCount: ") + QString::number(StepCount.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
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
    Inclination = treadmillInclinationOverride(0);

    if (speed > 0) {

        lastSpeed = speed;
        // lastInclination = incline;
    }

    firstCharacteristicChanged = false;
}

double kingsmithr1protreadmill::GetSpeedFromPacket(const QByteArray &packet) {

    uint8_t convertedData = (uint8_t)packet.at(3);
    double data = (double)convertedData / 10.0f;
    return data;
}

double kingsmithr1protreadmill::GetTargetSpeedFromPacket(const QByteArray &packet) {

    uint8_t convertedData = (uint8_t)packet.at(14);
    double data = (double)convertedData / 10.0f;
    return data;
}

double kingsmithr1protreadmill::GetStepsFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(12) << 8) | ((uint8_t)packet.at(13));
    return (double)convertedData;
}

void kingsmithr1protreadmill::btinit(bool startTape) {
    uint8_t initData1[] = {0xf7, 0xa5, 0x61, 0x01, 0x6a, 0x23, 0x1d, 0xbe, 0x6f, 0xfd};
    uint8_t initData1b[] = {0xf7, 0xa5, 0x61, 0x2b, 0xe9, 0x19, 0xc7, 0xd2, 0xcc, 0xfd};
    uint8_t initData2[] = {0xf7, 0xa2, 0x00, 0x00, 0xa2, 0xfd};
    uint8_t initData3[] = {0xf7, 0xa6, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa6, 0xfd};
    uint8_t initData4[] = {0xf7, 0xb1, 0x05, 0x07, 0x15, 0x16, 0x08, 0x18, 0x08, 0xfd};
    uint8_t initData5[] = {0xf7, 0xb3, 0x02, 0x1d, 0x00, 0xd2, 0xfd};

    if (version == CLASSIC)
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    else
        writeCharacteristic(initData1b, sizeof(initData1b), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
    writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);

    initDone = true;
}

void kingsmithr1protreadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xFE02);
    QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xFE01);
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &kingsmithr1protreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &kingsmithr1protreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &kingsmithr1protreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &kingsmithr1protreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void kingsmithr1protreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void kingsmithr1protreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                    const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void kingsmithr1protreadmill::serviceScanDone(void) {
    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xFE00);
    emit debug(QStringLiteral("serviceScanDone"));

    ignoreFirstPackage = true;

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &kingsmithr1protreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void kingsmithr1protreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("kingsmithr1protreadmill::errorService ") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void kingsmithr1protreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("kingsmithr1protreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void kingsmithr1protreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    if (!device.name().toUpper().compare(QStringLiteral("RE")) ||
        !device.name().toUpper().compare(QStringLiteral("KS-S1")))
        version = RE;
    else
        version = CLASSIC;

    qDebug() << "version" << version;

    {

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &kingsmithr1protreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &kingsmithr1protreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &kingsmithr1protreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &kingsmithr1protreadmill::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
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

void kingsmithr1protreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool kingsmithr1protreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void kingsmithr1protreadmill::searchingStop() { searchStopped = true; }

bool kingsmithr1protreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool kingsmithr1protreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
