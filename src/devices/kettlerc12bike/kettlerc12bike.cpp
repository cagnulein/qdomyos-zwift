#include "kettlerc12bike.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "homeform.h"
#include "qzsettings.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

kettlerc12bike::kettlerc12bike(bool noWriteResistance, bool noHeartService, bool testResistance,
                               int8_t bikeResistanceOffset, double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);

    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->testResistance = testResistance;
    this->bikeResistanceOffset = bikeResistanceOffset;
    this->bikeResistanceGain = bikeResistanceGain;

    initDone = false;
    connect(refresh, &QTimer::timeout, this, &kettlerc12bike::update);
    refresh->start(500ms);
}

kettlerc12bike::~kettlerc12bike() {
    qDebug() << QStringLiteral("~kettlerc12bike()");
}

void kettlerc12bike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                         bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if ((gattCommunicationChannelService &&
         gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered) ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattCommunicationChannelService && gattWriteCharacteristic.isValid()) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    }

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info;
    }

    loop.exec();

    if (timeout.isActive() == false) {
        qDebug() << QStringLiteral(" exit for timeout");
    }
}

void kettlerc12bike::btinit() {
    qDebug() << QStringLiteral("kettlerc12bike::btinit");
    initDone = true;
}

void kettlerc12bike::forceResistance(resistance_t requestResistance) {
    // For ergometer mode: resistance directly maps to target power in watts
    // Since this is an ergometer (0-250W), we treat resistance as target power
    qDebug() << QStringLiteral("kettlerc12bike::forceResistance") << requestResistance;
    setTargetPower((uint16_t)requestResistance);
}

void kettlerc12bike::setTargetPower(uint16_t targetWatts) {
    if (noWriteResistance) {
        return;
    }

    // Clamp to valid range (0-250 watts for ergometer)
    if (targetWatts > 250) targetWatts = 250;

    qDebug() << QStringLiteral("kettlerc12bike::setTargetPower") << targetWatts << QStringLiteral("watts");

    // Pre-command before power change
    uint8_t precmd[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x20, 0x02};
    writeCharacteristic(precmd, sizeof(precmd), QStringLiteral("power_pre"), false, false);

    // Initial power command
    uint8_t initcmd[] = {0xf0, 0x00, 0x00, 0x00, 0x70, 0x09, 0x00, 0x00};
    writeCharacteristic(initcmd, sizeof(initcmd), QStringLiteral("power_init"), false, false);

    // Set target power using F-series commands
    // The exact encoding needs testing, using F0 + watt level approach
    uint8_t level = 0xf0 + (targetWatts / 10);  // Tentative: divide watts by 10

    // First power command with 0x40
    uint8_t powercmd1[] = {level, 0x40};
    writeCharacteristic(powercmd1, sizeof(powercmd1),
                       QStringLiteral("power_set_") + QString::number(targetWatts), false, false);

    // Confirmation with 0xc0
    uint8_t powercmd2[] = {level, 0xc0};
    writeCharacteristic(powercmd2, sizeof(powercmd2),
                       QStringLiteral("power_confirm_") + QString::number(targetWatts), false, true);
}

void kettlerc12bike::sendPoll() {
    // The working HCI trace already streams live metrics as soon as notifications are
    // enabled, so avoid sending speculative poll traffic that can move the bike into
    // a different protocol mode.
}

void kettlerc12bike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() &&
               initDone) {

        update_metrics(true, watts());

        // Send periodic poll commands
        sendPoll();

        // ******************************************* virtual bike init *************************************
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
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence && !this->hasVirtualDevice()) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                qDebug() << QStringLiteral("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                connect(virtualBike, &virtualbike::changeInclination, this, &bike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
            firstStateChanged = 1;
        }
        // ********************************************************************************************
    }
}

void kettlerc12bike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") << gatt.toString();
}

void kettlerc12bike::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    const auto services = m_control->services();
    activeServiceUuid = QBluetoothUuid();

    if (services.contains(KETTLER_SERVICE_UUID)) {
        activeServiceUuid = KETTLER_SERVICE_UUID;
    } else if (services.contains(KETTLER_ALT_SERVICE_UUID)) {
        activeServiceUuid = KETTLER_ALT_SERVICE_UUID;
    } else {
        for (const auto &serviceUuid : services) {
            const QString serviceString = serviceUuid.toString().toLower();
            if (!serviceString.endsWith(QStringLiteral("-0000-1000-8000-00805f9b34fb"))) {
                activeServiceUuid = serviceUuid;
                qDebug() << QStringLiteral("Using fallback Kettler custom service") << activeServiceUuid.toString();
                break;
            }
        }
    }

    if (!activeServiceUuid.isNull()) {
        gattCommunicationChannelService = m_control->createServiceObject(activeServiceUuid, this);
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &kettlerc12bike::stateChanged);
        gattCommunicationChannelService->discoverDetails();
    } else {
        qDebug() << QStringLiteral("Kettler C12 custom service not found!") << services;
    }
}

