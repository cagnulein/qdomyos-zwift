#include "speraxtreadmill.h"
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

speraxtreadmill::speraxtreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                   double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT);
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
    connect(refresh, &QTimer::timeout, this, &speraxtreadmill::update);
    refresh->start(pollDeviceTime);
}

void speraxtreadmill::writeCharacteristic(const QLowEnergyCharacteristic characteristic, uint8_t *data,
                                           uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &speraxtreadmill::packetReceived, &loop, &QEventLoop::quit);
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

void speraxtreadmill::forceSpeed(double requestSpeed) {
    /*QSettings settings;
    uint8_t writeSpeed[] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00};

    writeSpeed[1] = (requestSpeed * 10);
    writeSpeed[5] += writeSpeed[1];

    writeCharacteristic(gattWriteCharacteristic, writeSpeed, sizeof(writeSpeed),
                        QStringLiteral("forceSpeed speed=") + QString::number(requestSpeed), false, false);*/
}

void speraxtreadmill::forceIncline(double requestIncline) {
    /*uint8_t writeIncline[] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x29, 0x06};

    writeIncline[2] = requestIncline;
    writeIncline[5] += requestIncline;
    writeIncline[6] += requestIncline;

    writeCharacteristic(gattWriteCharacteristic, writeIncline, sizeof(writeIncline),
                        QStringLiteral("forceIncline incline=") + QString::number(requestIncline), false, false);*/
}

void speraxtreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void speraxtreadmill::update() {
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
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &speraxtreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &speraxtreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &speraxtreadmill::changeInclinationRequested);
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
            else if (requestInclination != -100) {
                if (requestInclination < 0)
                    requestInclination = 0;
                if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                    requestInclination <= 15) {
                    emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                    forceIncline(requestInclination);
                }
                requestInclination = -100;
            }
            else if (requestStart != -1) {
                emit debug(QStringLiteral("starting..."));
                if (lastSpeed == 0.0) {

                    lastSpeed = 0.5;
                }

                requestStart = -1;
                emit tapeStarted();
            }
            else if (requestStop != -1) {
                emit debug(QStringLiteral("stopping... ") + (paused ? QStringLiteral("true") : QStringLiteral("false")));

                requestStop = -1;
            } else {
                uint8_t noop[] = {0xf5, 0x08, 0x00, 0x19, 0xf0, 0x0a, 0x59, 0xfa};

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

void speraxtreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void speraxtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
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

    if (newValue.length() < 24)
        return;

    lastPacket = value;
    // lastState = value.at(0);

    double speed = GetSpeedFromPacket(value);
    double incline = 0;

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

double speraxtreadmill::GetSpeedFromPacket(const QByteArray &packet) {

    uint8_t convertedData = (uint8_t)packet.at(17);
    double data = ((double)(convertedData) / 10.0);
    return data;
}

double speraxtreadmill::GetInclinationFromPacket(const QByteArray &packet) {

    return 0;
}

void speraxtreadmill::btinit(bool startTape) {
    uint8_t initData1[] = {0xf5, 0x07, 0x00, 0x01, 0x26, 0xd8, 0xfa};
    uint8_t initData2[] = {0xf5, 0x09, 0x00, 0x13, 0x01, 0x00, 0x89, 0xb8, 0xfa};

    writeCharacteristic(gattWriteCharacteristic, initData1, sizeof(initData1), QStringLiteral("init"), false, false);
    writeCharacteristic(gattWriteCharacteristic, initData1, sizeof(initData1), QStringLiteral("init"), false, false);
    writeCharacteristic(gattWriteCharacteristic, initData2, sizeof(initData2), QStringLiteral("init"), false, false);


    if (startTape) {
    }

    initDone = true;
}

double speraxtreadmill::minStepInclination() { return 1.0; }

void speraxtreadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xFFF2);
    QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xFFF1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {

        // qDebug() << gattCommunicationChannelService->characteristics();
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : std::as_const(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid() 
                     << c.properties();
        }

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &speraxtreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &speraxtreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &speraxtreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &speraxtreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void speraxtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void speraxtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void speraxtreadmill::serviceScanDone(void) {
    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    emit debug("Services found:");
    for (const QBluetoothUuid &s : std::as_const(services_list)) {
        emit debug(s.toString());
    }

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (gattCommunicationChannelService) {
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
                &speraxtreadmill::stateChanged);
        gattCommunicationChannelService->discoverDetails();
    } else {
        emit debug(QStringLiteral("error on find Service"));
    }
}

void speraxtreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("speraxtreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void speraxtreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("speraxtreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void speraxtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &speraxtreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &speraxtreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &speraxtreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &speraxtreadmill::controllerStateChanged);

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

void speraxtreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool speraxtreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void speraxtreadmill::searchingStop() { searchStopped = true; }