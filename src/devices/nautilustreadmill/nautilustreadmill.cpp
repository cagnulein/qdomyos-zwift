#include "nautilustreadmill.h"
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

nautilustreadmill::nautilustreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService,
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
    connect(refresh, &QTimer::timeout, this, &nautilustreadmill::update);
    refresh->start(500ms);
}

void nautilustreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                            bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &nautilustreadmill::packetReceived, &loop, &QEventLoop::quit);
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

void nautilustreadmill::updateDisplay(uint16_t elapsed) {}

void nautilustreadmill::forceIncline(double requestIncline) {}

double nautilustreadmill::minStepInclination() { return 1.0; }

void nautilustreadmill::forceSpeed(double requestSpeed) {}

void nautilustreadmill::update() {
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
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &nautilustreadmill::debug);
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
            if(T628) {
                Inclination = requestInclination;
                emit debug(QStringLiteral("Current incline: ") + QString::number(Inclination.value()));
            }
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
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {
                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape", false, true);
            requestStop = -1;
        }
    }
}

void nautilustreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void nautilustreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    emit packetReceived();

    if (newValue.length() == 12) {
        double speed = GetSpeedFromPacket(value);
        double incline = GetInclinationFromPacket(value);

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                update_hr_from_external();
            }
        }

        emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
        // debug("Current Distance: " + QString::number(distance));

        if (Speed.value() != speed) {
            emit speedChanged(speed);
        }
        Speed = speed;
        if(!T628) {
            if (Inclination.value() != incline) {
                emit inclinationChanged(0.0, incline);
            }
            Inclination = incline;
            emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
        }

        // KCal = kcal;
        // Distance = distance;

        if (!firstCharacteristicChanged) {
            if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal +=
                    ((((0.048 *
                            ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastTimeCharacteristicChanged.msecsTo(
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60

            Distance += ((Speed.value() / 3600.0) /
                         (1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
        }

        cadenceFromAppleWatch();

        emit debug(QStringLiteral("Current KCal Calculated: ") + QString::number(KCal.value()));
        emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

        if (m_control->error() != QLowEnergyController::NoError) {
            qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
        }

        lastTimeCharacteristicChanged = QDateTime::currentDateTime();
        firstCharacteristicChanged = false;

        if (Speed.value() > 0) {
            lastSpeed = Speed.value();
            lastInclination = Inclination.value();
        }
    }
}

double nautilustreadmill::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (uint16_t)((uint8_t)packet.at(3)) + (uint16_t)((((uint8_t)packet.at(4)) << 8) & 0xFF00);
    const double miles = 1.60934;
    double data = (((double)convertedData) * miles) / 100.0f;
    return data;
}

double nautilustreadmill::GetInclinationFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(6);
    double data = convertedData;

    return data;
}

void nautilustreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)
    uint8_t initData1[] = {0x07, 0x01, 0xd3, 0x00, 0x1f, 0x05, 0x01};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);

    initDone = true;
}

void nautilustreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("1717b3c0-9803-11e3-90e1-0002a5d5c51b"));
        QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("a46a4a80-9803-11e3-8f3c-0002a5d5c51b"));
        QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("6be8f580-9803-11e3-ab03-0002a5d5c51b"));

        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid();
            auto descriptors_list = c.descriptors();
            for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
            }
        }

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &nautilustreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &nautilustreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &nautilustreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &nautilustreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify2Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void nautilustreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void nautilustreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void nautilustreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId =
        QBluetoothUuid(QStringLiteral("89e14b5e-a841-48ad-b580-935f4545fffc"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (!gattCommunicationChannelService) {
        _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("f12f71ab-c228-45f2-98a2-6654f9adbe78"));

        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
        if (!gattCommunicationChannelService) {
            _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("19df5b20-31c7-11e6-a336-0002a5d5c51b"));

            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
            if (!gattCommunicationChannelService) {
                return;
            }
        }
    }
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &nautilustreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void nautilustreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("nautilustreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void nautilustreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("nautilustreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void nautilustreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        if(device.name().toUpper().startsWith(QStringLiteral("NAUTILUS T628"))) {
            qDebug() << "NAUTILUS T628 workaround";
            T628 = true;
        }
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &nautilustreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &nautilustreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &nautilustreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &nautilustreadmill::controllerStateChanged);

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

void nautilustreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool nautilustreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool nautilustreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool nautilustreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
