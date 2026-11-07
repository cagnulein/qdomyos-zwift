#include "urevotreadmill.h"

#include "qzsettings.h"
#include "virtualdevices/virtualtreadmill.h"

#include <QDateTime>
#include <QSettings>

urevotreadmill::urevotreadmill(bool noHeartService) : m_noHeartService(noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    m_refresh = new QTimer(this);
    connect(m_refresh, &QTimer::timeout, this, &urevotreadmill::update);
    m_refresh->start(200);
}

void urevotreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    emit debug(QStringLiteral("UREVO URTM047 found: ") + device.name());
    m_control = QLowEnergyController::createCentral(device, this);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &urevotreadmill::serviceScanDone);
    connect(m_control, &QLowEnergyController::stateChanged, this, &urevotreadmill::controllerStateChanged);
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        emit debug(QStringLiteral("UREVO connected: discovering services"));
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        emit debug(QStringLiteral("UREVO disconnected"));
        emit disconnected();
    });
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, [this](QLowEnergyController::Error error) {
                emit debug(QStringLiteral("UREVO controller error %1: %2")
                               .arg(int(error)).arg(m_control->errorString()));
            });
    m_control->connectToDevice();
}

void urevotreadmill::serviceScanDone() {
    if (!m_control)
        return;
    const auto services = m_control->services();
    const QBluetoothUuid nativeId(quint16(0xfff0));
    const QBluetoothUuid auxiliaryId(quint16(0xfee0));
    const QBluetoothUuid ftmsId(quint16(0x1826));
    if (!services.contains(nativeId) || !services.contains(auxiliaryId)) {
        emit debug(QStringLiteral("UREVO missing FFF0 or FEE0: native control disabled"));
        return;
    }
    m_mainService = m_control->createServiceObject(nativeId, this);
    m_auxService = m_control->createServiceObject(auxiliaryId, this);
    if (services.contains(ftmsId))
        m_ftmsService = m_control->createServiceObject(ftmsId, this);
    for (QLowEnergyService *service : {m_mainService, m_auxService, m_ftmsService}) {
        if (!service)
            continue;
        connect(service, &QLowEnergyService::stateChanged, this, &urevotreadmill::serviceStateChanged);
        connect(service, &QLowEnergyService::characteristicChanged, this, &urevotreadmill::characteristicChanged);
        connect(service, &QLowEnergyService::characteristicWritten, this, &urevotreadmill::characteristicWritten);
        connect(service, &QLowEnergyService::descriptorWritten, this, &urevotreadmill::descriptorWritten);
        connect(service,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, [this](QLowEnergyService::ServiceError error) {
                    emit debug(QStringLiteral("UREVO service error %1").arg(int(error)));
                    if (error == QLowEnergyService::CharacteristicWriteError) {
                        m_protocol.fail(QDateTime::currentMSecsSinceEpoch());
                        m_writeQueue.clear();
                        m_writeInFlight = false;
                    }
                });
        service->discoverDetails();
    }
}

void urevotreadmill::serviceStateChanged(QLowEnergyService::ServiceState state) {
    Q_UNUSED(state);
    tryConfigure();
}