void kettlerc12bike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    lastPacket = newValue;
    lastRefreshCharacteristicChanged = now;

    const quint16 handle = characteristic.handle();
    qDebug() << "characteristicChanged" << characteristic.uuid() << "handle:" << QString::number(handle, 16)
             << "value:" << newValue.toHex(' ');

    if (newValue.length() >= 2) {
        const uint16_t value = ((uint8_t)newValue.at(1) << 8) | (uint8_t)newValue.at(0);

        switch (handle) {
            case 0x34:
                Cadence = value / 10.0;
                break;
            case 0x3e:
                m_watt = value;
                break;
            case 0x48:
                Speed = value / 10.0;
                break;
            case 0x4c:
                Resistance = value;
                break;
            case 0x63:
                if (newValue.length() >= 2) {
                    Resistance = value;
                }
                break;
            default:
                break;
        }
    }

    if (handle == 0x5c && newValue.length() >= 4) {
        const uint32_t distanceRaw = (uint8_t)newValue.at(0) |
                                     ((uint8_t)newValue.at(1) << 8) |
                                     ((uint8_t)newValue.at(2) << 16) |
                                     ((uint8_t)newValue.at(3) << 24);
        Distance = distanceRaw / 1000.0;
    }

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
}

void kettlerc12bike::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto service = qobject_cast<QLowEnergyService *>(sender());

        if (service && service->serviceUuid() == activeServiceUuid) {
            qDebug() << QStringLiteral("Kettler C12 custom service discovered") << activeServiceUuid.toString();

            gattWriteCharacteristic = QLowEnergyCharacteristic();
            gattHandshakeCharacteristic = QLowEnergyCharacteristic();
            foreach (QLowEnergyCharacteristic c, service->characteristics()) {
                qDebug() << QStringLiteral("characteristic") << c.uuid()
                         << "handle:" << QString::number(c.handle(), 16)
                         << "properties:" << c.properties();

                if (c.uuid() == KETTLER_CONTROL_CHAR_UUID) {
                    gattWriteCharacteristic = c;
                    qDebug() << QStringLiteral("Found Kettler control characteristic") << c.uuid()
                             << "handle:" << QString::number(c.handle(), 16);
                } else if (c.uuid() == KETTLER_HANDSHAKE_CHAR_UUID) {
                    gattHandshakeCharacteristic = c;
                } else if (c.uuid() == KETTLER_CADENCE_CHAR_UUID) {
                    gattNotify0x0034 = c;
                } else if (c.uuid() == KETTLER_POWER_CHAR_UUID) {
                    gattNotify0x003e = c;
                } else if (c.uuid() == KETTLER_SPEED_CHAR_UUID) {
                    gattNotify0x0048 = c;
                } else if (!gattWriteCharacteristic.isValid() &&
                           ((c.properties() & QLowEnergyCharacteristic::Write) ||
                            (c.properties() & QLowEnergyCharacteristic::WriteNoResponse))) {
                    gattWriteCharacteristic = c;
                    qDebug() << QStringLiteral("Using fallback write characteristic") << c.uuid()
                             << "handle:" << QString::number(c.handle(), 16);
                }

                // Enable notifications on ALL characteristics with Notify property
                // The bike sends Speed/Cadence/Power on different characteristics (handles 0x0034, 0x003e, 0x0048)
                if (c.properties() & QLowEnergyCharacteristic::Notify) {
                    qDebug() << QStringLiteral("Enabling notifications on characteristic") << c.uuid()
                             << "handle:" << QString::number(c.handle(), 16);

                    // Enable notifications
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    service->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration),
                                           descriptor);
                }
            }

            connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                    &kettlerc12bike::characteristicChanged);
            connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                    &kettlerc12bike::characteristicWritten);
            connect(gattCommunicationChannelService,
                    static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                    this, &kettlerc12bike::errorService);
            connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                    &kettlerc12bike::descriptorWritten);

            if (gattWriteCharacteristic.isValid()) {
                qDebug() << QStringLiteral("Starting initialization...");
                initRequest = true;
            } else {
                qDebug() << QStringLiteral("ERROR: Write characteristic not found!");
            }
        }
    }
}

void kettlerc12bike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") << descriptor.name() << descriptor.uuid().toString()
             << newValue.toHex(' ');

    if (!initDone && !initRequest) {
        initRequest = true;
        emit connectedAndDiscovered();
    }
}

void kettlerc12bike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") << newValue.toHex(' ');
}

void kettlerc12bike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;

    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        initRequest = false;
        m_control->connectToDevice();
    }
}

void kettlerc12bike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("kettlerc12bike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void kettlerc12bike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("kettlerc12bike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err));
}

void kettlerc12bike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found device: ") + device.name();

    if (device.name().startsWith(QStringLiteral("ERGO C12")) ||
        device.name().toUpper().startsWith(QStringLiteral("KETTLER C12"))) {

        bluetoothDevice = device;

        if (device.name().toUpper().startsWith(QStringLiteral("KETTLER C12"))) {
            qDebug() << QStringLiteral("KETTLER C12 found");
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &kettlerc12bike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &kettlerc12bike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &kettlerc12bike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &kettlerc12bike::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    qDebug() << QStringLiteral("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool kettlerc12bike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t kettlerc12bike::watts() {
    // Return the actual power reading from the bike
    return m_watt.value();
}

void kettlerc12bike::startDiscover() {
    // No-op: device discovery is handled by bluetooth class
}

resistance_t kettlerc12bike::pelotonToBikeResistance(int pelotonResistance) {
    // Map Peloton resistance (0-100) to target power (0-250 watts)
    // This is for ergometer mode: resistance = target power
    return (pelotonResistance * max_resistance) / 100;
}
