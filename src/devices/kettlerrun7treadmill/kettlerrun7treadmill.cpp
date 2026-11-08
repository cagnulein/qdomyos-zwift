#include "kettlerrun7treadmill.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

namespace {
const QBluetoothUuid IsscService(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));
const QBluetoothUuid IsscWrite(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
const QBluetoothUuid IsscNotify(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));
// Known-good request from kdri, independently reproduced by S-FIT.
// The first two payload bytes are a nonce; the remaining 16 are its MD5 proof.
const QByteArray KdriHandshake = QByteArray::fromHex("2d24f22496a4ff9829f921be9daa9e4d0117");
constexpr int ChunkSize = 20; // Safe without negotiated MTU: ATT MTU 23 - 3.
constexpr int RequestTimeoutMs = 2500;
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
    m_control->connectToDevice();
}

void kettlerrun7treadmill::serviceScanDone() {
    service = m_control->createServiceObject(IsscService, this);
    if (!service) {
        fail(QStringLiteral("ISSC UART service not found"));
        return;
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
    txCharacteristic = service->characteristic(IsscWrite);
    rxCharacteristic = service->characteristic(IsscNotify);
    if (!txCharacteristic.isValid() || !rxCharacteristic.isValid()) {
        fail(QStringLiteral("ISSC UART write/notify characteristics not found"));
        return;
    }
    const auto properties = txCharacteristic.properties();
    if (properties & QLowEnergyCharacteristic::Write) {
        writeWithResponse = true;
    } else if (properties & QLowEnergyCharacteristic::WriteNoResponse) {
        writeWithResponse = false;
    } else {
        fail(QStringLiteral("ISSC UART write characteristic is not writable"));
        return;
    }
    if (!(rxCharacteristic.properties() & QLowEnergyCharacteristic::Notify)) {
        fail(QStringLiteral("ISSC UART receive characteristic cannot notify"));
        return;
    }
    const QLowEnergyDescriptor cccd =
        rxCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!cccd.isValid()) {
        fail(QStringLiteral("ISSC UART notification descriptor missing"));
        return;
    }
    emit debug(QStringLiteral("Kettler RUN 7 BLE: enabling notifications"));
    service->writeDescriptor(cccd, QByteArray::fromHex("0100"));
}

void kettlerrun7treadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
    if (descriptor.uuid() != QBluetoothUuid::ClientCharacteristicConfiguration ||
        value != QByteArray::fromHex("0100"))
        return;
    emit debug(QStringLiteral("Kettler RUN 7 BLE: notifications enabled, sending IRMP handshake"));
    enqueueRead(Handshake);
    sendNextRequest();
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
        QTimer::singleShot(50, this, &kettlerrun7treadmill::chunkWritten);
    }
}

void kettlerrun7treadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                 const QByteArray &value) {
    Q_UNUSED(value);
    if (writeWithResponse && waitingForWrite && characteristic.uuid() == IsscWrite)
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
    if (characteristic.uuid() != IsscNotify)
        return;
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
    if (!requestInFlight)
        return;
    emit debug(QStringLiteral("Kettler RUN 7 BLE IRMP timeout id=0x") +
               QString::number(activeFunction, 16));
    requestInFlight = false;
    waitingForWrite = false;
    pendingWrite.clear();
    requests.clear();
    if (!ready || ++telemetryTimeouts >= 3) {
        fail(QStringLiteral("IRMP request timeout"));
        return;
    }
}

void kettlerrun7treadmill::fail(const QString &reason) {
    emit debug(QStringLiteral("Kettler RUN 7 BLE failure: ") + reason);
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
    update_metrics(false, watts());
}