void urevotreadmill::tryConfigure() {
    if (m_configured || !m_mainService || !m_auxService ||
        m_mainService->state() != QLowEnergyService::ServiceDiscovered ||
        m_auxService->state() != QLowEnergyService::ServiceDiscovered ||
        (m_ftmsService && m_ftmsService->state() != QLowEnergyService::ServiceDiscovered))
        return;

    m_mainNotify = m_mainService->characteristic(QBluetoothUuid(quint16(0xfff1)));
    m_mainWrite = m_mainService->characteristic(QBluetoothUuid(quint16(0xfff2)));
    m_auxNotify = m_auxService->characteristic(QBluetoothUuid(quint16(0xfee1)));
    m_auxWrite = m_auxService->characteristic(QBluetoothUuid(quint16(0xfee2)));
    m_mainCccd = m_mainNotify.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    m_auxCccd = m_auxNotify.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!m_mainWrite.isValid() || !m_auxWrite.isValid() || !m_mainNotify.isValid() ||
        !m_auxNotify.isValid() || !m_mainCccd.isValid() || !m_auxCccd.isValid() ||
        !(m_mainNotify.properties() & QLowEnergyCharacteristic::Notify) ||
        !(m_auxNotify.properties() & QLowEnergyCharacteristic::Notify) ||
        !(m_mainWrite.properties() & QLowEnergyCharacteristic::Write) ||
        !(m_auxWrite.properties() & QLowEnergyCharacteristic::Write)) {
        emit debug(QStringLiteral("UREVO native GATT characteristic or CCCD missing"));
        return;
    }
    if (m_ftmsService)
        m_ftmsData = m_ftmsService->characteristic(QBluetoothUuid(quint16(0x2acd)));
    m_configured = true;
    emit debug(QStringLiteral("UREVO FFF1/FFF2 and FEE1/FEE2 discovered; enabling FFF1 notifications"));
    m_mainService->writeDescriptor(m_mainCccd, QByteArray::fromHex("0100"));
}

void urevotreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
    if (value != QByteArray::fromHex("0100"))
        return;
    if (descriptor == m_mainCccd && !m_mainSubscribed) {
        m_mainSubscribed = true;
        emit debug(QStringLiteral("UREVO FFF1 notifications enabled; enabling FEE1"));
        m_auxService->writeDescriptor(m_auxCccd, QByteArray::fromHex("0100"));
    } else if (descriptor == m_auxCccd && !m_auxSubscribed) {
        m_auxSubscribed = true;
        emit debug(QStringLiteral("UREVO FEE1 notifications enabled; starting recorded handshake"));
        m_protocol.subscribed(QDateTime::currentMSecsSinceEpoch());
        collectProtocolCommands();

        if (!m_connectedSignalSent) {
            m_connectedSignalSent = true;
            emit connectedAndDiscovered();
            createVirtualTreadmill();
        }

        if (m_ftmsData.isValid() && (m_ftmsData.properties() & QLowEnergyCharacteristic::Notify)) {
            const auto cccd = m_ftmsData.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (cccd.isValid())
                m_ftmsService->writeDescriptor(cccd, QByteArray::fromHex("0100"));
        }
    }
}

void urevotreadmill::createVirtualTreadmill() {
    if (m_firstVirtual || hasVirtualDevice())
        return;
    m_firstVirtual = true;
    QSettings settings;
    if (!settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool())
        return;
    auto *virtualTreadmill = new virtualtreadmill(this, m_noHeartService);
    connect(virtualTreadmill, &virtualtreadmill::debug, this, &urevotreadmill::debug);
    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
            [this](double grade, double percentage) {
                changeInclination(grade, percentage < 0 ? 0 : percentage);
            });
    setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
}

void urevotreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                          const QByteArray &value) {
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (sender() == m_mainService && characteristic.uuid() == QBluetoothUuid(quint16(0xfff1))) {
        emit debug(QStringLiteral("UREVO FFF1 << ") + QString::fromLatin1(value.toHex(' ')));
        double speed = 0, incline = 0;
        const bool full = UrevoProtocol::decodeFull(value, speed, incline);
        m_protocol.receiveMain(value, now);
        if (full)
            handleFullStatus(now);
        collectProtocolCommands();
    } else if (sender() == m_auxService && characteristic.uuid() == QBluetoothUuid(quint16(0xfee1))) {
        emit debug(QStringLiteral("UREVO FEE1 << ") + QString::fromLatin1(value.toHex(' ')));
        m_protocol.receiveAuxiliary(value, now);
        collectProtocolCommands();
    } else if (sender() == m_ftmsService && characteristic.uuid() == QBluetoothUuid(quint16(0x2acd))) {
        // Read-only FTMS telemetry fallback; never use its control point.
        // Bit 0 clear means instantaneous speed is present at bytes 2-3 (0.01 km/h).
        if (m_lastFullAt == 0 || now - m_lastFullAt > 5000) {
            if (value.size() >= 4 && !(quint8(value[0]) & 1)) {
                const int speed100 = quint8(value[2]) | (int(quint8(value[3])) << 8);
                parseSpeed(speed100 / 100.0);
            }
        }
    }
}

