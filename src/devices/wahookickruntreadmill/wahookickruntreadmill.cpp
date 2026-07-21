#include "wahookickruntreadmill.h"
#include "homeform.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QLowEnergyConnectionParameters>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

wahookickruntreadmill::wahookickruntreadmill(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &wahookickruntreadmill::update);
    refresh->start(500ms);
}

void wahookickruntreadmill::writeCharacteristic(const QByteArray &data, const QString &info) {
    if (!gattCmdCharacteristic.isValid() || !gattCmdService) {
        emit debug(QStringLiteral("writeCharacteristic: cmd characteristic not valid"));
        return;
    }

    if (writeBuffer)
        delete writeBuffer;
    writeBuffer = new QByteArray(data);

    gattCmdService->writeCharacteristic(gattCmdCharacteristic, *writeBuffer,
                                        QLowEnergyService::WriteWithoutResponse);
    emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
}

void wahookickruntreadmill::btinit() {
    // Init sequence observed in HCI trace.
    // Single-byte capability queries sent to the cmd characteristic.
    static const QList<QByteArray> initCmds = {
        QByteArray(1, 0x01),
        QByteArray(1, 0x10),
        QByteArray(1, 0x12),
        QByteArray(1, 0x18),
        QByteArray(1, 0x80),
        QByteArray(1, 0x86), // response contains 4-byte device ID for the 0x87 follow-up
        QByteArray(1, 0xf0),
    };

    for (const QByteArray &cmd : initCmds)
        writeCharacteristic(cmd, QStringLiteral("init"));

    // 0x87 + deviceId is sent after the 0x86 response arrives in characteristicChanged().
    // Workout unlock (0x19 0x00) is sent once before the first speed command.

    initDone = true;
    emit connectedAndDiscovered();
}

void wahookickruntreadmill::forceSpeed(double speed) {
    if (noWriteResistance)
        return;

    // Send 0x19 0x00 once to unlock workout/remote-control mode
    if (!workoutModeStarted) {
        workoutModeStarted = true;
        uint8_t unlockCmd[] = {0x19, 0x00};
        writeCharacteristic(QByteArray(reinterpret_cast<const char *>(unlockCmd), 2),
                            QStringLiteral("workout start"));
    }

    // Speed in μm/s as uint24 little-endian
    uint32_t um_s = static_cast<uint32_t>(std::round(speed / 3.6 * 1e6));
    uint8_t flag = firstSpeedSent ? 0x0A : 0xFF;
    firstSpeedSent = true;

    uint8_t cmd[6];
    cmd[0] = 0x02;
    cmd[1] = um_s & 0xFF;
    cmd[2] = (um_s >> 8) & 0xFF;
    cmd[3] = (um_s >> 16) & 0xFF;
    cmd[4] = 0x00;
    cmd[5] = flag;

    writeCharacteristic(QByteArray(reinterpret_cast<const char *>(cmd), 6),
                        QStringLiteral("forceSpeed ") + QString::number(speed));
}

