#include "kettlerrun7treadmill.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include "qzsettings.h"

namespace {
// RUN 71 advertises a BLE GATT serial service at UUID 0x1101, with
// FFF2 (WriteNoResponse) and FFF1 (Notify). These values are taken from
// the customer's actual QZ log; UUID 0x1101 is used as a *GATT* service,
// not through Bluetooth Classic or RFCOMM.
const QBluetoothUuid BleSerialService(quint16(0x1101));
const QBluetoothUuid BleSerialWrite(quint16(0xFFF2));
const QBluetoothUuid BleSerialNotify(quint16(0xFFF1));

// A different RUN variant may expose the standard ISSC transparent UART.
// These are NOT the ISSC connection-parameter (6daa) or air-patch (aca3)
// vendor-control characteristics observed on the customer's hardware.
const QBluetoothUuid IsscService(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));
const QBluetoothUuid IsscWrite(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
const QBluetoothUuid IsscNotify(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));
// Known-good request from kdri, independently reproduced by S-FIT.
// The first two payload bytes are a nonce; the remaining 16 are its MD5 proof.
const QByteArray KdriHandshake = QByteArray::fromHex("2d24f22496a4ff9829f921be9daa9e4d0117");
constexpr int ChunkSize = 20; // Safe without negotiated MTU: ATT MTU 23 - 3.
constexpr int RequestTimeoutMs = 2500;
constexpr int NotificationSettleDelayMs = 500;
constexpr int HandshakeChunkSpacingMs = 180;
constexpr int HandshakeRetryBaseDelayMs = 350;
constexpr int MaxHandshakeAttempts = 3; // One initial attempt and two same-GATT-link retries.
}

kettlerrun7treadmill::kettlerrun7treadmill() {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    requestTimer.setSingleShot(true);
    connect(&requestTimer, &QTimer::timeout, this, &kettlerrun7treadmill::requestTimedOut);
    connect(&refresh, &QTimer::timeout, this, &kettlerrun7treadmill::update);
    refresh.setInterval(500);
}

bool kettlerrun7treadmill::connected() {
    return ready && m_control && m_control->state() == QLowEnergyController::DiscoveredState;
}

void kettlerrun7treadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    // Invalidate any delayed handshake/retry callbacks from an older connection.
    ++connectionGeneration;
    handshakeScheduled = false;
    handshakeAttempts = 0;
    handshakeRxChunks = 0;
    handshakeRxBytes = 0;
    disconnectedReported = false;
    bluetoothDevice = device;
    emit debug(QStringLiteral("Kettler RUN 7 BLE: connecting to ") + device.name());
    m_control = QLowEnergyController::createCentral(device, this);
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        emit debug(QStringLiteral("Kettler RUN 7 BLE connected; discovering services"));
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::discoveryFinished,
            this, &kettlerrun7treadmill::serviceScanDone);
    connect(m_control, &QLowEnergyController::disconnected,
            this, &kettlerrun7treadmill::controllerDisconnected);
    requestTimer.start(15000); // Includes BLE connection and service discovery.
    m_control->connectToDevice();
}

void kettlerrun7treadmill::serviceScanDone() {
    // Prefer the actual RUN 71 BLE GATT serial profile; it coexists with
    // an ISSC vendor service whose characteristics are not a UART.
    service = m_control->createServiceObject(BleSerialService, this);
    if (service) {
        emit debug(QStringLiteral("Kettler RUN 7 BLE: using GATT 0x1101 FFF2/FFF1 transport"));
    } else {
        service = m_control->createServiceObject(IsscService, this);
        if (!service) {
            fail(QStringLiteral("BLE serial service (0x1101 or ISSC) not found"));
            return;
        }
        emit debug(QStringLiteral("Kettler RUN 7 BLE: trying standard ISSC transparent UART"));
    }
    connect(service, &QLowEnergyService::stateChanged,
            this, &kettlerrun7treadmill::serviceStateChanged);
    connect(service, &QLowEnergyService::descriptorWritten,
            this, &kettlerrun7treadmill::descriptorWritten);
    connect(service, &QLowEnergyService::characteristicWritten,
            this, &kettlerrun7treadmill::characteristicWritten);
    connect(service, &QLowEnergyService::characteristicChanged,
            this, &kettlerrun7treadmill::characteristicChanged);
    service->discoverDetails();
}