void urevotreadmill::handleFullStatus(qint64 now) {
    const double speed = m_protocol.speedKmh();
    if (m_lastFullAt > 0 && now >= m_lastFullAt && now - m_lastFullAt < 5000)
        Distance += speed * double(now - m_lastFullAt) / 3600000.0;
    m_lastFullAt = now;
    parseSpeed(speed);
    parseInclination(treadmillInclinationOverride(m_protocol.inclination()));
    if (speed > 0) {
        lastStart = 0;
        lastSpeed = speed;
    } else {
        lastStop = 0;
    }
}

void urevotreadmill::collectProtocolCommands() {
    const auto cmds = m_protocol.takeCommands();
    for (const auto &cmd : cmds)
        m_writeQueue.enqueue(cmd);
    flushWriteQueue();
}

void urevotreadmill::flushWriteQueue() {
    if (m_writeInFlight || m_writeQueue.isEmpty())
        return;
    const auto cmd = m_writeQueue.dequeue();
    QLowEnergyService *service = cmd.channel == UrevoProtocol::Channel::Main ? m_mainService : m_auxService;
    const QLowEnergyCharacteristic characteristic =
        cmd.channel == UrevoProtocol::Channel::Main ? m_mainWrite : m_auxWrite;
    if (!service || !characteristic.isValid() || !m_control ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("UREVO cannot write: BLE channel unavailable"));
        m_writeQueue.clear();
        m_protocol.fail(QDateTime::currentMSecsSinceEpoch());
        return;
    }
    emit debug(QStringLiteral("UREVO %1 >> %2")
                   .arg(cmd.channel == UrevoProtocol::Channel::Main ? QStringLiteral("FFF2") : QStringLiteral("FEE2"))
                   .arg(QString::fromLatin1(cmd.payload.toHex(' '))));
    m_writeInFlight = true;
    m_writeAt = QDateTime::currentMSecsSinceEpoch();
    m_inFlightStop = cmd.payload == UrevoProtocol::stopFrame(false) ||
                     cmd.payload == UrevoProtocol::stopFrame(true);
    service->writeCharacteristic(characteristic, cmd.payload, QLowEnergyService::WriteWithResponse);
}

void urevotreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                           const QByteArray &value) {
    if (!m_writeInFlight)
        return;
    if ((sender() != m_mainService || characteristic != m_mainWrite) &&
        (sender() != m_auxService || characteristic != m_auxWrite))
        return;
    emit debug(QStringLiteral("UREVO BLE write confirmed: ") + QString::fromLatin1(value.toHex(' ')));
    m_writeInFlight = false;
    m_inFlightStop = false;
    flushWriteQueue();
}

