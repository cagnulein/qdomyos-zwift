#include "lifespanbike.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

lifespanbike::lifespanbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                           double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    m_rawWatt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceOffset = bikeResistanceOffset;
    this->bikeResistanceGain = bikeResistanceGain;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &lifespanbike::update);
    refresh->start(500ms);
}

void lifespanbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                       bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    QByteArray command((const char *)data, data_len);
    lastPacket = command;

    if (command.startsWith(QByteArray::fromHex("A191"))) {
        currentCommand = CommandState::QueryStatus;
    } else if (command.startsWith(QByteArray::fromHex("A181"))) {
        currentCommand = CommandState::QueryUnknown81;
    } else if (command.startsWith(QByteArray::fromHex("A18A"))) {
        currentCommand = CommandState::QueryCadence;
    } else if (command.startsWith(QByteArray::fromHex("A186"))) {
        currentCommand = CommandState::QueryUnknown86;
    } else if (command.startsWith(QByteArray::fromHex("A187"))) {
        currentCommand = CommandState::QueryCalories;
    } else if (command.startsWith(QByteArray::fromHex("A185"))) {
        currentCommand = CommandState::QueryDistance;
    } else if (command.startsWith(QByteArray::fromHex("A189"))) {
        currentCommand = CommandState::QueryTime;
    } else if (command.startsWith(QByteArray::fromHex("A182"))) {
        currentCommand = CommandState::QuerySpeed;
    } else if (command.startsWith(QByteArray::fromHex("A184"))) {
        currentCommand = CommandState::QueryPower;
    }

    if (wait_for_response) {
        connect(this, &lifespanbike::packetReceived, &loop, &QEventLoop::quit);
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

    if (!disable_log) {
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;
    }

    loop.exec();
}

void lifespanbike::btinit() {
    uint8_t initData[] = {0x02, 0x00, 0x00, 0x00, 0x00};
    writeCharacteristic(initData, sizeof(initData), QStringLiteral("init"), false, true);
    initDone = true;
}

void lifespanbike::forceResistance(resistance_t requestResistance) {
    Q_UNUSED(requestResistance);
}

void lifespanbike::sendPoll() {
    static const uint8_t commands[][5] = {
        {0xA1, 0x91, 0x00, 0x00, 0x00}, {0xA1, 0x81, 0x00, 0x00, 0x00},
        {0xA1, 0x8A, 0x00, 0x00, 0x00}, {0xA1, 0x86, 0x00, 0x00, 0x00},
        {0xA1, 0x87, 0x00, 0x00, 0x00}, {0xA1, 0x85, 0x00, 0x00, 0x00},
        {0xA1, 0x89, 0x00, 0x00, 0x00}, {0xA1, 0x82, 0x00, 0x00, 0x00},
        {0xA1, 0x84, 0x00, 0x00, 0x00},
    };

    const auto &command = commands[pollIndex];
    writeCharacteristic((uint8_t *)command, sizeof(command), QStringLiteral("poll"), false, true);
    pollIndex = (pollIndex + 1) % (sizeof(commands) / sizeof(commands[0]));
}

uint16_t lifespanbike::data16(const QByteArray &packet) const {
    if (packet.length() < 4) {
        return 0;
    }
    return ((uint16_t)((uint8_t)packet.at(2)) << 8) | (uint16_t)((uint8_t)packet.at(3));
}

uint16_t lifespanbike::adjustedLifespanWatts() {
    QSettings settings;
    const uint16_t rawWatts = m_rawWatt.value();

    if (!settings.value(QZSettings::lifespan_bike, QZSettings::default_lifespan_bike).toBool() || rawWatts == 0) {
        return rawWatts;
    }

    const double resistance = Resistance.value();
    const double cadence = Cadence.value();
    if (resistance <= 0.0 || cadence <= 0.0) {
        return rawWatts;
    }

    return qRound(rawWatts * resistance * (cadence / 60.0));
}

double lifespanbike::GetSpeedFromPacket(const QByteArray &packet) const {
    if (packet.length() < 4) {
        return 0.0;
    }
    return (double)((uint8_t)packet.at(2)) + ((double)((uint8_t)packet.at(3)) / 100.0);
}

void lifespanbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() && initDone) {
        if (!firstInit && !this->hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstInit = 1;
            }
        }

        sendPoll();

        if (requestResistance != -1) {
            forceResistance(requestResistance);
            requestResistance = -1;
        }

        if (requestStart != -1) {
            requestStart = -1;
            emit bikeStarted();
        }

        if (requestStop != -1) {
            requestStop = -1;
        }

        update_metrics(true, watts());
    }
}

void lifespanbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    QSettings settings;
    QDateTime now = QDateTime::currentDateTime();
    QByteArray value = newValue;
    qDebug() << " << " << value.length() << value.toHex(' ') << (int)currentCommand;

    switch (currentCommand) {
    case CommandState::QueryStatus:
        if (value.length() >= 3) {
            Resistance = (uint8_t)value.at(2);
            emit resistanceRead(Resistance.value());
            m_pelotonResistance = Resistance.value();
        }
        break;
    case CommandState::QueryCadence:
        if (value.length() >= 4 &&
            settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            // A18A responses carry cadence in the fourth byte (for example 0x43 = 67 rpm).
            Cadence = (uint8_t)value.at(3);
        }
        break;
    case CommandState::QueryCalories:
        KCal = data16(value);
        break;
    case CommandState::QueryDistance:
        Distance = data16(value) / 10.0;
        break;
    case CommandState::QuerySpeed:
        Speed = GetSpeedFromPacket(value);
        break;
    case CommandState::QueryPower:
        m_rawWatt = data16(value);
        m_watt = adjustedLifespanWatts();
        break;
    default:
        break;
    }

    if (!firstCharacteristicChanged) {
        if (watts()) {
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(now))));
        }

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }
    }

    update_hr_from_external();
    lastTimeCharacteristicChanged = now;
    firstCharacteristicChanged = false;
    currentCommand = CommandState::None;
    emit packetReceived();
}

bool lifespanbike::connected() {
    return initDone;
}

uint16_t lifespanbike::watts() {
    return adjustedLifespanWatts();
}

void lifespanbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void lifespanbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId((uint16_t)0xfff0);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (gattCommunicationChannelService == nullptr) {
        qDebug() << "WRONG SERVICE";
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &lifespanbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void lifespanbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void lifespanbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
    initRequest = true;
    emit connectedAndDiscovered();
}

void lifespanbike::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::ServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId((uint16_t)0xfff2);
        QBluetoothUuid _gattNotifyCharacteristicId((uint16_t)0xfff1);
        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);

        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &lifespanbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &lifespanbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &lifespanbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &lifespanbike::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void lifespanbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState) {
        Speed = 0;
        initDone = false;
        emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
    }
}

void lifespanbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)));
}

void lifespanbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)));
}

void lifespanbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    bluetoothDevice = device;
    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &lifespanbike::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &lifespanbike::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &lifespanbike::error);
    connect(m_control, &QLowEnergyController::stateChanged, this, &lifespanbike::controllerStateChanged);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, [this](QLowEnergyController::Error error) {
                Q_UNUSED(error);
                emit debug(QStringLiteral("Cannot connect to remote device."));
                emit disconnected();
            });
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        emit debug(QStringLiteral("Controller connected. Search services..."));
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        emit debug(QStringLiteral("LowEnergy controller disconnected"));
        emit disconnected();
    });

    m_control->connectToDevice();
}