void kettlerrun7treadmill::serviceStateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::ServiceDiscovered)
        return;
    const bool isBleSerial = service->serviceUuid() == BleSerialService;
    const QBluetoothUuid txUuid = isBleSerial ? BleSerialWrite : IsscWrite;
    const QBluetoothUuid rxUuid = isBleSerial ? BleSerialNotify : IsscNotify;
    txCharacteristic = service->characteristic(txUuid);
    rxCharacteristic = service->characteristic(rxUuid);
    if (!txCharacteristic.isValid() || !rxCharacteristic.isValid()) {
        fail(QStringLiteral("BLE serial write/notify characteristics not found (TX=") +
             txUuid.toString() + QStringLiteral(", RX=") + rxUuid.toString() + ')');
        return;
    }
    emit debug(QStringLiteral("Kettler RUN 7 BLE: TX=") + txUuid.toString() +
               QStringLiteral(" RX=") + rxUuid.toString());
    const auto properties = txCharacteristic.properties();
    if (properties & QLowEnergyCharacteristic::Write) {
        writeWithResponse = true;
    } else if (properties & QLowEnergyCharacteristic::WriteNoResponse) {
        writeWithResponse = false;
    } else {
        fail(QStringLiteral("BLE UART write characteristic is not writable"));
        return;
    }
    if (!(rxCharacteristic.properties() & QLowEnergyCharacteristic::Notify)) {
        fail(QStringLiteral("BLE UART receive characteristic cannot notify"));
        return;
    }
    const QLowEnergyDescriptor cccd =
        rxCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!cccd.isValid()) {
        fail(QStringLiteral("BLE UART notification descriptor missing"));
        return;
    }
    emit debug(QStringLiteral("Kettler RUN 7 BLE: enabling notifications"));
    requestTimer.start(5000); // Do not hang indefinitely if CCCD never completes.
    service->writeDescriptor(cccd, QByteArray::fromHex("0100"));
}

void kettlerrun7treadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
    if (descriptor.uuid() != QBluetoothUuid::ClientCharacteristicConfiguration ||
        value != QByteArray::fromHex("0100"))
        return;
    requestTimer.stop();
    if (handshakeScheduled || requestInFlight || handshaken || ready)
        return;
    handshakeScheduled = true;
    const quint32 generation = connectionGeneration;
    emit debug(QStringLiteral("Kettler RUN 7 BLE: notifications enabled; waiting ") +
               QString::number(NotificationSettleDelayMs) +
               QStringLiteral(" ms before initial IRMP handshake"));
    QTimer::singleShot(NotificationSettleDelayMs, this, [this, generation]() {
        if (generation != connectionGeneration)
            return;
        handshakeScheduled = false;
        if (!m_control || m_control->state() != QLowEnergyController::DiscoveredState ||
            !service || handshaken || ready || requestInFlight)
            return;
        enqueueRead(Handshake);
        sendNextRequest();
    });
}

void kettlerrun7treadmill::enqueueRead(quint16 function) {
    requests.enqueue(function);
}

void kettlerrun7treadmill::sendNextRequest() {
    if (!service || !txCharacteristic.isValid() || requestInFlight ||
        waitingForWrite || !pendingWrite.isEmpty() || requests.isEmpty() ||
        !m_control || m_control->state() != QLowEnergyController::DiscoveredState)
        return;

    activeFunction = requests.dequeue();
    requestInFlight = true;
    if (activeFunction == Handshake) {
        ++handshakeAttempts;
        handshakeRxChunks = 0;
        handshakeRxBytes = 0;
        handshakeAttemptTimer.restart();
        emit debug(QStringLiteral("Kettler RUN 7 BLE: handshake attempt ") +
                   QString::number(handshakeAttempts) + '/' +
                   QString::number(MaxHandshakeAttempts) +
                   QStringLiteral(" on current GATT connection"));
    }
    const QByteArray payload = activeFunction == Handshake ? KdriHandshake : QByteArray();
    pendingWrite = kettlerclassicprotocol::encodeFrame(activeFunction, Read, payload);
    emit debug(QStringLiteral("Kettler RUN 7 BLE TX IRMP read 0x") +
               QString::number(activeFunction, 16) + QStringLiteral(" bytes=") +
               QString::number(pendingWrite.size()));
    sendNextChunk();
}

