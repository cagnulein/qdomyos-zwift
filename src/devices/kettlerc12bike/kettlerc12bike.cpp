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

    if (gattCommunicationChannelService &&
        gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
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

    // Phase 1: Basic initialization
    uint8_t init1[] = {0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    writeCharacteristic(init1, sizeof(init1), QStringLiteral("init1"), false, true);

    uint8_t init2[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00};
    writeCharacteristic(init2, sizeof(init2), QStringLiteral("init2"), false, true);

    // Phase 2: Sequential setup (3b00 through 3b04)
    for (int i = 0; i <= 4; i++) {
        uint8_t setup[] = {0x3b, (uint8_t)i};
        writeCharacteristic(setup, sizeof(setup), QStringLiteral("setup") + QString::number(i), false, true);
    }

    uint8_t setup_final[] = {0x3b, 0x05, 0x02, 0x00};
    writeCharacteristic(setup_final, sizeof(setup_final), QStringLiteral("setup_final"), false, true);

    // Phase 3: Configuration
    uint8_t config1[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02};
    writeCharacteristic(config1, sizeof(config1), QStringLiteral("config1"), false, true);

    uint8_t config2[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00};
    writeCharacteristic(config2, sizeof(config2), QStringLiteral("config2"), false, true);

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
    // Cycle through different poll commands based on counter
    switch (counterPoll % 4) {
        case 0: {
            uint8_t poll1[] = {0xe1, 0x00};
            writeCharacteristic(poll1, sizeof(poll1), QStringLiteral("poll1"), true, false);
            break;
        }
        case 1: {
            uint8_t poll2[] = {0xe2, 0x00};
            writeCharacteristic(poll2, sizeof(poll2), QStringLiteral("poll2"), true, false);
            break;
        }
        case 2: {
            uint8_t poll3[] = {0xe3, 0x00};
            writeCharacteristic(poll3, sizeof(poll3), QStringLiteral("poll3"), true, false);
            break;
        }
        case 3: {
            uint8_t keepalive[] = {0xc0, 0xc0};
            writeCharacteristic(keepalive, sizeof(keepalive), QStringLiteral("keepalive"), true, false);
            break;
        }
    }
    counterPoll++;
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

    // Get custom Kettler service - this is the ONLY service we use
    if (m_control->services().contains(KETTLER_SERVICE_UUID)) {
        gattCommunicationChannelService = m_control->createServiceObject(KETTLER_SERVICE_UUID, this);
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &kettlerc12bike::stateChanged);
        gattCommunicationChannelService->discoverDetails();
    } else {
        qDebug() << QStringLiteral("Kettler C12 custom service not found!");
    }
}

void kettlerc12bike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    lastPacket = newValue;
    lastRefreshCharacteristicChanged = now;

    // Get the handle value to determine which metric this is
    // Note: We need to check the characteristic handle, not UUID
    // Handle 0x0034 = Cadence (value / 10 = RPM)
    // Handle 0x003e = Power (value = Watts)
    // Handle 0x0048 = Speed (value / 10 = km/h)

    // For now, log all notifications to help identify them
    qDebug() << "characteristicChanged" << characteristic.uuid() << "handle:" << QString::number(characteristic.handle(), 16)
             << "value:" << newValue.toHex(' ');

    // Parse based on data length and pattern
    if (newValue.length() == 2) {
        // 2-byte values: little-endian format (byte0 + byte1*256)
        uint16_t value = ((uint8_t)newValue.at(1) << 8) | (uint8_t)newValue.at(0);

        // Heuristic to identify which metric based on value range:
        // Cadence: 200-800 (raw) → 20-80 RPM after /10
        // Power: 0-250 W
        // Speed: 0-400 (raw) → 0-40 km/h after /10

        if (value >= 200 && value <= 900) {
            // Likely Cadence (handle 0x0034)
            Cadence = value / 10.0;
            qDebug() << "Cadence:" << Cadence.value() << "RPM";
        } else if (value >= 0 && value < 200) {
            // Likely Power (handle 0x003e)
            m_watt = value;
            qDebug() << "Power:" << m_watt.value() << "W";
        } else if (value >= 0 && value <= 500) {
            // Likely Speed (handle 0x0048)
            Speed = value / 10.0;
            qDebug() << "Speed:" << Speed.value() << "km/h";
        }
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
    QBluetoothUuid _gattCommunicationChannelServiceId = KETTLER_SERVICE_UUID;
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto service = qobject_cast<QLowEnergyService *>(sender());

        if (service && service->serviceUuid() == _gattCommunicationChannelServiceId) {
            // Custom Kettler service - this is the ONLY service we use
            qDebug() << QStringLiteral("Kettler C12 custom service discovered");

            // Find write characteristic (0x2a28 - Software Revision String, repurposed for control)
            foreach (QLowEnergyCharacteristic c, service->characteristics()) {
                qDebug() << QStringLiteral("characteristic") << c.uuid()
                         << "handle:" << QString::number(c.handle(), 16)
                         << "properties:" << c.properties();

                // Write characteristic for sending commands
                if (c.uuid() == QBluetoothUuid((quint16)0x2a28)) {
                    gattWriteCharacteristic = c;
                    qDebug() << QStringLiteral("Found write characteristic (0x2a28)");
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
    qDebug() << QStringLiteral("descriptorWritten ") << descriptor.name() << descriptor.uuid().toString();
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
