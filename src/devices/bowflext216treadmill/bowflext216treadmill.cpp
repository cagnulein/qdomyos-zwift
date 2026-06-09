#include "bowflext216treadmill.h"
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

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

bowflext216treadmill::bowflext216treadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService,
                                           double forceInitSpeed, double forceInitInclination) {

#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
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
    connect(refresh, &QTimer::timeout, this, &bowflext216treadmill::update);
    refresh->start(500ms);
}

void bowflext216treadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                               bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &bowflext216treadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        // connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop,
        // &QEventLoop::quit); timeout.singleShot(300ms, &loop, &QEventLoop::quit);
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

void bowflext216treadmill::updateDisplay(uint16_t elapsed) {}

void bowflext216treadmill::forceIncline(double requestIncline) {}

double bowflext216treadmill::minStepInclination() { return 1.0; }

void bowflext216treadmill::forceSpeed(double requestSpeed) {}

void bowflext216treadmill::update() {

    if (!m_control)
        return;

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
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &bowflext216treadmill::debug);
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

        if (requestStart != -1) {
            uint8_t start[] = {0x07, 0x06, 0xcf, 0x00, 0x1f, 0x05, 0x00};
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {
                lastSpeed = 0.5;
            }
            writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1 || requestPause != -1) {
            uint8_t stop[] = {0x0a, 0x08, 0x7a, 0x00, 0x19, 0x28, 0x01, 0x32, 0x00, 0x00};
            emit debug(QStringLiteral("stopping..."));
            writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
            requestStop = -1;
            requestPause = -1;
        }
    }
}

void bowflext216treadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void bowflext216treadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' ') + characteristic.uuid().toString());

    emit packetReceived();

    if (characteristic.uuid() != gattNotify3Characteristic.uuid() && !bowflex_btx116 && !bowflex_t8j) {
        if (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()) > 5000) {
            Speed = 0;
            qDebug() << QStringLiteral("resetting speed since i'm not receiving metrics in the last 5 seconds");
            emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
        }

        return;
    }

    if ((newValue.length() != 20) && bowflex_t8j == false)
        return;
    else if ((newValue.length() != 12) && bowflex_t8j == true)
        return;

    if(bowflex_T128 && characteristic.uuid() != QBluetoothUuid(QStringLiteral("a46a4a80-9803-11e3-8f3c-0002a5d5c51b"))) {
        double incline = GetInclinationFromPacket(value);
        qDebug() << QStringLiteral("Current incline: ") << incline;
        if (Inclination.value() != incline) {
            emit inclinationChanged(0.0, incline);
        }
        Inclination = incline;
        return;
    }

    if ((bowflex_t6 == true || bowflex_T128 == true) && characteristic.uuid() != QBluetoothUuid(QStringLiteral("a46a4a80-9803-11e3-8f3c-0002a5d5c51b")))
        return;

    double speed = GetSpeedFromPacket(value);    
    double incline = GetInclinationFromPacket(value);
    // double kcal = GetKcalFromPacket(value);
    // double distance = GetDistanceFromPacket(value);

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QLatin1String("Disabled"))) {
            update_hr_from_external();
        }
    }
    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    // emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
    // emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    // debug("Current Distance: " + QString::number(distance));

    if (Speed.value() != speed) {
        emit speedChanged(speed);
    }
    Speed = speed;

    if(bowflex_T128 == false) {
        if (Inclination.value() != incline) {
            emit inclinationChanged(0.0, incline);
        }
        Inclination = incline;
    }

    // KCal = kcal;
    // Distance = distance;

    if (speed > 0) {
        lastSpeed = speed;
        lastInclination = incline;
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

double bowflext216treadmill::GetSpeedFromPacket(const QByteArray &packet) {
    if (bowflex_t8j) {
        uint16_t convertedData = (uint16_t)((uint8_t)packet.at(3)) + ((uint16_t)((uint8_t)packet.at(4)) << 8);
        double data = (double)convertedData / 100.0f;
        return data * 1.60934;
    } else if(bowflex_T128) {
        uint16_t convertedData = (uint16_t)((uint8_t)packet.at(9)) + ((uint16_t)((uint8_t)packet.at(10)) << 8);
        double data = (double)convertedData / 100.0f;
        return data * 1.60934;
    } else if (bowflex_t6 == false) {
        uint16_t convertedData = (uint16_t)((uint8_t)packet.at(6)) + (((uint16_t)((uint8_t)packet.at(7)) << 8) & 0xFF00);
        double data = (double)convertedData / 100.0f;
        return data * 1.60934;
    } else {
        uint16_t convertedData = (uint16_t)((uint8_t)packet.at(9)) + (((uint16_t)((uint8_t)packet.at(10)) << 8) & 0xFF00);
        double data = (double)convertedData / 100.0f;
        return data * 1.60934;
    }
}

double bowflext216treadmill::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return (double)convertedData;
}