void urevotreadmill::update() {
    if (!m_control || m_control->state() == QLowEnergyController::UnconnectedState)
        return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    if (m_writeInFlight && now - m_writeAt > 1800) {
        emit debug(QStringLiteral("UREVO BLE write response timed out; control suspended"));
        const bool stopPending = !m_writeQueue.isEmpty() &&
            (m_writeQueue.head().payload == UrevoProtocol::stopFrame(false) ||
             m_writeQueue.head().payload == UrevoProtocol::stopFrame(true));
        const auto stopCommand = stopPending ? m_writeQueue.head() : UrevoProtocol::Command{};
        m_writeInFlight = false;
        m_writeQueue.clear();
        m_protocol.fail(now);
        if (stopPending)
            m_writeQueue.enqueue(stopCommand); // best-effort Stop takes priority
    }

    if (requestStop != -1) {
        const bool pause = requestPause != -1;
        requestStop = requestPause = requestStart = -1;
        requestSpeed = -1;
        requestInclination = -100;
        m_pendingSpeed = -1;
        m_pendingInclination = -100;
        m_pendingTargetAt = m_lastStartRequestAt = 0;
        m_writeQueue.clear(); // Cancel queued (not in-flight) start/target.
        if (m_mainWrite.isValid() && m_mainSubscribed) {
            m_protocol.requestStop(pause, now);
            emit debug(QStringLiteral("UREVO stop requested; actual belt stop unverified"));
        } else {
            emit debug(QStringLiteral("UREVO stop unavailable before FFF1 subscription"));
        }
    } else {
        if (requestStart != -1) {
            requestStart = -1;
            m_lastStartRequestAt = now;
        }
        if (m_lastStartRequestAt) {
            if (m_protocol.requestStart(now)) {
                emit debug(QStringLiteral("UREVO Start requested: replaying official handshake"));
                m_lastStartRequestAt = 0;
            } else if (now - m_lastStartRequestAt >= 5000 || m_protocol.running() ||
                       m_protocol.state() == UrevoProtocol::State::Failed) {
                emit debug(QStringLiteral("UREVO Start rejected: native initialization or idle status unavailable"));
                m_lastStartRequestAt = 0;
            }
        }

        if (requestSpeed != -1 || requestInclination != -100) {
            if (!m_pendingTargetAt)
                m_pendingTargetAt = now;
            if (requestSpeed != -1)
                m_pendingSpeed = requestSpeed;
            if (requestInclination != -100)
                m_pendingInclination = requestInclination;
            requestSpeed = -1;
            requestInclination = -100;
        }
        if (m_pendingTargetAt) {
            const double speed = m_pendingSpeed == -1 ? m_protocol.speedKmh() : m_pendingSpeed;
            const double inclination = m_pendingInclination == -100 ? m_protocol.inclination() :
                treadmillInclinationOverrideReverse(m_pendingInclination);
            if (m_protocol.requestTarget(speed, inclination, now)) {
                m_pendingSpeed = -1;
                m_pendingInclination = -100;
                m_pendingTargetAt = 0;
            } else if (now - m_pendingTargetAt >= 10000 ||
                       UrevoProtocol::targetFrame(speed, inclination).isEmpty()) {
                emit debug(QStringLiteral("UREVO target rejected or expired awaiting running status"));
                m_pendingSpeed = -1;
                m_pendingInclination = -100;
                m_pendingTargetAt = 0;
            }
        }
    }

    m_protocol.tick(now);
    collectProtocolCommands();
    QSettings settings;
    update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toDouble()));
    update_hr_from_external();
}

void urevotreadmill::resetConnectionState() {
    m_protocol.reset();
    m_writeQueue.clear();
    m_writeInFlight = false;
    m_inFlightStop = false;
    m_configured = m_mainSubscribed = m_auxSubscribed = m_connectedSignalSent = false;
    m_lastFullAt = m_lastStartRequestAt = m_pendingTargetAt = 0;
    m_pendingSpeed = -1;
    m_pendingInclination = -100;
    requestSpeed = -1;
    requestInclination = -100;
    requestStart = requestStop = requestPause = -1;
    m_mainWrite = m_mainNotify = m_auxWrite = m_auxNotify = m_ftmsData = QLowEnergyCharacteristic();
    m_mainCccd = m_auxCccd = QLowEnergyDescriptor();
    if (m_mainService) m_mainService->deleteLater();
    if (m_auxService) m_auxService->deleteLater();
    if (m_ftmsService) m_ftmsService->deleteLater();
    m_mainService = m_auxService = m_ftmsService = nullptr;
}

void urevotreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    if (state == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("UREVO link lost; clearing control state before reconnect"));
        resetConnectionState();
        connectBackLater();
    }
}

bool urevotreadmill::connected() {
    return m_control && m_control->state() == QLowEnergyController::DiscoveredState &&
           m_mainSubscribed && m_auxSubscribed;
}