void kettlerrun7treadmill::sendNextChunk() {
    if (!service || !txCharacteristic.isValid() || !requestInFlight ||
        waitingForWrite || pendingWrite.isEmpty())
        return;

    const QByteArray chunk = pendingWrite.left(ChunkSize);
    pendingWrite.remove(0, chunk.size());
    waitingForWrite = true;
    requestTimer.start(RequestTimeoutMs);
    emit debug(QStringLiteral("Kettler RUN 7 BLE TX chunk: ") + chunk.toHex(' '));
    service->writeCharacteristic(txCharacteristic, chunk,
                                 writeWithResponse ? QLowEnergyService::WriteWithResponse
                                                   : QLowEnergyService::WriteWithoutResponse);
    if (!writeWithResponse) {
        // QLowEnergyService may not emit characteristicWritten for unacknowledged writes.
        // Pace ATT writes instead of sending back-to-back fragments.
        // Only the fragmented handshake needs conservative inter-chunk pacing.
        // Keep subsequent read requests at their original rate.
        const int delayMs = activeFunction == Handshake && !pendingWrite.isEmpty()
                                ? HandshakeChunkSpacingMs : 50;
        QTimer::singleShot(delayMs, this, &kettlerrun7treadmill::chunkWritten);
    }
}

void kettlerrun7treadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &value) {
    Q_UNUSED(value);
    if (writeWithResponse && waitingForWrite && characteristic.uuid() == txCharacteristic.uuid())
        chunkWritten();
}

void kettlerrun7treadmill::chunkWritten() {
    if (!waitingForWrite)
        return;
    waitingForWrite = false;
    if (!pendingWrite.isEmpty()) {
        sendNextChunk();
    } else if (!requestInFlight) {
        sendNextRequest();
    }
}

void kettlerrun7treadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &value) {
    if (characteristic.uuid() != rxCharacteristic.uuid())
        return;
    if (requestInFlight && activeFunction == Handshake) {
        if (handshakeRxChunks == 0) {
            emit debug(QStringLiteral("Kettler RUN 7 BLE: first handshake RX after ") +
                       QString::number(handshakeAttemptTimer.elapsed()) +
                       QStringLiteral(" ms (attempt ") + QString::number(handshakeAttempts) + ')');
        }
        ++handshakeRxChunks;
        handshakeRxBytes += value.size();
    }
    emit debug(QStringLiteral("Kettler RUN 7 BLE RX chunk: ") + value.toHex(' '));
    receiveBuffer.append(value);
    for (const auto &frame : kettlerclassicprotocol::consumeFrames(receiveBuffer))
        handleFrame(frame);
}

quint16 kettlerrun7treadmill::be16(const QByteArray &payload) {
    if (payload.size() != 2)
        return 0;
    return (static_cast<quint16>(static_cast<quint8>(payload.at(0))) << 8) |
           static_cast<quint8>(payload.at(1));
}

bool kettlerrun7treadmill::validHandshakeResponse(const QByteArray &payload) {
    if (payload.size() != 18)
        return false;
    // The S-FIT client verifies the challenge returned by the treadmill with
    // MD5(response salt || the S-FIT IRMP shared secret).
    const QByteArray secret = QByteArray::fromHex("35d834d59da00bbb43c91743accd5603");
    const QByteArray expected = QCryptographicHash::hash(payload.left(2) + secret, QCryptographicHash::Md5);
    return payload.mid(2) == expected;
}

void kettlerrun7treadmill::handleFrame(const kettlerclassicprotocol::Frame &frame) {
    emit debug(QStringLiteral("Kettler RUN 7 BLE RX IRMP id=0x") +
               QString::number(frame.function, 16) + QStringLiteral(" sub=0x") +
               QString::number(frame.subFunction, 16) +
               QStringLiteral(" payload=") + frame.payload.toHex(' '));

    if (!requestInFlight || frame.function != activeFunction)
        return;
    if (frame.subFunction != Answer && frame.subFunction != Status && frame.subFunction != Error)
        return;

    requestTimer.stop();
    requestInFlight = false;
    telemetryTimeouts = 0;
    if (frame.subFunction == Error) {
        if (!ready) {
            fail(QStringLiteral("IRMP setup rejected for id 0x") + QString::number(frame.function, 16));
            return;
        }
    } else if (frame.function == Handshake) {
        if (frame.subFunction != Answer || !validHandshakeResponse(frame.payload)) {
            fail(QStringLiteral("invalid IRMP handshake response"));
            return;
        }
        emit debug(QStringLiteral("Kettler RUN 7 BLE: handshake accepted on attempt ") +
                   QString::number(handshakeAttempts) + QStringLiteral(" after ") +
                   QString::number(handshakeAttemptTimer.elapsed()) + QStringLiteral(" ms"));
        handshaken = true;
        enqueueRead(DeviceType);
    } else if (frame.function == DeviceType) {
        if (!handshaken || frame.subFunction != Answer ||
            frame.payload.size() != 1 || frame.payload.at(0) != 'T') {
            fail(QStringLiteral("IRMP device type is not a treadmill (T)"));
            return;
        }
        ready = true;
        emit debug(QStringLiteral("Kettler RUN 7 BLE IRMP ready (read only)"));
        emit connectedAndDiscovered();
        enqueueRead(DeviceName);
        refresh.start();
    } else if (ready && frame.payload.size() == 2 && frame.subFunction == Answer) {
        if (frame.function == SpeedCurrent) {
            Speed = be16(frame.payload) / 10.0;  // km/h
        } else if (frame.function == InclineCurrent) {
            Inclination = be16(frame.payload) / 10.0;
        } else if (frame.function == Pulse) {
            Heart = be16(frame.payload);
        }
    }
    if (!waitingForWrite && pendingWrite.isEmpty())
        sendNextRequest();
}