void wahookickruntreadmill::update() {
    if (!m_control || m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
        return;
    }

    if (!initDone || !gattCmdCharacteristic.isValid())
        return;

    // Create virtual treadmill on first valid update
    if (!this->hasVirtualDevice()) {
        QSettings settings;
        if (settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
            emit debug(QStringLiteral("creating virtual treadmill interface..."));
            auto *vt = new virtualtreadmill(this, noHeartService);
            connect(vt, &virtualtreadmill::debug, this, &wahookickruntreadmill::debug);
            this->setVirtualDevice(vt, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }

    QSettings settings;
    update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

    if (requestSpeed != -1) {
        if (requestSpeed != currentSpeed().value() && requestSpeed >= 0) {
            emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
            forceSpeed(requestSpeed);
        }
        requestSpeed = -1;
    }

    if (requestStart != -1) {
        emit debug(QStringLiteral("starting..."));
        if (lastSpeed == 0.0)
            lastSpeed = 0.5;
        forceSpeed(lastSpeed);
        requestStart = -1;
        emit tapeStarted();
    }

    if (requestStop != -1) {
        emit debug(QStringLiteral("stopping..."));
        forceSpeed(0.0);
        requestStop = -1;
    }
}

void wahookickruntreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void wahookickruntreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                  const QByteArray &newValue) {
    QSettings settings;
    emit debug(QStringLiteral(" << ") + newValue.toHex(' ') + QStringLiteral(" uuid=") +
               characteristic.uuid().toString());

    const int len = newValue.length();

    // --- Command/response channel (same char used for writes) ---
    // FE 86 01 [id0 id1 id2 id3] ... → store device ID, send 87+id
    if (len >= 7 && (uint8_t)newValue.at(0) == 0xFE && (uint8_t)newValue.at(1) == 0x86 &&
        (uint8_t)newValue.at(2) == 0x01 && !deviceIdReceived) {
        deviceId = newValue.mid(3, 4);
        deviceIdReceived = true;
        QByteArray cmd87;
        cmd87.append((char)0x87);
        cmd87.append(deviceId);
        writeCharacteristic(cmd87, QStringLiteral("device id ack"));
        return;
    }

    // FD E0 01 [code_hi] [code_lo] → paddle pushed, treadmill issued challenge
    if (len >= 5 && (uint8_t)newValue.at(0) == 0xFD && (uint8_t)newValue.at(1) == 0xE0 &&
        (uint8_t)newValue.at(2) == 0x01) {
        QByteArray confirm;
        confirm.append((char)0xE0);
        confirm.append(newValue.at(3));
        confirm.append(newValue.at(4));
        writeCharacteristic(confirm, QStringLiteral("challenge response"));
        emit debug(QStringLiteral("KickRun paddle confirmed, echoing challenge code"));
        return;
    }

    // FD E0 02 00 00 → waiting for paddle push (informational, nothing to do)
    if (len >= 3 && (uint8_t)newValue.at(0) == 0xFD && (uint8_t)newValue.at(1) == 0xE0 &&
        (uint8_t)newValue.at(2) == 0x02) {
        emit debug(QStringLiteral("KickRun waiting for paddle push"));
        return;
    }

    // --- Telemetry channel: FF 01 [tgt_b0 b1 b2] 00 ... ---
    if (len >= 5 && (uint8_t)newValue.at(0) == 0xFF && (uint8_t)newValue.at(1) == 0x01) {
        // Target speed (bytes 2-4, LE24, μm/s) — echoes exactly what we commanded
        uint32_t tgt_um_s = static_cast<uint8_t>(newValue.at(2)) |
                            (static_cast<uint8_t>(newValue.at(3)) << 8) |
                            (static_cast<uint8_t>(newValue.at(4)) << 16);
        double tgt_kmh = tgt_um_s / 1e6 * 3.6;

        emit debug(QStringLiteral("KickRun speed=") + QString::number(tgt_kmh, 'f', 2));

        if (Speed.value() != tgt_kmh)
            emit speedChanged(tgt_kmh);
        Speed = tgt_kmh;

        if (tgt_kmh > 0) {
            lastSpeed = tgt_kmh;
        }

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
#endif

        if (!firstCharacteristicChanged) {
            Distance += ((Speed.value() / 3600.0) /
                         (1000.0 / (double)lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

            if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal += ((((0.048 *
                            (double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()) +
                            1.19) *
                           settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                          200.0) /
                         (60000.0 / (double)lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        }

        lastTimeCharacteristicChanged = QDateTime::currentDateTime();
        firstCharacteristicChanged = false;

        cadenceFromAppleWatch();
        return;
    }
}

void wahookickruntreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                  const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void wahookickruntreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor,
                                              const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    // Trigger init after the first CCCD descriptor is written
    if (!initDone) {
        initRequest = true;
    }
}

void wahookickruntreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    // Wait until all services are discovered
    for (QLowEnergyService *s : qAsConst(gattServices)) {
        if (s->state() != QLowEnergyService::ServiceDiscovered &&
            s->state() != QLowEnergyService::InvalidService) {
            return;
        }
    }

    emit debug(QStringLiteral("all services discovered"));

    for (QLowEnergyService *s : qAsConst(gattServices)) {
        if (s->state() != QLowEnergyService::ServiceDiscovered)
            continue;

        connect(s, &QLowEnergyService::characteristicChanged, this,
                &wahookickruntreadmill::characteristicChanged);
        connect(s, &QLowEnergyService::characteristicWritten, this,
                &wahookickruntreadmill::characteristicWritten);
        connect(s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(
                       &QLowEnergyService::error),
                this, &wahookickruntreadmill::errorService);
        connect(s, &QLowEnergyService::descriptorWritten, this,
                &wahookickruntreadmill::descriptorWritten);

        const auto chars = s->characteristics();
        for (const QLowEnergyCharacteristic &c : chars) {
            emit debug(QStringLiteral("char uuid=") + c.uuid().toString() +
                       QStringLiteral(" handle=0x") + QString::number(c.handle(), 16));

            // Command characteristic: UUID a026e03e at handle 0x0023.
            // a026e002 (handle 0x0019) is the Wahoo debug/console stream — it also has
            // WriteNoResponse+Notify so a property-only heuristic always picks it first,
            // causing all init/speed writes to be silently swallowed by the wrong channel.
            static const QBluetoothUuid cmdCharUuid(
                QStringLiteral("a026e03e-0a7d-4ab3-97fa-f1500f9feb8b"));

            if (c.uuid() == cmdCharUuid) {
                gattCmdCharacteristic = c;
                gattCmdService = s;
                emit debug(QStringLiteral("KickRun cmd char found: handle=0x") +
                           QString::number(c.handle(), 16));
            }

            // Subscribe to all notify/indicate characteristics
            if (hasNotify) {
                QByteArray descriptor;
                descriptor.append((char)0x01);
                descriptor.append((char)0x00);
                if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                    s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration),
                                       descriptor);
                    notificationsSubscribed++;
                    emit debug(QStringLiteral("subscribed notify: ") + c.uuid().toString());
                }
            } else if (c.properties() & QLowEnergyCharacteristic::Indicate) {
                QByteArray descriptor;
                descriptor.append((char)0x02);
                descriptor.append((char)0x00);
                if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                    s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration),
                                       descriptor);
                    notificationsSubscribed++;
                    emit debug(QStringLiteral("subscribed indicate: ") + c.uuid().toString());
                }
            }
        }
    }
}

void wahookickruntreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    const auto services = m_control->services();
    for (const QBluetoothUuid &uuid : services) {
        auto *s = m_control->createServiceObject(uuid);
        gattServices.append(s);
        connect(s, &QLowEnergyService::stateChanged, this, &wahookickruntreadmill::stateChanged);
        s->discoverDetails();
    }
}

void wahookickruntreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("wahookickruntreadmill::errorService ") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void wahookickruntreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("wahookickruntreadmill::error ") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void wahookickruntreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    bluetoothDevice = device;
    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);

    connect(m_control, &QLowEnergyController::serviceDiscovered, this,
            &wahookickruntreadmill::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this,
            &wahookickruntreadmill::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(
                &QLowEnergyController::error),
            this, &wahookickruntreadmill::error);
    connect(m_control, &QLowEnergyController::stateChanged, this,
            &wahookickruntreadmill::controllerStateChanged);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(
                &QLowEnergyController::error),
            this, [this](QLowEnergyController::Error) {
                emit debug(QStringLiteral("Cannot connect to remote device."));
                emit disconnected();
            });
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        emit debug(QStringLiteral("Controller connected. Discovering services..."));
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        emit debug(QStringLiteral("LowEnergy controller disconnected"));
        emit disconnected();
    });

    m_control->connectToDevice();
}

void wahookickruntreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to reconnect...");
        initDone = false;
        firstSpeedSent = false;
        workoutModeStarted = false;
        deviceIdReceived = false;
        gattCmdCharacteristic = QLowEnergyCharacteristic();
        gattCmdService = nullptr;
        gattTelService = nullptr;
        qDeleteAll(gattServices);
        gattServices.clear();
        m_control->connectToDevice();
    }
}

bool wahookickruntreadmill::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool wahookickruntreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    return false;
}

bool wahookickruntreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    return false;
}
