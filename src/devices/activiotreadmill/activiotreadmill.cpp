#include "activiotreadmill.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

activiotreadmill::activiotreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                   double forceInitInclination) {
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
    connect(refresh, &QTimer::timeout, this, &activiotreadmill::update);
    refresh->start(pollDeviceTime);
}

void activiotreadmill::writeCharacteristic(const QLowEnergyCharacteristic characteristic, uint8_t *data,
                                           uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &activiotreadmill::packetReceived, &loop, &QEventLoop::quit);
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

    gattCommunicationChannelService->writeCharacteristic(characteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void activiotreadmill::forceSpeed(double requestSpeed) {
    QSettings settings;
    uint8_t writeSpeed[] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00};

    writeSpeed[1] = (requestSpeed * 10);
    writeSpeed[5] += writeSpeed[1];
    if (!settings.value(QZSettings::fitfiu_mc_v460, QZSettings::default_fitfiu_mc_v460).toBool() &&
        !settings.value(QZSettings::zero_zt2500_treadmill, QZSettings::default_zero_zt2500_treadmill).toBool())
        writeSpeed[6] = writeSpeed[1] + 1;
    else {
        switch (writeSpeed[1] & 0x0F) {
        case 0x00:
            writeSpeed[6] = writeSpeed[1] + 5;
            break;
        case 0x01:
            writeSpeed[6] = writeSpeed[1] + 3;
            break;
        case 0x02:
            writeSpeed[6] = writeSpeed[1] + 1;
            break;
        case 0x03:
            writeSpeed[6] = writeSpeed[1] - 1;
            break;
        case 0x04:
            writeSpeed[6] = writeSpeed[1] + 5;
            break;
        case 0x05:
            writeSpeed[6] = writeSpeed[1] + 3;
            break;
        case 0x06:
            writeSpeed[6] = writeSpeed[1] + 1;
            break;
        case 0x07:
            writeSpeed[6] = writeSpeed[1] - 1;
            break;
        case 0x08:
            writeSpeed[6] = writeSpeed[1] + 5;
            break;
        case 0x09:
            writeSpeed[6] = writeSpeed[1] + 3;
            break;
        case 0x0A:
            writeSpeed[6] = writeSpeed[1] + 1;
            break;
        case 0x0B:
            writeSpeed[6] = writeSpeed[1] - 1;
            break;
        case 0x0C:
            writeSpeed[6] = writeSpeed[1] + 5;
            break;
        case 0x0D:
            writeSpeed[6] = writeSpeed[1] + 3;
            break;
        case 0x0E:
            writeSpeed[6] = writeSpeed[1] + 1;
            break;
        case 0x0F:
            writeSpeed[6] = writeSpeed[1] - 1;
            break;
        }
    }

    writeCharacteristic(gattWriteCharacteristic, writeSpeed, sizeof(writeSpeed),
                        QStringLiteral("forceSpeed speed=") + QString::number(requestSpeed), false, false);
}

void activiotreadmill::forceIncline(double requestIncline) {
    uint8_t writeIncline[] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x29, 0x06};

    writeIncline[2] = requestIncline;
    writeIncline[5] += requestIncline;
    writeIncline[6] += requestIncline;

    writeCharacteristic(gattWriteCharacteristic, writeIncline, sizeof(writeIncline),
                        QStringLiteral("forceIncline incline=") + QString::number(requestIncline), false, false);
}

void activiotreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void activiotreadmill::update() {
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
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &activiotreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &activiotreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &activiotreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        // debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        {
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

                // should be:
                // 0x49 = inited
                // 0x8a = tape stopped after a pause
                /*if (lastState == 0x49)*/ {
                    uint8_t initData2[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x26, 0x03};

                    writeCharacteristic(gattWriteCharacteristic, initData2, sizeof(initData2), QStringLiteral("start"),
                                        false, true);
                } /*else {
                    uint8_t pause[] = {0x05, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x07};

                    writeCharacteristic(gattWriteCharacteristic, pause, sizeof(pause), QStringLiteral("pause"), false,
                                        true);
                }*/

                requestStart = -1;
                emit tapeStarted();
            }
            if (requestStop != -1) {
                emit debug(QStringLiteral("stopping... ") + (paused ? QStringLiteral("true") : QStringLiteral("false")));
                if (lastState == PAUSED) {
                    uint8_t pause[] = {0x05, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x07};

                    writeCharacteristic(gattWriteCharacteristic, pause, sizeof(pause), QStringLiteral("pause"), false,
                                        true);

                } else {
                    uint8_t stop[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x27, 0x04};

                    writeCharacteristic(gattWriteCharacteristic, stop, sizeof(stop), QStringLiteral("stop"), false,
                                        true);
                }

                requestStop = -1;
            }
            /*if (requestFanSpeed != -1) {
                emit debug(QStringLiteral("changing fan speed..."));

                sendChangeFanSpeed(requestFanSpeed);
                requestFanSpeed = -1;
            }
            if (requestIncreaseFan != -1) {
                emit debug(QStringLiteral("increasing fan speed..."));

                sendChangeFanSpeed(FanSpeed + 1);
                requestIncreaseFan = -1;
            } else if (requestDecreaseFan != -1) {
                emit debug(QStringLiteral("decreasing fan speed..."));

                sendChangeFanSpeed(FanSpeed - 1);
                requestDecreaseFan = -1;
            }*/
        }
    }
}

void activiotreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void activiotreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;
    QDateTime now = QDateTime::currentDateTime();

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));
    emit packetReceived();

    if (newValue.length() < 12)
        return;

    lastPacket = value;
    // lastState = value.at(0);

    double speed = GetSpeedFromPacket(value);
    double incline = 1.0; // "fitfiu_mc_v460" has 1.0 fixed inclination
    if (!settings.value(QZSettings::fitfiu_mc_v460, QZSettings::default_fitfiu_mc_v460).toBool())
        incline = GetInclinationFromPacket(value);
        // double kcal = GetKcalFromPacket(value);
        // double distance = GetDistanceFromPacket(value);

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
            } else

                Heart = heart;
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
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        Distance += ((speed / (double)3600.0) /
                     ((double)1000.0 / (double)(lastTimeCharacteristicChanged.msecsTo(now))));
        lastTimeCharacteristicChanged = now;
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    // emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    // emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    if (Speed.value() != speed) {

        emit speedChanged(speed);
    }
    Speed = speed;
    if (Inclination.value() != incline) {

        emit inclinationChanged(0, incline);
    }
    Inclination = incline;

    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

    if (speed > 0) {

        lastSpeed = speed;
        lastInclination = incline;
    }

    firstCharacteristicChanged = false;
}

double activiotreadmill::GetSpeedFromPacket(const QByteArray &packet) {

    uint8_t convertedData = (uint8_t)packet.at(1);
    convertedData = convertedData - 0x40;
    uint8_t lownibble = convertedData & 0x0F;
    uint8_t highnibble = convertedData & 0xF0;
    highnibble = ((highnibble >> 4) & 0x0F);
    switch (lownibble) {
    case 0x09:
        lownibble = 0x00;
        break;
    case 0x0A:
        lownibble = 0x01;
        break;
    case 0x0F:
        lownibble = 0x02;
        break;
    case 0x00:
        highnibble--;
        lownibble = 0x03;
        break;
    case 0x0D:
        lownibble = 0x04;
        break;
    case 0x0E:
        lownibble = 0x05;
        break;
    case 0x03:
        highnibble--;
        lownibble = 0x06;
        break;
    case 0x04:
        highnibble--;
        lownibble = 0x07;
        break;
    case 0x01:
        highnibble--;
        lownibble = 0x08;
        break;
    case 0x02:
        highnibble--;
        lownibble = 0x09;
        break;
    case 0x07:
        highnibble--;
        lownibble = 0x0A;
        break;
    case 0x08:
        highnibble--;
        lownibble = 0x0B;
        break;
    case 0x05:
        highnibble--;
        lownibble = 0x0C;
        break;
    case 0x06:
        highnibble--;
        lownibble = 0x0D;
        break;
    case 0x0B:
        highnibble--;
        lownibble = 0x0E;
        break;
    case 0x0C:
        highnibble--;
        lownibble = 0x0F;
        break;
    }

    convertedData = (highnibble << 4) + lownibble;

    double data = ((double)(convertedData) / 10.0f);
    return data;
}

double activiotreadmill::GetInclinationFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (uint8_t)packet.at(9);

    switch (convertedData) {
    case 0x49:
        convertedData = 0x00;
        break;
    case 0x4A:
        convertedData = 0x01;
        break;
    case 0x4F:
        convertedData = 0x02;
        break;
    case 0x50:
        convertedData = 0x03;
        break;
    case 0x4D:
        convertedData = 0x04;
        break;
    case 0x4E:
        convertedData = 0x05;
        break;
    case 0x53:
        convertedData = 0x06;
        break;
    case 0x54:
        convertedData = 0x07;
        break;
    case 0x51:
        convertedData = 0x08;
        break;
    case 0x52:
        convertedData = 0x09;
        break;
    case 0x57:
        convertedData = 0x0A;
        break;
    case 0x58:
        convertedData = 0x0B;
        break;
    case 0x55:
        convertedData = 0x0C;
        break;
    case 0x56:
        convertedData = 0x0D;
        break;
    case 0x5B:
        convertedData = 0x0E;
        break;
    case 0x5C:
        convertedData = 0x0F;
        break;
    }

    double data = (double)(convertedData);
    return data;
}

void activiotreadmill::btinit(bool startTape) {
    uint8_t initData1[] = {0x88};
    uint8_t initData2[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x26, 0x03};

    writeCharacteristic(gattWrite2Characteristic, initData1, sizeof(initData1), QStringLiteral("init"), false, false);

    // starts the tape
    // writeCharacteristic(gattWriteCharacteristic, initData2, sizeof(initData2), QStringLiteral("init"), false, true);

    if (startTape) {
    }

    initDone = true;
}

double activiotreadmill::minStepInclination() { return 1.0; }

void activiotreadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("e54eaa57-371b-476c-99a3-74d267e3edae"));
    QBluetoothUuid _gattWrite2CharacteristicId(QStringLiteral("e54eaa55-371b-476c-99a3-74d267e3edae"));
    QBluetoothUuid _gattNotifyCharacteristicId(QStringLiteral("e54eaa56-371b-476c-99a3-74d267e3edae"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {

        // qDebug() << gattCommunicationChannelService->characteristics();
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid()
                     << c.properties();
        }

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattWrite2Characteristic = gattCommunicationChannelService->characteristic(_gattWrite2CharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattWrite2Characteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &activiotreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &activiotreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &activiotreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &activiotreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void activiotreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void activiotreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void activiotreadmill::serviceScanDone(void) {
    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("e54eaa50-371b-476c-99a3-74d267e3edae"));
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    emit debug("Services found:");
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        emit debug(s.toString());
    }

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (gattCommunicationChannelService) {
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
                &activiotreadmill::stateChanged);
        gattCommunicationChannelService->discoverDetails();
    } else {
        emit debug(QStringLiteral("error on find Service"));
    }
}

void activiotreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("activiotreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void activiotreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("activiotreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void activiotreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &activiotreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &activiotreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &activiotreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &activiotreadmill::controllerStateChanged);

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

void activiotreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool activiotreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void activiotreadmill::searchingStop() { searchStopped = true; }