void kettlerrun7treadmill::requestTimedOut() {
    if (!requestInFlight) {
        if (!ready)
            fail(QStringLiteral("BLE connection/notification setup timeout"));
        return;
    }
    const bool handshakeTimedOut = !ready && activeFunction == Handshake;
    emit debug(QStringLiteral("Kettler RUN 7 BLE IRMP timeout id=0x") +
               QString::number(activeFunction, 16) +
               (handshakeTimedOut
                    ? QStringLiteral(" attempt=") + QString::number(handshakeAttempts) +
                          QStringLiteral(" rxChunks=") + QString::number(handshakeRxChunks) +
                          QStringLiteral(" rxBytes=") + QString::number(handshakeRxBytes) +
                          QStringLiteral(" bufferedBytes=") + QString::number(receiveBuffer.size())
                    : QString()));
    requestInFlight = false;
    waitingForWrite = false;
    pendingWrite.clear();
    requests.clear();
    if (handshakeTimedOut && handshakeAttempts < MaxHandshakeAttempts) {
        // A GATT connection can be up while the UART endpoint is not yet
        // receptive. Retry on the same link before restarting BLE discovery.
        receiveBuffer.clear(); // Do not mix a partial old response with the retry.
        const int delayMs = HandshakeRetryBaseDelayMs * handshakeAttempts;
        const quint32 generation = connectionGeneration;
        emit debug(QStringLiteral("Kettler RUN 7 BLE: retrying IRMP handshake on same GATT link in ") +
                   QString::number(delayMs) + QStringLiteral(" ms"));
        QTimer::singleShot(delayMs, this, [this, generation]() {
            if (generation != connectionGeneration || ready || handshaken ||
                !m_control || m_control->state() != QLowEnergyController::DiscoveredState ||
                !service || requestInFlight)
                return;
            enqueueRead(Handshake);
            sendNextRequest();
        });
        return;
    }
    if (!ready || ++telemetryTimeouts >= 3) {
        fail(QStringLiteral("IRMP request timeout"));
        return;
    }
}

void kettlerrun7treadmill::fail(const QString &reason) {
    emit debug(QStringLiteral("Kettler RUN 7 BLE failure: ") + reason);
    ++connectionGeneration;
    handshakeScheduled = false;
    ready = false;
    refresh.stop();
    requestTimer.stop();
    requestInFlight = false;
    waitingForWrite = false;
    pendingWrite.clear();
    requests.clear();
    if (m_control && m_control->state() != QLowEnergyController::UnconnectedState) {
        m_control->disconnectFromDevice();
    } else {
        controllerDisconnected();
    }
}

void kettlerrun7treadmill::controllerDisconnected() {
    ++connectionGeneration;
    handshakeScheduled = false;
    ready = false;
    handshaken = false;
    requestInFlight = false;
    waitingForWrite = false;
    pendingWrite.clear();
    requests.clear();
    receiveBuffer.clear();
    refresh.stop();
    requestTimer.stop();
    if (!disconnectedReported) {
        disconnectedReported = true;
        emit disconnected();
    }
}

void kettlerrun7treadmill::update() {
    if (!connected())
        return;
    // This first experimental implementation deliberately never writes speed,
    // incline, start/stop or resets counters. Read requests are the only writes.
    requestSpeed = -1;
    requestInclination = -100;
    if (!requestInFlight && !waitingForWrite && requests.isEmpty()) {
        enqueueRead(SpeedCurrent);
        enqueueRead(InclineCurrent);
        enqueueRead(Pulse);
        sendNextRequest();
    }
    QSettings settings;
    update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));
}
