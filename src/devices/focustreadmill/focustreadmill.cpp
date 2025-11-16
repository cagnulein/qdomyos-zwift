#include "focustreadmill.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

focustreadmill::focustreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                 double forceInitInclination) {

#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = false;
#endif

    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0)
        lastSpeed = forceInitSpeed;

    if (forceInitInclination > 0)
        lastInclination = forceInitInclination;

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &focustreadmill::update);
    refresh->start(500ms);
}

void focustreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &focustreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(400ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(400ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

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

void focustreadmill::updateDisplay(uint16_t elapsed) {
    Q_UNUSED(elapsed);
    uint8_t noOpData[] = {0xf0, 0xc3, 0x03, 0x00, 0x00, 0x00, 0xb6};

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"));
}

void focustreadmill::forceIncline(double requestIncline) {
    forceSpeedAndInclination(Speed.value(), requestIncline);
}

double focustreadmill::minStepInclination() { return 1.0; }
double focustreadmill::minStepSpeed() { return 0.1; }

void focustreadmill::forceSpeedAndInclination(double speed, double inclination) {
    uint8_t s[] = {0xf0, 0xc3, 0x03, 0x03, 0x1e, 0x00, 0xd7};
    s[4] = (uint8_t)(speed * 10.0);
    s[5] = (uint8_t)(inclination);
    s[6] = ((uint8_t)0xb9) + s[4] + s[5];
    writeCharacteristic(s, sizeof(s), QStringLiteral("forceSpeedAndInclination ") + QString::number(speed) + " "  + QString::number(inclination));
}

void focustreadmill::forceSpeed(double requestSpeed) {
    forceSpeedAndInclination(requestSpeed, Inclination.value());
}

void focustreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void focustreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    qDebug() << m_control->state() << bluetoothDevice.isValid() << gattCommunicationChannelService
             << gattWriteCharacteristic.isValid() << initDone << requestSpeed << requestInclination;

    if (initRequest) {
        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() && initDone) {
        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &focustreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &focustreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
                QThread::msleep(400);
                forceIncline(Inclination.value());
            }
            requestSpeed = -1;
        } else if (requestInclination != -100) {
            if (requestInclination < 0)
                requestInclination = 0;
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                forceSpeed(Speed.value());
                QThread::msleep(400);
                forceIncline(requestInclination);
            }
            requestInclination = -100;
        } else if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            lastStart = QDateTime::currentMSecsSinceEpoch();
            uint8_t start[] = {0xf0, 0xc3, 0x03, 0x01, 0x00, 0x00, 0xb7};
            writeCharacteristic(start, sizeof(start), "start", false, true);
            if (lastSpeed == 0.0) {
                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();
        } else if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            uint8_t stop[] = {0xf0, 0xc3, 0x03, 0x02, 0x00, 0x00, 0xb8};
            writeCharacteristic(stop, sizeof(stop), "stop", false, true);
            uint8_t stop2[] = {0xf0, 0xc5, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf};
            writeCharacteristic(stop2, sizeof(stop2), "stop2", false, true);
            requestStop = -1;
            lastStop = QDateTime::currentMSecsSinceEpoch();
        } else if (sec1Update++ >= (400 / refresh->interval())) {
            updateDisplay(elapsed.value());
            sec1Update = 0;
        }
    }
}

void focustreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void focustreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    emit packetReceived();

    if ((newValue.length() != 20))
        return;

    double speed = GetSpeedFromPacket(value);
    double incline = GetInclinationFromPacket(value);

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        /*if(heartRateBeltName.startsWith("Disabled"))
        Heart = value.at(18);*/
    }

    cadenceFromAppleWatch();

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current incline: ") + QString::number(incline));

    if (Speed.value() != speed) {
        emit speedChanged(speed);
    }
    Speed = speed;
    if (Inclination.value() != incline) {
        emit inclinationChanged(0.0, incline);
    }
    Inclination = incline;

    if (speed > 0) {
        lastSpeed = speed;
        lastInclination = incline;
    }

    // this treadmill has a bug that always send 1km/h even if the tape is stopped
    if (speed > 1.0) {
        lastStart = 0;
    } else {
        lastStop = 0;
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

    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
}

double focustreadmill::GetSpeedFromPacket(const QByteArray &packet) {
    double convertedData = ((double)((uint8_t)packet.at(10))) / 10.0;

    return convertedData;
}

double focustreadmill::GetInclinationFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(11);
    double data = convertedData;

    return data;
}

void focustreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)

    uint8_t initData1[] = {0xf0, 0xc0, 0x00, 0xb0};
    uint8_t initData2[] = {0xf0, 0xc1, 0x02, 0x00, 0x0, 0x0b3};
    uint8_t initData3[] = {0xf0, 0xc3, 0x03, 0x00, 0x00, 0x00, 0xb6};
    uint8_t initData4[] = {0xf0, 0xc5, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf};
    uint8_t initData5[] = {0xf0, 0xc4, 0x05, 0x00, 0x3a, 0x01, 0xaf, 0x4d, 0xf0};
    uint8_t initData6[] = {0xf0, 0xc6, 0x01, 0x01, 0xb8};
    uint8_t initData7[] = {0xf0, 0xc4, 0x05, 0x00, 0x3a, 0x01, 0xaf, 0x4d, 0xf0};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
    writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
    writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
    writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);

    initDone = true;
}

void focustreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
        QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &focustreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &focustreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &focustreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &focustreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void focustreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void focustreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void focustreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &focustreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void focustreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("focustreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void focustreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("focustreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void focustreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &focustreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &focustreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &focustreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &focustreadmill::controllerStateChanged);

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

void focustreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool focustreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}
/*
bool focustreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool focustreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}*/


void focustreadmill::searchingStop() { searchStopped = true; }
