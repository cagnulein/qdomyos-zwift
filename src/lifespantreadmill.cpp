#include "lifespantreadmill.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualbike.h"
#include "virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

lifespantreadmill::lifespantreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService,
                                     double forceInitSpeed, double forceInitInclination) {
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
    connect(refresh, &QTimer::timeout, this, &lifespantreadmill::update);
    refresh->start(300ms);
}

void lifespantreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                            bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &lifespantreadmill::packetReceived, &loop, &QEventLoop::quit);
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

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void lifespantreadmill::updateDisplay(uint16_t elapsed) {}

void lifespantreadmill::forceSpeed(double requestSpeed) { Q_UNUSED(requestSpeed) }

void lifespantreadmill::forceIncline(double requestIncline) { Q_UNUSED(requestIncline) }

void lifespantreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void lifespantreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (/*bluetoothDevice.isValid() &&*/
               m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() && gattNotify1Characteristic.isValid() && initDone) {
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
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &lifespantreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &lifespantreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &lifespantreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        uint8_t poll0[] = {0xa1, 0x91, 0x00, 0x00, 0x00};
        uint8_t poll1[] = {0xa1, 0x88, 0x00, 0x00, 0x00};
        uint8_t poll2[] = {0xa1, 0x86, 0x00, 0x00, 0x00};
        uint8_t poll3[] = {0xa1, 0x87, 0x00, 0x00, 0x00};
        uint8_t poll4[] = {0xa1, 0x85, 0x00, 0x00, 0x00};
        uint8_t poll5[] = {0xa1, 0x89, 0x00, 0x00, 0x00};
        uint8_t poll6[] = {0xa1, 0x82, 0x00, 0x00, 0x00};
        switch (counterPoll % 7) {
        case 0:
            writeCharacteristic(poll0, sizeof(poll0), QStringLiteral("poll0"), false, true);
            break;
        case 1:
            writeCharacteristic(poll1, sizeof(poll1), QStringLiteral("poll1"), false, true);
            break;
        case 2:
            writeCharacteristic(poll2, sizeof(poll2), QStringLiteral("poll2"), false, true);
            break;
        case 3:
            writeCharacteristic(poll3, sizeof(poll3), QStringLiteral("poll3"), false, true);
            break;
        case 4:
            writeCharacteristic(poll4, sizeof(poll4), QStringLiteral("poll4"), false, true);
            break;
        case 5:
            writeCharacteristic(poll5, sizeof(poll5), QStringLiteral("poll5"), false, true);
            break;
        case 6:
            writeCharacteristic(poll6, sizeof(poll6), QStringLiteral("poll6"), false, true);
            break;
        }
        counterPoll++;

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
            lastStart = QDateTime::currentMSecsSinceEpoch();
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            requestStop = -1;
            lastStop = QDateTime::currentMSecsSinceEpoch();
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

void lifespantreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double lifespantreadmill::minStepInclination() { return 1.0; }

void lifespantreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    qDebug() << QStringLiteral(" << ") << counterPoll << newValue.toHex(' ');

    lastPacket = newValue;

    if (newValue.length() < 6)
        return;

    switch (counterPoll % 7) {
    case 0:

        break;
    case 1:
        double distance = (double)((((uint16_t)newValue.at(2)) << 8) + ((uint16_t)newValue.at(3)));
        qDebug() << "Current Local Distance" << distance;
        break;
    case 2:

        break;
    case 3:

        break;
    case 4:

        break;
    case 5:

        break;
    case 6:

        break;
    }

    if (((unsigned char)newValue.at(0)) == 0xf0 && ((unsigned char)newValue.at(1)) == 0xd3) {

        writeCharacteristic((uint8_t *)newValue.constData(), newValue.length(), "reply to d3", false, false);

        double miles = 1;
        if (settings.value(QZSettings::sole_treadmill_miles, QZSettings::default_sole_treadmill_miles).toBool())
            miles = 1.60934;

        // this line on iOS sometimes gives strange overflow values
        // uint16_t convertedData = (((uint16_t)newValue.at(3)) << 8) | (uint16_t)newValue.at(4);
        qDebug() << "speed1" << newValue.at(3);
        uint16_t convertedData = (uint8_t)newValue.at(3);
        qDebug() << "speed2" << convertedData;
        convertedData = convertedData << 8;
        qDebug() << "speed3" << convertedData;
        convertedData = convertedData & 0xFF00;
        qDebug() << "speed4" << convertedData;
        convertedData = convertedData + (uint8_t)newValue.at(4);
        qDebug() << "speed5" << convertedData;
        Speed = (((double)convertedData) / 1000.0) * miles;

        if (Speed.value() > 0)
            lastStart = 0;
        else
            lastStop = 0;

        qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
        return;
    } else if (((unsigned char)newValue.at(0)) == 0xf0 && ((unsigned char)newValue.at(1)) == 0xd2) {
        writeCharacteristic((uint8_t *)newValue.constData(), newValue.length(), "reply to d2", false, false);
        Inclination = (uint8_t)newValue.at(3);
        qDebug() << QStringLiteral("Current Inclination: ") + QString::number(Inclination.value());
        return;
    } else if (((unsigned char)newValue.at(0)) == 0xf0 && ((unsigned char)newValue.at(1)) == 0xd0) {
        writeCharacteristic((uint8_t *)newValue.constData(), newValue.length(), "reply to d0", false, false);
        return;
    }

    /*if (newValue.length() != 21)
        return;*/

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

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

    if ((uint8_t)newValue.at(1) == 0xD1 && newValue.length() > 11)
#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {

                uint8_t heart = ((uint8_t)newValue.at(11));
                if (heart == 0) {
                    update_hr_from_external();
                } else {
                    Heart = heart;
                }
            }
        }

    cadenceFromAppleWatch();

    qDebug() << QStringLiteral("Current Heart: ") + QString::number(Heart.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Watt: ") +
                    QString::number(watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

    if (m_control->error() != QLowEnergyController::NoError)
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
}

void lifespantreadmill::btinit() {
    uint8_t initData0[] = {0x02, 0x00, 0x00, 0x00, 0x00};

    writeCharacteristic(initData0, sizeof(initData0), QStringLiteral("init"), false, true);

    initDone = true;
}

void lifespantreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &lifespantreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &lifespantreadmill::characteristicWritten);
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &lifespantreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void lifespantreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void lifespantreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void lifespantreadmill::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &lifespantreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void lifespantreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("lifespantreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void lifespantreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("lifespantreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void lifespantreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &lifespantreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &lifespantreadmill::serviceScanDone);
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(error(QLowEnergyController::Error)));
        connect(m_control, &QLowEnergyController::stateChanged, this, &lifespantreadmill::controllerStateChanged);

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

void lifespantreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool lifespantreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool lifespantreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool lifespantreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
