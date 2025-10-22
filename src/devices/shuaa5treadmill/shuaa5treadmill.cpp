#include "shuaa5treadmill.h"
#include "devices/ftmsbike/ftmsbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif

#include <chrono>

using namespace std::chrono_literals;

shuaa5treadmill::shuaa5treadmill(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &shuaa5treadmill::update);
    refresh->start(200ms);
}

void shuaa5treadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &shuaa5treadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(2000, &loop, SLOT(quit()));
    } else {
        connect(gattFTMSService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(2000, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer);

    if (!disable_log)
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;

    loop.exec();
}

void shuaa5treadmill::btinit() {
    uint8_t initData01[] = {FTMS_REQUEST_CONTROL};
    uint8_t initData02[] = {FTMS_START_RESUME};

    writeCharacteristic(initData01, sizeof(initData01), QStringLiteral("init1"), false, true);
    writeCharacteristic(initData02, sizeof(initData02), QStringLiteral("init2"), false, true);

    initDone = true;
}

void shuaa5treadmill::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &shuaa5treadmill::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

void shuaa5treadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest && firstStateChanged) {
        uint8_t initData01[] = {FTMS_REQUEST_CONTROL};
        writeCharacteristic(initData01, sizeof(initData01), QStringLiteral("init1"), false, true);
        initRequest = false;
    } else if (bluetoothDevice.isValid() //&&

               // m_control->state() == QLowEnergyController::DiscoveredState //&&
               // gattCommunicationChannelService &&
               // gattWriteCharacteristic.isValid() &&
               // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {

        QSettings settings;
        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
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
            btinit();
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

            requestStop = -1;
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

void shuaa5treadmill::forceSpeed(double requestSpeed) {
    uint8_t i[] = {FTMS_SET_TARGET_SPEED, 0x00, 0x00};
    i[1] = (uint8_t)(requestSpeed * 100.0);
    i[2] = (uint8_t)((uint16_t)(requestSpeed * 100.0) >> 8);

    writeCharacteristic(i, sizeof(i), QStringLiteral("request Speed") + QString::number(requestSpeed), false, true);
}

void shuaa5treadmill::forceIncline(double requestIncline) {
    uint8_t i[] = {FTMS_SET_TARGET_INCLINATION, 0x00, 0x00};
    i[1] = (uint8_t)(requestIncline * 10.0);

    writeCharacteristic(i, sizeof(i), QStringLiteral("request Inclination") + QString::number(requestIncline), false,
                        true);
}

double shuaa5treadmill::minStepInclination() { return 1.0; }

void shuaa5treadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void shuaa5treadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    double heart = 0; // NOTE : Should be initialized with a value to shut clang-analyzer's
                      // UndefinedBinaryOperatorResult
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + characteristic.uuid().toString() + " " + QString::number(newValue.length()) +
               " " + newValue.toHex(' '));

    emit packetReceived();

    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACD)) {
        lastPacket = newValue;

        // default flags for this treadmill is 84 04

        union flags {
            struct {

                uint16_t moreData : 1;
                uint16_t avgSpeed : 1;
                uint16_t totalDistance : 1;
                uint16_t inclination : 1;
                uint16_t elevation : 1;
                uint16_t instantPace : 1;
                uint16_t averagePace : 1;
                uint16_t expEnergy : 1;
                uint16_t heartRate : 1;
                uint16_t metabolic : 1;
                uint16_t elapsedTime : 1;
                uint16_t remainingTime : 1;
                uint16_t forceBelt : 1;
                uint16_t spare : 3;
            };

            uint16_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {
            double speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                     (uint16_t)((uint8_t)newValue.at(index)))) /
                           100.0;
            if (Speed.value() != speed) {

                emit speedChanged(speed);
            }
            Speed = speed;
            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.totalDistance) {
            // ignoring the distance, because it's a total life odometer
            // Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
            // (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
            index += 3;
        }
        // else
        {
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.inclination) {
            Inclination = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          10.0;
            index += 4; // the ramo value is useless
            emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        }

        if (Flags.elevation) {
            index += 4; // TODO
        }

        if (Flags.instantPace) {
            index += 1; // TODO
        }

        if (Flags.averagePace) {
            index += 1; // TODO
        }

        if (Flags.expEnergy) {
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal +=
                    ((((0.048 *
                            ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged.msecsTo(
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate) {
                if (index < newValue.length()) {

                    heart = ((double)(((uint8_t)newValue.at(index))));
                    emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
                } else {
                    emit debug(QStringLiteral("Error on parsing heart!"));
                }
                // index += 1; //NOTE: clang-analyzer-deadcode.DeadStores
            }
        }

        if (Flags.metabolic) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }

        if (Flags.forceBelt) {
            // todo
        }
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0 ||
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool()) {
            update_hr_from_external();
        } else {
            Heart = heart;
        }
    }

    cadenceFromAppleWatch();

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void shuaa5treadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::RemoteServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::RemoteServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &shuaa5treadmill::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &shuaa5treadmill::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &shuaa5treadmill::characteristicRead);
            connect(
                s, &QLowEnergyService::errorOccurred,
                this, &shuaa5treadmill::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &shuaa5treadmill::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &shuaa5treadmill::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("FTMS service and Control Point found");
                    gattWriteCharControlPointId = c;
                    gattFTMSService = s;
                }
            }
        }
    }

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {

        QSettings settings;
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual treadmill interface..."));

            auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
            connect(virtualTreadmill, &virtualtreadmill::debug, this, &shuaa5treadmill::debug);
            connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                    &shuaa5treadmill::changeInclinationRequested);
            this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void shuaa5treadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void shuaa5treadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void shuaa5treadmill::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void shuaa5treadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void shuaa5treadmill::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void shuaa5treadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;
    firstStateChanged = 0;
    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &shuaa5treadmill::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void shuaa5treadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("shuaa5treadmill::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void shuaa5treadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("shuaa5treadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void shuaa5treadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &shuaa5treadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &shuaa5treadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &shuaa5treadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &shuaa5treadmill::controllerStateChanged);

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

bool shuaa5treadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void shuaa5treadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}
