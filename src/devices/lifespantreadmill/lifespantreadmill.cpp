#include "lifespantreadmill.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

lifespantreadmill::lifespantreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService,
                                   double forceInitSpeed, double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;
    this->pollDeviceTime = pollDeviceTime;

    if (forceInitSpeed > 0)
        lastSpeed = forceInitSpeed;

    if (forceInitInclination > 0)
        lastInclination = forceInitInclination;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &lifespantreadmill::update);
    refresh->start(500ms);
}

void lifespantreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, const QString& info, bool disable_log,
                                         bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    QByteArray command((const char*)data, data_len);
    lastPacket = command;

    // Determine command type from packet
    if (command.startsWith(QByteArray::fromHex("A182"))) {
        currentCommand = CommandState::QuerySpeed;
    } else if (command.startsWith(QByteArray::fromHex("A185"))) {
        currentCommand = CommandState::QueryDistance;
    } else if (command.startsWith(QByteArray::fromHex("A187"))) {
        currentCommand = CommandState::QueryCalories;
    } else if (command.startsWith(QByteArray::fromHex("A189"))) {
        currentCommand = CommandState::QueryTime;
    } else if (command.startsWith(QByteArray::fromHex("D0"))) {
        currentCommand = CommandState::SetSpeed;
    } else if (command.startsWith(QByteArray::fromHex("E1"))) {
        currentCommand = CommandState::Start;
    } else if (command.startsWith(QByteArray::fromHex("E0"))) {
        currentCommand = CommandState::Stop;
    } else if (command.startsWith(QByteArray::fromHex("A188"))) {
        currentCommand = CommandState::QuerySteps;
    }

    if (wait_for_response) {
        connect(this, &lifespantreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    if (!disable_log)
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;

    loop.exec();
}

void lifespantreadmill::btinit(bool startTape) {
    const QByteArray initSequence[] = {
        QByteArray::fromHex("0200000000"), // Unknown
        QByteArray::fromHex("C200000000"), // Firmware
        QByteArray::fromHex("E9FF000000"), // Zeroes
        QByteArray::fromHex("E400F40000")  // Zeroes
    };

    for (const auto& cmd : initSequence) {
        writeCharacteristic((uint8_t*)cmd.data(), cmd.size(), QStringLiteral("init"), false, true);
    }

    initDone = true;
}

double lifespantreadmill::GetSpeedFromPacket(const QByteArray& packet) {
    if (packet.length() < 4) return 0.0;
    return ((double)((uint16_t)((uint8_t)packet.at(2)) + ((uint16_t)((uint8_t)packet.at(3))) / 100.0));
}

double lifespantreadmill::GetInclinationFromPacket(const QByteArray& packet) {
    if (packet.length() < 3) return 0.0;
    return packet[2];
}

double lifespantreadmill::GetKcalFromPacket(const QByteArray& packet) {
    if (packet.length() < 4) return 0.0;
    return (packet[2] << 8) | packet[3];
}

double lifespantreadmill::GetDistanceFromPacket(const QByteArray& packet) {
    if (packet.length() < 4) return 0.0;
    double data = ((packet[2] << 8) | packet[3]) / 10.0;
    return data;
}

void lifespantreadmill::forceSpeed(double requestSpeed) {
    uint16_t speed_int = (uint16_t)(requestSpeed * 100);
    uint8_t units = speed_int / 100;
    uint8_t hundredths = speed_int % 100;
    uint8_t cmd[] = {0xd0, units, hundredths, 0, 0};
    writeCharacteristic(cmd, sizeof(cmd), QStringLiteral("set speed"), false, true);
}

void lifespantreadmill::forceIncline(double requestIncline) {
    // Not implemented for this model
}

void lifespantreadmill::updateDisplay(uint16_t elapsed) {
    // Not implemented for this model
}

void lifespantreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

uint32_t lifespantreadmill::GetStepsFromPacket(const QByteArray& packet) {
    if (packet.length() < 4) return 0;
    return ((uint16_t)((uint8_t)packet[2]) << 8) | (uint16_t)((uint8_t)packet[3]);
}

void lifespantreadmill::update() {

    if (!m_control)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    static uint8_t queue = 0;

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
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &lifespantreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &lifespantreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        if(queue == 0) {
            // Query metrics periodically
            uint8_t speedQuery[] = {0xA1, 0x82, 0x00, 0x00, 0x00};
            writeCharacteristic(speedQuery, sizeof(speedQuery), QStringLiteral("query speed"), false, true);
            queue = 1;
        } else {
            uint8_t stepQuery[] = {0xA1, 0x88, 0x00, 0x00, 0x00};
            writeCharacteristic(stepQuery, sizeof(stepQuery), QStringLiteral("query steps"), false, true);
            queue = 0;
        }

        if (requestStart != -1) {
            uint8_t start[] = {0xE1, 0x00, 0x00, 0x00, 0x00};
            writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
            requestStart = -1;
            emit tapeStarted();
        }

        if (requestStop != -1 || requestPause != -1) {
            uint8_t stop[] = {0xE0, 0x00, 0x00, 0x00, 0x00};
            writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
            requestStop = -1;
            requestPause = -1;
        }

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));
    }
}

