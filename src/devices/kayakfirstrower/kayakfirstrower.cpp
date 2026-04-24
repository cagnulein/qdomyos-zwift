#include "kayakfirstrower.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif

#include "qzsettings.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"

#include <QBluetoothLocalDevice>
#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QMetaEnum>
#include <QRegularExpression>
#include <QSettings>
#include <QStringList>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

static constexpr int kKayakFirstMtu = 20;
static constexpr int kKayakFirstCommandRetryCount = 3;
static constexpr int kKayakFirstInterChunkDelayMs = 50;
static constexpr int kKayakFirstLongDelayMs = 1000;
static constexpr int kKayakFirstExtraLongDelayMs = 5000;

kayakfirstrower::kayakfirstrower(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                 double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);

    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceOffset = bikeResistanceOffset;
    this->bikeResistanceGain = bikeResistanceGain;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &kayakfirstrower::update);
    refresh->start(500ms);
}

bool kayakfirstrower::writeCommand(const QString &command, const QString &info, bool wait_for_response, bool append_crlf,
                                   int max_payload_size) {
    if (!gattCommunicationChannelService || !gattWriteCharacteristic.isValid()) {
        return false;
    }

    QByteArray payload = command.toUtf8();
    if (append_crlf && !payload.endsWith("\r\n")) {
        payload += "\r\n";
    }
    if (max_payload_size > 0 && payload.size() > max_payload_size) {
        payload = payload.left(max_payload_size);
    }

    QEventLoop loop;
    QMetaObject::Connection packetConnection;
    if (wait_for_response) {
        packetConnection = connect(this, &kayakfirstrower::packetReceived, &loop, &QEventLoop::quit);
    }

    for (int i = 0; i < payload.size(); i += kKayakFirstMtu) {
        QByteArray chunk = payload.mid(i, kKayakFirstMtu);
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, chunk,
                                                            QLowEnergyService::WriteWithoutResponse);
        emit debug(QStringLiteral(" >> ") + chunk.toHex(' ') + QStringLiteral(" // ") + info);

        if (wait_for_response) {
            QTimer::singleShot(500ms, &loop, &QEventLoop::quit);
            loop.exec();
        } else {
            QThread::msleep(kKayakFirstInterChunkDelayMs);
        }
    }

    if (packetConnection) {
        disconnect(packetConnection);
    }

    return true;
}

bool kayakfirstrower::sendCommandWithRetries(const QString &command, char expectedResponseByte, const QString &info,
                                             int maxAttempts, int postSuccessDelayMs, bool withEcho,
                                             bool append_crlf, int max_payload_size) {
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        controlResponsesQueue.clear();
        writeCommand(command, info, false, append_crlf, max_payload_size);
        if (waitForResponse(expectedResponseByte, kKayakFirstExtraLongDelayMs, withEcho)) {
            if (postSuccessDelayMs > 0) {
                QThread::msleep(postSuccessDelayMs);
            }
            return true;
        }

        if (attempt + 1 < maxAttempts) {
            QThread::msleep(kKayakFirstLongDelayMs);
        }
    }

    return false;
}

static void sleepAndDrainEvents(int delayMs) {
    const QDateTime start = QDateTime::currentDateTime();
    while (start.msecsTo(QDateTime::currentDateTime()) < delayMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        QThread::msleep(50);
    }
}

bool kayakfirstrower::waitForResponse(char expectedResponseByte, int timeoutMs, bool withEcho) {
    const QString expectedResponseText(QChar::fromLatin1(expectedResponseByte));

    auto waitForByte = [&](int waitMs) {
        const QDateTime start = QDateTime::currentDateTime();

        while (start.msecsTo(QDateTime::currentDateTime()) < waitMs) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

            while (!controlResponsesQueue.isEmpty()) {
                const char responseByte = controlResponsesQueue.dequeue();
                if (responseByte == expectedResponseByte) {
                    emit debug(QStringLiteral("response ok: ") + expectedResponseText);
                    return true;
                }
            }

            QThread::msleep(50);
        }

        return false;
    };

    bool seenEcho = !withEcho;
    if (withEcho) {
        seenEcho = waitForByte(timeoutMs);
        controlResponsesQueue.clear();
    }

    const bool seenResponse = waitForByte(timeoutMs);
    if (!seenResponse && !seenEcho) {
        emit debug(QStringLiteral("response timeout for byte ") + expectedResponseText);
    }
    return seenResponse || seenEcho;
}

