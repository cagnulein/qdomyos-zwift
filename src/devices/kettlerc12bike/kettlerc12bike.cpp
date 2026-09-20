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
        const bool canWriteWithoutResponse =
            (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse);
        const QLowEnergyService::WriteMode writeMode =
            canWriteWithoutResponse ? QLowEnergyService::WriteWithoutResponse : QLowEnergyService::WriteWithResponse;

        if (wait_for_response || writeMode == QLowEnergyService::WriteWithResponse) {
            connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop,
                    &QEventLoop::quit);
            timeout.singleShot(300ms, &loop, &QEventLoop::quit);
        }

        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer, writeMode);
    }

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info;
    }

    if (timeout.isActive()) {
        loop.exec();

        if (timeout.isActive() == false) {
            qDebug() << QStringLiteral(" exit for timeout");
        }
    }
}

void kettlerc12bike::btinit() {
    qDebug() << QStringLiteral("kettlerc12bike::btinit");

    // Startup sequence observed in the Kinomap bugreport btsnoop. The longer
    // app/session metadata packets are intentionally not mirrored here.
    uint8_t init0[] = {0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00};
    writeCharacteristic(init0, sizeof(init0), QStringLiteral("kinomap_init0"), false, false);

    uint8_t init1[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x04, 0x00, 0x00};
    writeCharacteristic(init1, sizeof(init1), QStringLiteral("kinomap_init1"), false, false);

    uint8_t setup0[] = {0x25, 0x00};
    uint8_t setup1[] = {0x25, 0x01};
    uint8_t setup2[] = {0x25, 0x02};
    uint8_t setup3[] = {0x25, 0x03};
    uint8_t setup4[] = {0x25, 0x04};
    uint8_t setup5[] = {0x25, 0x05, 0x02, 0x00};
    writeCharacteristic(setup0, sizeof(setup0), QStringLiteral("kinomap_setup0"), false, false);
    writeCharacteristic(setup1, sizeof(setup1), QStringLiteral("kinomap_setup1"), false, false);
    writeCharacteristic(setup2, sizeof(setup2), QStringLiteral("kinomap_setup2"), false, false);
    writeCharacteristic(setup3, sizeof(setup3), QStringLiteral("kinomap_setup3"), false, false);
    writeCharacteristic(setup4, sizeof(setup4), QStringLiteral("kinomap_setup4"), false, false);
    writeCharacteristic(setup5, sizeof(setup5), QStringLiteral("kinomap_setup5"), false, false);

    uint8_t init2[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x01, 0x00, 0x02};
    uint8_t init3[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x16, 0x00, 0x00};
    writeCharacteristic(init2, sizeof(init2), QStringLiteral("kinomap_init2"), false, false);
    writeCharacteristic(init3, sizeof(init3), QStringLiteral("kinomap_init3"), false, false);

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

    const uint8_t wattsLow = targetWatts & 0xff;
    const uint8_t wattsHigh = (targetWatts >> 8) & 0xff;

    uint8_t powercmd[] = {0x81, kettlerCommandSequence++, 0x00, 0x06, 0x84,
                          0x12, 0x01, 0x0c, wattsLow, wattsHigh};
    writeCharacteristic(powercmd, sizeof(powercmd),
                        QStringLiteral("kinomap_target_power_") + QString::number(targetWatts), false, true);
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
        } else if (uuid == KETTLER_RESISTANCE_CHAR_UUID || handle == 0x38) {
            Resistance = value;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
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
        connect(service,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &kettlerc12bike::errorService, Qt::UniqueConnection);
        connect(service, &QLowEnergyService::descriptorWritten, this, &kettlerc12bike::descriptorWritten,
                Qt::UniqueConnection);

        const bool customService = service->serviceUuid() == activeServiceUuid;

        if (customService) {
            qDebug() << QStringLiteral("Kettler C12 custom service discovered") << activeServiceUuid.toString();

            gattWriteCharacteristic = QLowEnergyCharacteristic();
            gattHandshakeCharacteristic = QLowEnergyCharacteristic();
        }

        foreach (QLowEnergyCharacteristic c, service->characteristics()) {
            qDebug() << QStringLiteral("characteristic") << c.uuid()
                     << "service:" << service->serviceUuid()
                     << "handle:" << QString::number(c.handle(), 16)
                     << "properties:" << c.properties();

            if (customService) {
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

    if (!connectedAndDiscoveredEmitted) {
        connectedAndDiscoveredEmitted = true;
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
