#include "kettlerc12bike.h"
#include "kettlerc12protocol.h"
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

namespace {
uint16_t readUInt16LE(const QByteArray &value, int index) {
    return ((uint16_t)((uint8_t)value.at(index + 1)) << 8) | (uint16_t)((uint8_t)value.at(index));
}

uint32_t readUInt32LE(const QByteArray &value, int index) {
    return ((uint32_t)((uint8_t)value.at(index + 3)) << 24) |
           ((uint32_t)((uint8_t)value.at(index + 2)) << 16) |
           ((uint32_t)((uint8_t)value.at(index + 1)) << 8) |
           (uint32_t)((uint8_t)value.at(index));
}
}

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
    ergModeSupported = true; // C12 accepts direct target watt commands
    connect(refresh, &QTimer::timeout, this, &kettlerc12bike::update);
    refresh->start(500ms);
}

kettlerc12bike::~kettlerc12bike() {
    qDebug() << QStringLiteral("~kettlerc12bike()");
}

bool kettlerc12bike::setNativeResistanceLevel(int level) {
    if (noWriteResistance || !controlReady || !gattCommunicationChannelService ||
        !gattResistanceCharacteristic.isValid() ||
        gattCommunicationChannelService->state() != QLowEnergyService::ServiceDiscovered) {
        return false;
    }

    // Kinomap sends a single native resistance-level byte to 638a1001
    // (ATT 0x38); watt targets use a different characteristic (ATT 0x42).
    const uint8_t nativeLevel = KettlerC12Protocol::encodeResistanceLevel(level);
    const QByteArray payload(1, static_cast<char>(nativeLevel));
    gattCommunicationChannelService->writeCharacteristic(
        gattResistanceCharacteristic, payload, QLowEnergyService::WriteWithResponse);
    qDebug() << QStringLiteral("Kettler C12 native resistance") << nativeLevel
             << QStringLiteral("->") << payload.toHex(' ');
    return true;
}

void kettlerc12bike::btinit() {
    qDebug() << QStringLiteral("kettlerc12bike::btinit - starting observed Kinomap handshake");
    initDone = true; // Keep collecting metrics even if authentication is not confirmed.
    challengeReadPending = false;
    handshakeWritePending = false;
    controlReady = false;
    initialResistanceSent = false;

    if (noWriteResistance) {
        qDebug() << QStringLiteral("C12 control disabled by No Write Resistance setting");
        return;
    }

    if (!gattCommunicationChannelService || !gattChallengeCharacteristic.isValid() ||
        !gattHandshakeCharacteristic.isValid() ||
        !(gattChallengeCharacteristic.properties() & QLowEnergyCharacteristic::Read) ||
        !(gattHandshakeCharacteristic.properties() & QLowEnergyCharacteristic::Write)) {
        qDebug() << QStringLiteral("C12 Kinomap handshake characteristics unavailable");
        return;
    }

    // Kinomap reads 638a1104 (ATT 0x30), then writes an authenticated 16-byte
    // response to 638a1105 (ATT 0x32) before controlling the bike.
    challengeReadPending = true;
    gattCommunicationChannelService->readCharacteristic(gattChallengeCharacteristic);
}

void kettlerc12bike::forceResistance(resistance_t requestResistance) {
    // QZ resistance requests are treated as watt targets, as in the existing
    // C12 driver. The separate 1-byte Kinomap writes to handle 0x0038 are
    // physical resistance levels and must not be confused with watt targets.
    targetPowerWatts = qBound(0, static_cast<int>(requestResistance),
                              KettlerC12Protocol::maxTargetPowerWatts);
    targetPowerActive = true;
    lastTargetPowerWrite.invalidate();
}