QString kayakfirstrower::buildStartCommand() const {
    qint64 deviceTimestampMs = 0;

    if (lastDeviceTimestampSeconds > 0) {
        deviceTimestampMs = lastDeviceTimestampSeconds * 1000;
        if (lastDeviceTimestampCapturedAt.isValid()) {
            deviceTimestampMs += lastDeviceTimestampCapturedAt.msecsTo(QDateTime::currentDateTime());
        }
    } else {
        deviceTimestampMs = QDateTime(QDate(2000, 1, 1), QTime(0, 0), Qt::UTC).msecsTo(QDateTime::currentDateTimeUtc());
    }

    return QStringLiteral("9;1;1;%1").arg(deviceTimestampMs);
}

void kayakfirstrower::btinit() {
    // Sequence aligned with a working Kayak First session captured from TrackMyIndoorWorkout.
    emit debug(QStringLiteral("KayakFirst init: reset phase"));
    if (!sendCommandWithRetries(QStringLiteral("1"), '1', QStringLiteral("reset-1"), kKayakFirstCommandRetryCount,
                                kKayakFirstLongDelayMs, true)) {
        return;
    }

    if (!sendCommandWithRetries(QStringLiteral("1"), '1', QStringLiteral("reset-2"), kKayakFirstCommandRetryCount,
                                kKayakFirstExtraLongDelayMs, true)) {
        return;
    }

    QSettings settings;
    const int athleteWeight = settings.value(QZSettings::weight, QZSettings::default_weight).toInt();
    const QDateTime now = QDateTime::currentDateTime();
    const qint64 unixEpoch = now.toSecsSinceEpoch();
    const int tzOffsetMinutes = now.offsetFromUtc() / 60;

    // The working trace only sends the first 20-byte MTU chunk for the handshake command,
    // without the trailing sport flag/CRLF. Reproducing that behavior makes the console respond.
    const QString handshake = QStringLiteral("2;%1;%2;%3;").arg(unixEpoch).arg(tzOffsetMinutes).arg(athleteWeight);
    // The working app does not appear to receive an explicit ack for command '2',
    // but it still proceeds with the rest of the init sequence.
    emit debug(QStringLiteral("KayakFirst init: handshake phase"));
    bool handshakeAck = false;
    for (int attempt = 0; attempt < kKayakFirstCommandRetryCount; ++attempt) {
        controlResponsesQueue.clear();
        writeCommand(handshake, QStringLiteral("handshake"), false, false, kKayakFirstMtu);
        if (waitForResponse('2', kKayakFirstExtraLongDelayMs, true)) {
            handshakeAck = true;
            break;
        }

        emit debug(QStringLiteral("handshake ack not received, continuing retry flow"));
        if (attempt + 1 < kKayakFirstCommandRetryCount) {
            sleepAndDrainEvents(kKayakFirstLongDelayMs);
        }
    }
    Q_UNUSED(handshakeAck);
    sleepAndDrainEvents(kKayakFirstExtraLongDelayMs);

    emit debug(QStringLiteral("KayakFirst init: display configuration"));
    if (!sendCommandWithRetries(QStringLiteral("5;0;2;5;11;15"), '5', QStringLiteral("display-config"),
                                kKayakFirstCommandRetryCount, kKayakFirstExtraLongDelayMs, true)) {
        return;
    }

    initDone = true;
    autoStartPending = true;
    emit debug(QStringLiteral("KayakFirst init complete"));
}