void lifespantreadmill::characteristicChanged(const QLowEnergyCharacteristic& characteristic,
                                           const QByteArray& newValue) {
    QSettings settings;
    QByteArray value = newValue;
    qDebug() << " << " << value.length() << value.toHex(' ') << (int)currentCommand;

    double speed = 0.0;
    switch(currentCommand) {
        case CommandState::QuerySpeed:
            speed = GetSpeedFromPacket(value);
            if (Speed.value() != speed) {
                emit speedChanged(speed);                
            }
            emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
            Speed = speed;
            if (speed > 0) {
                lastSpeed = speed;
            }
            break;
        case CommandState::QueryDistance:
            Distance = GetDistanceFromPacket(value);
            break;
        case CommandState::QueryCalories:
            KCal = GetKcalFromPacket(value);
            break;
        case CommandState::QuerySteps:
        {
            uint32_t newSteps = GetStepsFromPacket(value);
            if (uint32_t(StepCount.value()) != newSteps) {
                StepCount = newSteps;
                emit debug(QStringLiteral("Current steps: ") + QString::number(StepCount.value()));
            }
        }
        break;
        default:
            break;
    }

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) {
            KCal += ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) / 200.0) /
                   (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
        }

        Distance += ((Speed.value() / 3600.0) /
                    (1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }

    update_hr_from_external();

    cadenceFromAppleWatch();

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
    currentCommand = CommandState::None;
    emit packetReceived();
}

bool lifespantreadmill::connected() {
    return initDone;
}

double lifespantreadmill::minStepInclination() {
    return 1.0;
}

bool lifespantreadmill::autoPauseWhenSpeedIsZero() {
    return lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000);
}

bool lifespantreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    return (lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1;
}

// Direct copy of Bowflex Bluetooth compatibility functions
void lifespantreadmill::serviceDiscovered(const QBluetoothUuid& gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void lifespantreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId((uint16_t)0xfff0);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (gattCommunicationChannelService == nullptr) {
        qDebug() << "WRONG SERVICE";
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &lifespantreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void lifespantreadmill::characteristicWritten(const QLowEnergyCharacteristic& characteristic,
                                           const QByteArray& newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void lifespantreadmill::descriptorWritten(const QLowEnergyDescriptor& descriptor,
                                       const QByteArray& newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
    initRequest = true;

    emit connectedAndDiscovered();
}

void lifespantreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId((uint16_t)0xfff2);
        QBluetoothUuid _gattNotifyCharacteristicId((uint16_t)0xfff1);
        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);

        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &lifespantreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &lifespantreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &lifespantreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &lifespantreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void lifespantreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState) {
        Speed = 0;
        emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
        initDone = false;
    }
}

void lifespantreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)));
}

void lifespantreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)));
}

void lifespantreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &lifespantreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &lifespantreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &lifespantreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &lifespantreadmill::controllerStateChanged);

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

        m_control->connectToDevice();
        return;
    }
}
