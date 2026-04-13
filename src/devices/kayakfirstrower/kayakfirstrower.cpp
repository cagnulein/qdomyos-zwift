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

bool kayakfirstrower::writeCommand(const QString &command, const QString &info, bool wait_for_response) {
    if (!gattCommunicationChannelService || !gattWriteCharacteristic.isValid()) {
        return false;
    }

    QString actualCommand = command;
    if (!actualCommand.endsWith(QStringLiteral("\r\n"))) {
        actualCommand += QStringLiteral("\r\n");
    }

    QByteArray payload = actualCommand.toUtf8();
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &kayakfirstrower::packetReceived, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
    }
    timeout.singleShot(500ms, &loop, &QEventLoop::quit);

    for (int i = 0; i < payload.size(); i += kKayakFirstMtu) {
        QByteArray chunk = payload.mid(i, kKayakFirstMtu);
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, chunk);
        emit debug(QStringLiteral(" >> ") + chunk.toHex(' ') + QStringLiteral(" // ") + info);
        loop.exec();
    }

    return true;
}

bool kayakfirstrower::waitForResponse(const QString &expectedResponse, int timeoutMs, bool checkExact) {
    const QDateTime start = QDateTime::currentDateTime();

    while (start.msecsTo(QDateTime::currentDateTime()) < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

        if (lastControlResponseTime >= start && !lastControlResponse.isEmpty()) {
            if ((checkExact && lastControlResponse == expectedResponse) ||
                (!checkExact && lastControlResponse.startsWith(expectedResponse))) {
                emit debug(QStringLiteral("response ok: ") + lastControlResponse);
                return true;
            }
        }

        QThread::msleep(50);
    }

    emit debug(QStringLiteral("response timeout for ") + expectedResponse + QStringLiteral(" last=") + lastControlResponse);
    return false;
}

void kayakfirstrower::btinit() {
    // Sequence ported from TrackMyIndoorWorkout KayakFirst descriptor
    if (!writeCommand(QStringLiteral("1"), QStringLiteral("reset-1"), false) ||
        !waitForResponse(QStringLiteral("1"), 5000, true)) {
        return;
    }
    QThread::msleep(500);

    if (!writeCommand(QStringLiteral("1"), QStringLiteral("reset-2"), false) ||
        !waitForResponse(QStringLiteral("1"), 5000, true)) {
        return;
    }
    QThread::msleep(5000);

    QSettings settings;
    const int athleteWeight = settings.value(QZSettings::weight, QZSettings::default_weight).toInt();
    const QDateTime now = QDateTime::currentDateTime();
    const qint64 unixEpoch = now.toSecsSinceEpoch();
    const int tzOffsetMinutes = now.offsetFromUtc() / 60;

    // TrackMyIndoorWorkout old-handshake format:
    // 2;<epochSec>;<timezoneMinutes>;<athleteWeight>;<sportFlag>
    // sportFlag: 1 = kayaking, 2 = canoe
    const int sportFlag = 1;
    const QString handshake =
        QStringLiteral("2;%1;%2;%3;%4").arg(unixEpoch).arg(tzOffsetMinutes).arg(athleteWeight).arg(sportFlag);
    if (!writeCommand(handshake, QStringLiteral("handshake"), false) ||
        !waitForResponse(QStringLiteral("2;"), 5000, false)) {
        return;
    }
    QThread::msleep(5000);

    // Basic display configuration (8 slots all to default value 1)
    if (!writeCommand(QStringLiteral("5;1;1;1;1;1;1;1;1"), QStringLiteral("display-config"), false) ||
        !waitForResponse(QStringLiteral("5;"), 5000, false)) {
        return;
    }
    QThread::msleep(5000);

    initDone = true;
}

void kayakfirstrower::parseLine(const QByteArray &line) {
    if (line.isEmpty()) {
        return;
    }

    emit packetReceived();

    if (!line.startsWith('6')) {
        emit debug(QStringLiteral(" << ctrl ") + line);
        lastControlResponse = QString::fromUtf8(line).trimmed();
        lastControlResponseTime = QDateTime::currentDateTime();
        return;
    }

    emit debug(QStringLiteral(" << data ") + line);

    const QString packet = QString::fromUtf8(line);
    const QStringList parts = packet.split(';');

    if (parts.size() < 24) {
        emit debug(QStringLiteral("KayakFirst partial fragment: ") + packet);
        return;
    }

    bool ok = false;

    const double distanceMeters = parts.value(9).toDouble(&ok);
    if (ok) {
        Distance = distanceMeters / 1000.0;
    }

    const double speedMs = parts.value(11).toDouble(&ok);
    if (ok) {
        Speed = speedMs * 3.6;
    }

    const double strokeRate = parts.value(13).toDouble(&ok);
    if (ok) {
        Cadence = strokeRate;
    }

    const double heartRate = parts.value(16).toDouble(&ok);
    if (ok && heartRate > 0) {
        Heart = heartRate;
    }

    const double calories = parts.value(17).toDouble(&ok);
    if (ok && calories >= 0) {
        KCal = calories;
    }

    const double power = parts.value(21).toDouble(&ok);
    if (ok) {
        m_watt = power;
    }

    const double elapsed = parts.value(22).toDouble(&ok);
    if (ok) {
        // device reports elapsed seconds, use only for telemetry/debug
        Q_UNUSED(elapsed);
    }

    StrokesCount += 1;
    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)Cadence.value()) / 60.0));
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

    if (requestStart != -1) {
        writeCommand(QStringLiteral("9;1"), QStringLiteral("start"), false);
        waitForResponse(QStringLiteral("9;"), 5000, false);
        requestStart = -1;
        emit bikeStarted();
    }

    if (requestStop != -1) {
        writeCommand(QStringLiteral("9;3"), QStringLiteral("stop"), false);
        waitForResponse(QStringLiteral("9;"), 5000, false);
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