bool kettlerc12bike::setTargetPower(uint16_t targetWatts) {
    if (noWriteResistance || !controlReady || !gattCommunicationChannelService ||
        !gattTargetPowerCharacteristic.isValid() ||
        gattCommunicationChannelService->state() != QLowEnergyService::ServiceDiscovered) {
        return false;
    }

    // Kinomap uses a *plain 2-byte little-endian watt value* on ATT handle
    // 0x0042 (e.g. 19 00 = 25 W; 32 00 = 50 W). It does not send the
    // 10-byte 81/... control frame previously used here.
    const auto bytes = KettlerC12Protocol::encodeTargetPower(targetWatts);
    const QByteArray payload(reinterpret_cast<const char *>(bytes.data()),
                             static_cast<int>(bytes.size()));
    const auto mode = (gattTargetPowerCharacteristic.properties() & QLowEnergyCharacteristic::Write)
                          ? QLowEnergyService::WriteWithResponse
                          : QLowEnergyService::WriteWithoutResponse;
    gattCommunicationChannelService->writeCharacteristic(gattTargetPowerCharacteristic, payload, mode);
    qDebug() << QStringLiteral("Kettler C12 target power") << targetWatts
             << QStringLiteral("W ->") << payload.toHex(' ')
             << QStringLiteral("characteristic") << gattTargetPowerCharacteristic.uuid();
    return true;
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
               initDone) {

        update_metrics(true, watts());

        // Consume QZ Target Watt tile, workout and virtual FTMS ERG commands.
        // Power takes precedence over manual resistance requests.
        if (!noWriteResistance) {
            if (requestPower >= 0) {
                forceResistance(requestPower);
                emit debug(QStringLiteral("Kettler C12 requested watt target: ") +
                           QString::number(targetPowerWatts));
            } else if (requestResistance != -1) {
                // Manual resistance controls the C12 native 1..20 level, not watts.
                pendingNativeResistance = KettlerC12Protocol::encodeResistanceLevel(requestResistance);
                targetPowerActive = false;
                emit debug(QStringLiteral("Kettler C12 requested native resistance: ") +
                           QString::number(pendingNativeResistance));
            }
        }
        requestPower = -1;
        requestResistance = -1;

        if (requestStop != -1) {
            targetPowerActive = false;
            requestStop = -1;
        }

        if (controlReady && pendingNativeResistance != -1) {
            if (setNativeResistanceLevel(pendingNativeResistance)) {
                pendingNativeResistance = -1;
            }
        }

        // Kinomap first sets native resistance to level 1 when a watt-target
        // session starts, then repeats the power target about once a second.
        // Send these writes on separate refresh ticks.
        if (targetPowerActive && controlReady && !initialResistanceSent) {
            initialResistanceSent = setNativeResistanceLevel(1);
        } else if (targetPowerActive && initialResistanceSent &&
                   (!lastTargetPowerWrite.isValid() || lastTargetPowerWrite.elapsed() >= 1000)) {
            if (setTargetPower(targetPowerWatts)) {
                lastTargetPowerWrite.start();
            }
        }

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
        if (gattCommunicationChannelService) {
            connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
                    &kettlerc12bike::stateChanged);
            gattCommunicationChannelService->discoverDetails();
        }
    } else {
        qDebug() << QStringLiteral("Kettler C12 custom service not found!") << services;
    }

    if (services.contains(CYCLING_SPEED_CADENCE_SERVICE_UUID)) {
        auto cscService = m_control->createServiceObject(CYCLING_SPEED_CADENCE_SERVICE_UUID, this);
        if (cscService) {
            gattMetricServices.append(cscService);
            connect(cscService, &QLowEnergyService::stateChanged, this, &kettlerc12bike::stateChanged);
            cscService->discoverDetails();
        }
    }

    if (services.contains(CYCLING_POWER_SERVICE_UUID)) {
        auto powerService = m_control->createServiceObject(CYCLING_POWER_SERVICE_UUID, this);
        if (powerService) {
            gattMetricServices.append(powerService);
            connect(powerService, &QLowEnergyService::stateChanged, this, &kettlerc12bike::stateChanged);
            powerService->discoverDetails();
        }
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

    if (characteristic.uuid() == CYCLING_POWER_MEASUREMENT_CHAR_UUID && newValue.length() >= 4) {
        const uint16_t flags = readUInt16LE(newValue, 0);
        int index = 4;
        m_watt = (int16_t)readUInt16LE(newValue, 2);
        lastCyclingPower = now;
        emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));

        if (flags & 0x01) {
            index += 1; // Pedal Power Balance Present
        }
        if (flags & 0x04) {
            index += 2; // Accumulated Torque Present
        }
        if (flags & 0x10) {
            index += 6; // Wheel Revolution Data Present
        }
        if ((flags & 0x20) && newValue.length() >= index + 4) {
            CrankRevs = readUInt16LE(newValue, index);
            index += 2;
            LastCrankEventTime = readUInt16LE(newValue, index);

            int32_t deltaT = LastCrankEventTime - oldLastCrankEventTime;
            if (deltaT < 0) {
                deltaT = LastCrankEventTime + 65536 - oldLastCrankEventTime;
            }

            int32_t crankDelta = CrankRevs - oldCrankRevs;
            if (crankDelta < 0 && crankDelta > -100) {
                crankDelta += 65536;
            }

            if (crankDelta > 0 && deltaT > 0) {
                const double cadence = (crankDelta / (double)deltaT) * 1024.0 * 60.0;
                if (cadence >= 0 && cadence < 255) {
                    Cadence = cadence;
                    lastGoodCadence = now;
                    emit cadenceChanged(Cadence.value());
                    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
                }
            } else if (lastGoodCadence.msecsTo(now) > 2000) {
                Cadence = 0;
            }

            oldLastCrankEventTime = LastCrankEventTime;
            oldCrankRevs = CrankRevs;
        }
    } else if (characteristic.uuid() == CSC_MEASUREMENT_CHAR_UUID && newValue.length() >= 1) {
        const bool wheelPresent = (newValue.at(0) & 0x01) == 0x01;
        const bool crankPresent = (newValue.at(0) & 0x02) == 0x02;
        int index = 1;
        double wheelRevs = 0;
        uint16_t lastWheelEventTime = 0;
        double crankRevs = 0;
        uint16_t lastCrankEventTime = 0;

        if (wheelPresent && newValue.length() >= index + 6) {
            wheelRevs = readUInt32LE(newValue, index);
            index += 4;
            lastWheelEventTime = readUInt16LE(newValue, index);
            index += 2;
        }

        if (crankPresent && newValue.length() >= index + 4) {
            crankRevs = readUInt16LE(newValue, index);
            index += 2;
            lastCrankEventTime = readUInt16LE(newValue, index);
        }

        if ((!crankPresent || crankRevs == 0) && wheelPresent) {
            CrankRevs = wheelRevs;
            LastCrankEventTime = lastWheelEventTime;
        } else if (crankPresent) {
            CrankRevs = crankRevs;
            LastCrankEventTime = lastCrankEventTime;
        }

        int32_t deltaT = LastCrankEventTime - oldLastCrankEventTime;
        if (deltaT < 0) {
            deltaT = LastCrankEventTime + 65536 - oldLastCrankEventTime;
        }

        if (CrankRevs != oldCrankRevs && deltaT) {
            const double cadence = ((CrankRevs - oldCrankRevs) / (double)deltaT) * 1024.0 * 60.0;
            if ((cadence >= 0 && (cadence < 256 || crankRevs == 0) && crankPresent) ||
                (!crankPresent && wheelPresent)) {
                Cadence = cadence;
                lastGoodCadence = now;
            }
        } else if (lastGoodCadence.msecsTo(now) > 2000) {
            Cadence = 0;
        }

        oldLastCrankEventTime = LastCrankEventTime;
        oldCrankRevs = CrankRevs;
        emit cadenceChanged(Cadence.value());
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    } else if ((characteristic.uuid() == KETTLER_RESISTANCE_CHAR_UUID || handle == 0x38) &&
               newValue.length() >= 1) {
        // The native resistance-level characteristic sends a single byte.
        Resistance = static_cast<uint8_t>(newValue.at(0));
        emit resistanceRead(Resistance.value());
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    } else if (newValue.length() >= 2) {
        const uint16_t value = readUInt16LE(newValue, 0);
        const QBluetoothUuid uuid = characteristic.uuid();

        if (uuid == KETTLER_POWER_CHAR_UUID || handle == 0x3e) {
            const bool hasRecentCyclingPower =
                lastCyclingPower.isValid() && lastCyclingPower.msecsTo(now) < 3000;

            if (!hasRecentCyclingPower) {
                m_watt = value;
                emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
            }
        } else if (uuid == KETTLER_SPEED_CHAR_UUID || handle == 0x48) {
            Speed = value / 10.0;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        } else if (uuid == QBluetoothUuid(QStringLiteral("638a1005-7bde-3e25-ffc5-9de9b2a0197a")) ||
                   uuid == QBluetoothUuid(QStringLiteral("638a100a-7bde-3e25-ffc5-9de9b2a0197a")) ||
                   handle == 0x4c || handle == 0x63) {
            if (value <= max_resistance) {
                Resistance = value;
                emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
            }
        }
    }

    if ((characteristic.uuid() == KETTLER_DISTANCE_CHAR_UUID || handle == 0x5c) && newValue.length() >= 4) {
        const uint32_t distanceRaw = readUInt32LE(newValue, 0);
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
        if (!service) {
            return;
        }

        connect(service, &QLowEnergyService::characteristicChanged, this, &kettlerc12bike::characteristicChanged,
                Qt::UniqueConnection);
        connect(service, &QLowEnergyService::characteristicWritten, this, &kettlerc12bike::characteristicWritten,
                Qt::UniqueConnection);
        connect(service, &QLowEnergyService::characteristicRead, this, &kettlerc12bike::characteristicRead,
                Qt::UniqueConnection);
        connect(service,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &kettlerc12bike::errorService, Qt::UniqueConnection);
        connect(service, &QLowEnergyService::descriptorWritten, this, &kettlerc12bike::descriptorWritten,
                Qt::UniqueConnection);

        const bool customService = service->serviceUuid() == activeServiceUuid;

        if (customService) {
            qDebug() << QStringLiteral("Kettler C12 custom service discovered") << activeServiceUuid.toString();

            gattWriteCharacteristic = QLowEnergyCharacteristic();
            gattChallengeCharacteristic = QLowEnergyCharacteristic();
            gattHandshakeCharacteristic = QLowEnergyCharacteristic();
            gattResistanceCharacteristic = QLowEnergyCharacteristic();
            gattTargetPowerCharacteristic = QLowEnergyCharacteristic();
        }

        foreach (QLowEnergyCharacteristic c, service->characteristics()) {
            qDebug() << QStringLiteral("characteristic") << c.uuid()
                     << "service:" << service->serviceUuid()
                     << "handle:" << QString::number(c.handle(), 16)
                     << "properties:" << c.properties();

            if (customService) {
                // The capture begins after Android's cached GATT discovery, so
                // the UUID for 0x0042 is not present in the snoop. Match the
                // observed ATT value handle on this C12 firmware, but never
                // attempt to write unless the characteristic is writable.
                if (c.handle() == KettlerC12Protocol::targetPowerHandle &&
                    (c.properties() & (QLowEnergyCharacteristic::Write |
                                       QLowEnergyCharacteristic::WriteNoResponse))) {
                    gattTargetPowerCharacteristic = c;
                    qDebug() << QStringLiteral("Found C12 watt target characteristic")
                             << c.uuid() << QStringLiteral("handle:") << QString::number(c.handle(), 16);
                }

                if (c.uuid() == KETTLER_CONTROL_CHAR_UUID) {
                    gattWriteCharacteristic = c;
                    qDebug() << QStringLiteral("Found Kettler control characteristic") << c.uuid()
                             << "handle:" << QString::number(c.handle(), 16);
                } else if (c.uuid() == KETTLER_CHALLENGE_CHAR_UUID) {
                    gattChallengeCharacteristic = c;
                } else if (c.uuid() == KETTLER_HANDSHAKE_CHAR_UUID) {
                    gattHandshakeCharacteristic = c;
                } else if (c.uuid() == KETTLER_RESISTANCE_CHAR_UUID) {
                    gattResistanceCharacteristic = c;
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
            }

            if ((c.properties() & QLowEnergyCharacteristic::Notify) ||
                (c.properties() & QLowEnergyCharacteristic::Indicate)) {
                qDebug() << QStringLiteral("Enabling notifications on characteristic") << c.uuid()
                         << "handle:" << QString::number(c.handle(), 16);

                QByteArray descriptor;
                if (c.properties() & QLowEnergyCharacteristic::Notify) {
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                } else {
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                }

                const auto cccd = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (cccd.isValid()) {
                    service->writeDescriptor(cccd, descriptor);
                }
            }
        }

        if (customService) {
            if (!gattTargetPowerCharacteristic.isValid()) {
                qDebug() << QStringLiteral("C12 watt target characteristic not found (expected ATT 0x0042)");
            }
            qDebug() << QStringLiteral("Starting C12 Kinomap initialization...");
            initRequest = true;
        }
    }
}