double bowflext216treadmill::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

double bowflext216treadmill::GetInclinationFromPacket(const QByteArray &packet) {
    if (bowflex_T128) {
        uint16_t convertedData = packet.at(7);
        double data = convertedData;

        return data;
    }
    if (bowflex_t8j) {
        uint16_t convertedData = packet.at(6);
        double data = convertedData;

        return data;
    } else if (bowflex_t6 == false) {
        uint16_t convertedData = packet.at(4);
        double data = convertedData;

        return data;
    } else {
        return 0;
    }
}

void bowflext216treadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)
    uint8_t initData1[] = {0x05, 0x01, 0xe3, 0x00, 0x17};
    uint8_t initData2[] = {0x05, 0x02, 0xe4, 0x00, 0x15};
    uint8_t initData3[] = {0x05, 0x03, 0xe1, 0x00, 0x17};
    uint8_t initData4[] = {0x07, 0x04, 0xd1, 0x00, 0x1f, 0x05, 0x00};
    uint8_t initData5[] = {0x05, 0x05, 0xdf, 0x00, 0x17};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
    writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);

    initDone = true;
}

void bowflext216treadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("1717b3c0-9803-11e3-90e1-0002a5d5c51b"));
        QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("35ddd0a0-9803-11e3-9a8b-0002a5d5c51b"));
        QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("6be8f580-9803-11e3-ab03-0002a5d5c51b"));
        QBluetoothUuid _gattNotify3CharacteristicId(QStringLiteral("a46a4a80-9803-11e3-8f3c-0002a5d5c51b"));
        QBluetoothUuid _gattNotify4CharacteristicId(QStringLiteral("b8066ec0-9803-11e3-8346-0002a5d5c51b"));
        QBluetoothUuid _gattNotify5CharacteristicId(QStringLiteral("d57cda20-9803-11e3-8426-0002a5d5c51b"));

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        gattNotify3Characteristic = gattCommunicationChannelService->characteristic(_gattNotify3CharacteristicId);
        gattNotify4Characteristic = gattCommunicationChannelService->characteristic(_gattNotify4CharacteristicId);
        gattNotify5Characteristic = gattCommunicationChannelService->characteristic(_gattNotify5CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());
        Q_ASSERT(gattNotify3Characteristic.isValid());
        Q_ASSERT(gattNotify4Characteristic.isValid());
        Q_ASSERT(gattNotify5Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &bowflext216treadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &bowflext216treadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &bowflext216treadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &bowflext216treadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify2Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify3Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify4Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify5Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void bowflext216treadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void bowflext216treadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void bowflext216treadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("edff9e80-cad7-11e5-ab63-0002a5d5c51b"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (gattCommunicationChannelService == nullptr) {
        QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("B1216C2E-464E-4C46-B2B4-0B5C8EB23DAE"));
        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
        if (gattCommunicationChannelService == nullptr) {
            qDebug() << "trying with the BOWFLEX T6 treadmill";
            bowflex_t6 = true;
            QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("15B7BF49-1693-481E-B877-69D33CE6BAFA"));
            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
            if (gattCommunicationChannelService == nullptr) {
                qDebug() << "trying with the BOWFLEX BTX116 treadmill";
                bowflex_t6 = false;
                bowflex_btx116 = true;
                QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("b5c78780-cad7-11e5-b9f8-0002a5d5c51b"));
                gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
                if (gattCommunicationChannelService == nullptr) {
                    qDebug() << "trying with the BOWFLEX T8J treadmill";
                    bowflex_btx116 = false;
                    bowflex_t8j = true;
                    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("ebbe6870-d00a-4bf4-bb79-63b35bf4c6b5"));
                    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
                    if (gattCommunicationChannelService == nullptr) {
                        qDebug() << "WRONG SERVICE";
                        return;
                    }
                }
            }
        }
    }
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &bowflext216treadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void bowflext216treadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("bowflext216treadmill::errorService ") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void bowflext216treadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("bowflext216treadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void bowflext216treadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        if(bluetoothDevice.name().toUpper().startsWith("BOWFLEX T128")) {
            qDebug() << "BOWFLEX T128 workaround found!";
            bowflex_T128 = true;
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &bowflext216treadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &bowflext216treadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &bowflext216treadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &bowflext216treadmill::controllerStateChanged);

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

void bowflext216treadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        Speed = 0;
        emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
        qDebug() << QStringLiteral("trying to connect back again...");
        qDebug() << QStringLiteral("resetting to speed to 0...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool bowflext216treadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool bowflext216treadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool bowflext216treadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
