#include "echelonstride.h"
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

echelonstride::echelonstride(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
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
    connect(refresh, &QTimer::timeout, this, &echelonstride::update);
    refresh->start(pollDeviceTime);
}

void echelonstride::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                        bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &echelonstride::packetReceived, &loop, &QEventLoop::quit);
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
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void echelonstride::updateDisplay(uint16_t elapsed) {}

void echelonstride::forceSpeed(double requestSpeed) {
    uint8_t noOpData[] = {0xf0, 0xb2, 0x02, 0x00, 0x00, 0x00};

    noOpData[3] = (uint8_t)(((uint16_t)(requestSpeed * 1000.0)) >> 8);
    noOpData[4] = ((uint8_t)(requestSpeed * 1000.0));

    for (uint8_t i = 0; i < sizeof(noOpData) - 1; i++) {
        noOpData[5] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("force speed"), false, true);
}

void echelonstride::forceIncline(double requestIncline) {
    uint8_t noOpData[] = {0xf0, 0xb1, 0x01, 0x00, 0x00};

    noOpData[3] = requestIncline;

    for (uint8_t i = 0; i < sizeof(noOpData) - 1; i++) {
        noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("force incline"), false, true);
}

void echelonstride::sendPoll() {
    uint8_t noOpData[] = {0xf0, 0xa0, 0x01, 0x00, 0x00};

    noOpData[3] = counterPoll;

    for (uint8_t i = 0; i < sizeof(noOpData) - 1; i++) {
        noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);

    counterPoll++;
    if (!counterPoll) {
        counterPoll = 1;
    }
}

void echelonstride::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void echelonstride::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (/*bluetoothDevice.isValid() &&*/
               m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() && gattNotify1Characteristic.isValid() &&
               gattNotify2Characteristic.isValid() && initDone) {
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
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &echelonstride::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &echelonstride::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &echelonstride::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ >= (2000 / refresh->interval())) {
            sec1Update = 0;
            sendPoll();
        }

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
            uint8_t initData3[] = {0xf0, 0xb0, 0x01, 0x01, 0xa2};
            writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("start"), false, true);

            if(stride4) {
                uint8_t initData0[] = {0xf0, 0xa5, 0x00, 0x95};
                writeCharacteristic(initData0, sizeof(initData0), QStringLiteral("start"), false, false);
            }

            uint8_t initData4[] = {0xf0, 0xd0, 0x01, 0x00, 0xc1};
            writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("start"), false, false);

            uint8_t initData5[] = {0xf0, 0xd0, 0x01, 0x11, 0xd2};
            writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("start"), false, false);

            if(stride4) {
                uint8_t initData0[] = {0xf0, 0xd3, 0x02, 0x01, 0xf4, 0xba};
                writeCharacteristic(initData0, sizeof(initData0), QStringLiteral("start"), false, false);
            }

            lastStart = QDateTime::currentMSecsSinceEpoch();
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            uint8_t initData3[] = {0xf0, 0xb0, 0x01, 0x00, 0xa1};
            writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("stop"), false, true);
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

void echelonstride::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double echelonstride::minStepInclination() { return 1.0; }

void echelonstride::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    qDebug() << QStringLiteral(" << ") + newValue.toHex(' ');

    lastPacket = newValue;

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
    } else if (((unsigned char)newValue.at(0)) == 0xf0 && ((unsigned char)newValue.at(1)) == 0xd1 && stride4) {

        double miles = 1;
        if (settings.value(QZSettings::sole_treadmill_miles, QZSettings::default_sole_treadmill_miles).toBool())
            miles = 1.60934;

        // this line on iOS sometimes gives strange overflow values
        // uint16_t convertedData = (((uint16_t)newValue.at(3)) << 8) | (uint16_t)newValue.at(4);
        qDebug() << "speed1" << newValue.at(7);
        uint16_t convertedData = (uint8_t)newValue.at(7);
        qDebug() << "speed2" << convertedData;
        convertedData = convertedData << 8;
        qDebug() << "speed3" << convertedData;
        convertedData = convertedData & 0xFF00;
        qDebug() << "speed4" << convertedData;
        convertedData = convertedData + (uint8_t)newValue.at(8);
        qDebug() << "speed5" << convertedData;
        Speed = (((double)convertedData) / 100.0) * miles;

        if (Speed.value() > 0)
            lastStart = 0;
        else
            lastStop = 0;

        qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
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
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        Distance += ((Speed.value() / 3600.0) /
                     (1000.0 / (lastTimeCharacteristicChanged.msecsTo(now))));
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

    lastTimeCharacteristicChanged = now;
    firstCharacteristicChanged = false;
}

void echelonstride::btinit() {
    uint8_t initData0[] = {0xf0, 0xa4, 0x00, 0x94};
    uint8_t initData1[] = {0xf0, 0xa1, 0x00, 0x91};
    uint8_t initData2[] = {0xf0, 0xa3, 0x00, 0x93};

    // stride4
    uint8_t initDataStride4_0[] = {0xf0, 0xe0, 0xfd, 0x3e, 0x65, 0x48, 0xd5, 0x8d};

    if(stride4) {
        writeCharacteristic(initData0, sizeof(initData0), QStringLiteral("init"), false, true); // send a frame to wait the Value: f0e0728518586198
        writeCharacteristic(initData0, sizeof(initData0), QStringLiteral("init"), false, false);
        writeCharacteristic(initDataStride4_0, sizeof(initDataStride4_0), QStringLiteral("init"), false, false);
        writeCharacteristic(initData0, sizeof(initData0), QStringLiteral("init"), false, false);
    } else {
        writeCharacteristic(initData0, sizeof(initData0), QStringLiteral("init"), false, true);
    }

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);

    if(!stride4)
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

    initDone = true;
}

void echelonstride::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("0bf669f2-45f2-11e7-9598-0800200c9a66"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("0bf669f3-45f2-11e7-9598-0800200c9a66"));
    QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &echelonstride::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &echelonstride::characteristicWritten);
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &echelonstride::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify2Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void echelonstride::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void echelonstride::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void echelonstride::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("0bf669f1-45f2-11e7-9598-0800200c9a66"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &echelonstride::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void echelonstride::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("echelonstride::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void echelonstride::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("echelonstride::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void echelonstride::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {
        bluetoothDevice = device;

        if(bluetoothDevice.name().toUpper().startsWith("STRIDE4")) {
            stride4 = true;
            qDebug() << "STRIDE4 workaround enabled!";
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &echelonstride::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &echelonstride::serviceScanDone);
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(error(QLowEnergyController::Error)));
        connect(m_control, &QLowEnergyController::stateChanged, this, &echelonstride::controllerStateChanged);

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

void echelonstride::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool echelonstride::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool echelonstride::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool echelonstride::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