void kettlerc12bike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") << descriptor.name() << descriptor.uuid().toString()
             << newValue.toHex(' ');

    if (!connectedAndDiscoveredEmitted) {
        connectedAndDiscoveredEmitted = true;
        emit connectedAndDiscovered();
    }
}

void kettlerc12bike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    if (characteristic.uuid() != KETTLER_CHALLENGE_CHAR_UUID || !challengeReadPending ||
        sender() != gattCommunicationChannelService) {
        return;
    }

    challengeReadPending = false;
    qDebug() << QStringLiteral("C12 handshake challenge read") << newValue.toHex(' ');

    const auto *bytes = reinterpret_cast<const uint8_t *>(newValue.constData());
    if (!KettlerC12Protocol::matchesCapturedChallenge(bytes, static_cast<size_t>(newValue.size()))) {
        // Only one challenge/response pair was captured from Kinomap.
        // Do not send a stale response when the challenge changes.
        qDebug() << QStringLiteral("C12 authentication needs another challenge/response capture; watt control disabled for this session");
        emit debug(QStringLiteral("C12 authentication challenge differs from recorded Kinomap session"));
        return;
    }

    if (!gattHandshakeCharacteristic.isValid() || !gattCommunicationChannelService) {
        qDebug() << QStringLiteral("C12 handshake response characteristic unavailable");
        return;
    }

    const auto bytesToSend = KettlerC12Protocol::capturedHandshakeResponse();
    const QByteArray response(reinterpret_cast<const char *>(bytesToSend.data()),
                              static_cast<int>(bytesToSend.size()));
    handshakeWritePending = true;
    qDebug() << QStringLiteral("C12 handshake sending response on 638a1105");
    gattCommunicationChannelService->writeCharacteristic(
        gattHandshakeCharacteristic, response, QLowEnergyService::WriteWithResponse);
}

void kettlerc12bike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicWritten") << characteristic.uuid() << newValue.toHex(' ');
    if (characteristic.uuid() == KETTLER_HANDSHAKE_CHAR_UUID && handshakeWritePending) {
        const auto expected = KettlerC12Protocol::capturedHandshakeResponse();
        const QByteArray response(reinterpret_cast<const char *>(expected.data()),
                                  static_cast<int>(expected.size()));
        if (newValue == response) {
            handshakeWritePending = false;
            controlReady = true;
            qDebug() << QStringLiteral("C12 handshake write acknowledged (acceptance by bike unverified)");
        }
    }
}

void kettlerc12bike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;

    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        initRequest = false;
        challengeReadPending = false;
        handshakeWritePending = false;
        controlReady = false;
        initialResistanceSent = false;
        pendingNativeResistance = -1;
        targetPowerActive = false;
        lastTargetPowerWrite.invalidate();
        gattTargetPowerCharacteristic = QLowEnergyCharacteristic();
        connectBackLater();
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