void kayakfirstrower::parseLine(const QByteArray &line) {
    if (line.isEmpty()) {
        return;
    }

    emit packetReceived();

    if (!line.startsWith('6')) {
        QString controlResponse = QString::fromUtf8(line).trimmed();
        emit debug(QStringLiteral(" << ctrl ") + controlResponse);
        lastControlResponse = controlResponse;
        lastControlResponseTime = QDateTime::currentDateTime();
        const QStringList responseParts = controlResponse.split(';');
        if (responseParts.size() >= 3) {
            bool ok = false;
            const qint64 responseTimestamp = responseParts.at(2).toLongLong(&ok);
            if (ok && responseTimestamp > 0) {
                lastDeviceTimestampSeconds = responseTimestamp;
                lastDeviceTimestampCapturedAt = QDateTime::currentDateTime();
            }
        }
        if (!controlResponse.isEmpty()) {
            controlResponsesQueue.enqueue(controlResponse.at(0).toLatin1());
        }
        while (controlResponsesQueue.size() > 50) {
            controlResponsesQueue.dequeue();
        }
        return;
    }

    emit debug(QStringLiteral(" << data ") + line);

    const QString packet = QString::fromUtf8(line);
    const QStringList parts = packet.split(';');

    if (parts.size() < 20) {
        emit debug(QStringLiteral("KayakFirst partial fragment: ") + packet);
        return;
    }

    bool ok = false;

    const double strokeCount = parts.value(9).toDouble(&ok);
    if (ok) {
        if (strokeCount >= StrokesCount.value()) {
            const uint32_t delta = static_cast<uint32_t>(strokeCount - StrokesCount.value());
            if (delta > 0) {
                CrankRevs += delta;
            }
        }
        StrokesCount = strokeCount;
    }

    const double distanceKm = parts.value(10).toDouble(&ok);
    if (ok) {
        Distance = distanceKm;
    }

    const double speedMs = parts.value(11).toDouble(&ok);
    if (ok) {
        Speed = speedMs * 3.6;
    }

    const double strokeRate = parts.value(13).toDouble(&ok);
    if (ok) {
        Cadence = strokeRate;
    }

    double heartRate = parts.size() > 20 ? parts.value(20).toDouble(&ok) : 0.0;
    if ((!ok || heartRate <= 0) && parts.size() > 21) {
        heartRate = parts.value(21).toDouble(&ok);
    }
    if (ok && heartRate > 0) {
        Heart = heartRate;
    }

    const double calories = parts.size() > 17 ? parts.value(17).toDouble(&ok) : 0.0;
    if (ok && calories >= 0) {
        KCal = calories;
    }

    const double power = parts.size() > 19 ? parts.value(19).toDouble(&ok) : 0.0;
    if (ok) {
        m_watt = power;
    }

    const double elapsed = parts.value(22).toDouble(&ok);
    if (ok) {
        Q_UNUSED(elapsed);
    }
    if (Cadence.value() > 0) {
        LastCrankEventTime += static_cast<uint16_t>(1024.0 / (static_cast<double>(Cadence.value()) / 60.0));
    }

    lastDataUpdate = QDateTime::currentDateTime();
    update_metrics(false, m_watt.value());
}

void kayakfirstrower::update() {
    if (!m_control) {
        return;
    }

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
        return;
    }

    if (!(bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
          gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
          gattNotifyCharacteristic.isValid() && initDone)) {
        return;
    }

    if ((autoStartPending || requestStart != -1) && !workoutStarted) {
        const QString startCommand = buildStartCommand();
        emit debug(QStringLiteral("KayakFirst workout start requested"));
        const bool started = sendCommandWithRetries(startCommand, '9', QStringLiteral("start"),
                                                    kKayakFirstCommandRetryCount, 0, true);
        autoStartPending = false;
        requestStart = -1;
        if (started) {
            workoutStarted = true;
            emit bikeStarted();
        }
    }

    if (requestStop != -1) {
        sendCommandWithRetries(QStringLiteral("9;3;1"), '9', QStringLiteral("stop"), kKayakFirstCommandRetryCount, 0,
                               true);
        workoutStarted = false;
        autoStartPending = false;
        requestStop = -1;
    }

    if (sec1Update++ % 2 == 0) {
        writeCommand(QStringLiteral("6"), QStringLiteral("poll"), false);
    }

    update_metrics(false, m_watt.value());
}

void kayakfirstrower::serviceDiscovered(const QBluetoothUuid &gatt) { emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString()); }

void kayakfirstrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);

    streamBuffer.append(newValue);

    int lineEnd = -1;
    while ((lineEnd = streamBuffer.indexOf("\r\n")) >= 0) {
        const QByteArray line = streamBuffer.left(lineEnd);
        streamBuffer.remove(0, lineEnd + 2);
        parseLine(line);
    }

    if (streamBuffer.size() == 3 && streamBuffer.at(0) == '1' && streamBuffer.at(1) == 0x01 &&
        streamBuffer.at(2) == 0x00) {
        const QByteArray line = streamBuffer;
        streamBuffer.clear();
        parseLine(line);
    }

#ifdef Q_OS_ANDROID
    QSettings settings;
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        Heart = (uint8_t)KeepAwakeHelper::heart();
    }
#endif
}

void kayakfirstrower::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state != QLowEnergyService::ServiceDiscovered) {
        return;
    }

    auto characteristics = gattCommunicationChannelService->characteristics();
    for (const QLowEnergyCharacteristic &c : qAsConst(characteristics)) {
        emit debug(QStringLiteral("characteristic ") + c.uuid().toString());
    }

    const QBluetoothUuid rwUuid(QStringLiteral("0000ffe1-0000-1000-8000-00805f9b34fb"));

    gattWriteCharacteristic = gattCommunicationChannelService->characteristic(rwUuid);
    gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(rwUuid);

    Q_ASSERT(gattWriteCharacteristic.isValid());
    Q_ASSERT(gattNotifyCharacteristic.isValid());

    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
            &kayakfirstrower::characteristicChanged);
    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
            &kayakfirstrower::characteristicWritten);
    connect(gattCommunicationChannelService,
            static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
            this, &kayakfirstrower::errorService);
    connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
            &kayakfirstrower::descriptorWritten);

    if (!firstVirtualBike && !this->hasVirtualDevice()) {
        QSettings settings;
        const bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        const bool virtual_device_rower =
            settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();

        if (virtual_device_enabled) {
            if (!virtual_device_rower) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                connect(virtualBike, &virtualbike::changeInclination, this, &kayakfirstrower::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                emit debug(QStringLiteral("creating virtual rower interface..."));
                auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
    }
    firstVirtualBike = 1;

    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);
    gattCommunicationChannelService->writeDescriptor(
        gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
}

void kayakfirstrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));
    initDone = false;
    autoStartPending = false;
    workoutStarted = false;
    streamBuffer.clear();
    controlResponsesQueue.clear();
    lastDeviceTimestampSeconds = 0;
    lastDeviceTimestampCapturedAt = QDateTime();
    initRequest = true;
    emit connectedAndDiscovered();
}

void kayakfirstrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void kayakfirstrower::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    const QBluetoothUuid serviceUuid(QStringLiteral("0000ffe0-0000-1000-8000-00805f9b34fb"));
    gattCommunicationChannelService = m_control->createServiceObject(serviceUuid);

    if (!gattCommunicationChannelService) {
        emit debug(QStringLiteral("invalid service ") + serviceUuid.toString());
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &kayakfirstrower::stateChanged);
    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
            &kayakfirstrower::characteristicChanged);
    gattCommunicationChannelService->discoverDetails();
}

void kayakfirstrower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::ControllerState>();
    emit debug(QStringLiteral("controllerStateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyController::UnconnectedState && m_control) {
        emit debug(QStringLiteral("trying to connect back again..."));
        initDone = false;
        initRequest = false;
        m_control->connectToDevice();
    }
}

void kayakfirstrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") + device.address().toString() +
               QStringLiteral(")"));

    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &kayakfirstrower::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &kayakfirstrower::serviceScanDone);
    connect(m_control, &QLowEnergyController::stateChanged, this, &kayakfirstrower::controllerStateChanged);
    connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &kayakfirstrower::error);
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

void kayakfirstrower::error(QLowEnergyController::Error err) {
    Q_UNUSED(err);
    emit debug(QStringLiteral("controller error ") + m_control->errorString());
}

void kayakfirstrower::errorService(QLowEnergyService::ServiceError err) {
    Q_UNUSED(err);
    emit debug(QStringLiteral("service error"));
}

uint16_t kayakfirstrower::watts() { return m_watt.value(); }

bool kayakfirstrower::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}
