#include "xcxbike.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "qzsettings.h"
#include "virtualdevices/virtualbike.h"

#include <QDebug>
#include <QSettings>

#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

namespace {
const QBluetoothUuid fff0Uuid(QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb"));
const QBluetoothUuid fff5Uuid(QStringLiteral("0000fff5-0000-1000-8000-00805f9b34fb"));
const QBluetoothUuid fff6Uuid(QStringLiteral("0000fff6-0000-1000-8000-00805f9b34fb"));

quint8 byteAt(const QByteArray &packet, int offset) { return static_cast<quint8>(packet.at(offset)); }

quint16 le16(const QByteArray &packet, int offset) {
    return static_cast<quint16>(byteAt(packet, offset)) | (static_cast<quint16>(byteAt(packet, offset + 1)) << 8);
}

quint32 le24(const QByteArray &packet, int offset) {
    return static_cast<quint32>(byteAt(packet, offset)) | (static_cast<quint32>(byteAt(packet, offset + 1)) << 8) |
           (static_cast<quint32>(byteAt(packet, offset + 2)) << 16);
}
} // namespace

xcxbike::xcxbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset, double bikeResistanceGain)
    : noWriteResistance(noWriteResistance), noHeartService(noHeartService), bikeResistanceOffset(bikeResistanceOffset),
      bikeResistanceGain(bikeResistanceGain) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    connect(&refresh, &QTimer::timeout, this, &xcxbike::update);
    refresh.start(300ms);
}

bool xcxbike::parseTelemetry(const QByteArray &packet, XcxTelemetry *telemetry) {
    // Captures contain the original 20-byte telemetry frame, while the physical bike has also
    // been observed appending a three-byte trailer. The trailer's semantics are not known, so only
    // the proven first 20 bytes are decoded for both variants.
    if (!telemetry || (packet.size() != 20 && packet.size() != 23) || byteAt(packet, 0) != 0xfa ||
        byteAt(packet, 1) != 0x05)
        return false;

    telemetry->state1 = byteAt(packet, 2);
    telemetry->state2 = byteAt(packet, 3);
    telemetry->timer = le16(packet, 4);
    telemetry->speedKph = le16(packet, 6) / 10.0;
    telemetry->distanceRaw = le24(packet, 8);
    telemetry->energy = le16(packet, 11);
    telemetry->cadence = le16(packet, 13);
    telemetry->unknown15 = le16(packet, 15);
    // The capture only proves that byte 17 contains power up to 255 W. A new capture is required
    // before treating any other byte as a high byte for larger power values.
    telemetry->power = byteAt(packet, 17);
    telemetry->resistance = le16(packet, 18);
    return true;
}

void xcxbike::update() {
    if (m_control && m_control->state() == QLowEnergyController::UnconnectedState)
        emit disconnected();
    if (notificationEnabled)
        update_metrics(false, watts());

    // This protocol is telemetry-only: no FFF5 command format was present in the capture.
    requestResistance = -1;
    if (requestStart != -1) {
        requestStart = -1;
        emit bikeStarted();
    }
    requestStop = -1;
}

void xcxbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
    if (characteristic.uuid() != fff6Uuid)
        return;

    XcxTelemetry telemetry;
    if (!parseTelemetry(value, &telemetry)) {
        qDebug() << QStringLiteral("XCX unexpected FFF6 packet") << value.size() << value.toHex(' ');
        return;
    }

    qDebug() << QStringLiteral("XCX << raw:") << value.toHex(' ') << QStringLiteral("state1:") << telemetry.state1
             << QStringLiteral("state2:") << telemetry.state2 << QStringLiteral("timer:") << telemetry.timer
             << QStringLiteral("speed:") << telemetry.speedKph << QStringLiteral("distanceRaw:")
             << telemetry.distanceRaw << QStringLiteral("energy:") << telemetry.energy << QStringLiteral("cadence:")
             << telemetry.cadence << QStringLiteral("unknown15:") << telemetry.unknown15 << QStringLiteral("power:")
             << telemetry.power << QStringLiteral("resistance:") << telemetry.resistance;

    if (!receivedState || telemetry.state1 != lastState1 || telemetry.state2 != lastState2) {
        qDebug() << QStringLiteral("XCX state changed") << lastState1 << lastState2 << QStringLiteral("->")
                 << telemetry.state1 << telemetry.state2;
        lastState1 = telemetry.state1;
        lastState2 = telemetry.state2;
        receivedState = true;
    }

    const QDateTime now = QDateTime::currentDateTime();
    const qint64 elapsedMs = lastTelemetryTime.isValid() ? lastTelemetryTime.msecsTo(now) : 0;
    Speed = telemetry.speedKph;
    Cadence = telemetry.cadence;
    m_watt = telemetry.power;
    Resistance = telemetry.resistance;
    KCal = telemetry.energy;
    // The proprietary counter advances plausibly in 0.1 km units. It must not be interpreted as
    // metres like the device's broken FTMS mirror does.
    Distance = telemetry.distanceRaw / 10.0;

    if (elapsedMs > 0 && elapsedMs < 10000 && telemetry.cadence > 0) {
        partialCrankRevolution += telemetry.cadence * (elapsedMs / 60000.0);
        const quint32 completedRevolutions = static_cast<quint32>(std::floor(partialCrankRevolution));
        if (completedRevolutions) {
            CrankRevs += completedRevolutions;
            LastCrankEventTime += static_cast<quint16>(completedRevolutions * 1024.0 * 60.0 / telemetry.cadence);
            partialCrankRevolution -= completedRevolutions;
        }
    }
    lastTelemetryTime = now;

#ifdef Q_OS_ANDROID
    QSettings settings;
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = static_cast<uint8_t>(KeepAwakeHelper::heart());
    else
#endif
        update_hr_from_external();
}

void xcxbike::serviceStateChanged(QLowEnergyService::ServiceState state) {
    qDebug() << QStringLiteral("XCX FFF0 service state") << state;
    if (state != QLowEnergyService::ServiceDiscovered)
        return;

    fff5WriteCharacteristic = fff0Service->characteristic(fff5Uuid);
    fff6NotifyCharacteristic = fff0Service->characteristic(fff6Uuid);
    if (!fff6NotifyCharacteristic.isValid()) {
        qDebug() << QStringLiteral("XCX FFF6 notification characteristic not found");
        return;
    }
    qDebug() << QStringLiteral("XCX FFF5 present for future protocol research:") << fff5WriteCharacteristic.isValid();
    connect(fff0Service, &QLowEnergyService::characteristicChanged, this, &xcxbike::characteristicChanged);
    connect(fff0Service, &QLowEnergyService::descriptorWritten, this, &xcxbike::descriptorWritten);
    createVirtualBike();

    const QLowEnergyDescriptor cccd =
        fff6NotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!cccd.isValid()) {
        qDebug() << QStringLiteral("XCX FFF6 CCCD not found");
        return;
    }
    fff0Service->writeDescriptor(cccd, QByteArray::fromHex("0100"));
}

void xcxbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
    qDebug() << QStringLiteral("XCX descriptor written") << descriptor.uuid() << value.toHex(' ');
    if (descriptor.uuid() == QBluetoothUuid::ClientCharacteristicConfiguration &&
        value == QByteArray::fromHex("0100")) {
        notificationEnabled = true;
        emit connectedAndDiscovered();
    }
}

void xcxbike::createVirtualBike() {
    if (hasVirtualDevice())
        return;
    QSettings settings;
    if (settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
        auto *virtualBike =
            new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        connect(virtualBike, &virtualbike::changeInclination, this, &xcxbike::changeInclination);
        setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
    }
}

void xcxbike::serviceScanDone() {
    fff0Service = m_control->createServiceObject(fff0Uuid, this);
    if (!fff0Service) {
        qDebug() << QStringLiteral("XCX proprietary FFF0 service not found");
        return;
    }
    connect(fff0Service, &QLowEnergyService::stateChanged, this, &xcxbike::serviceStateChanged);
    fff0Service->discoverDetails();
}

void xcxbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &xcxbike::serviceScanDone);
    connect(m_control, &QLowEnergyController::stateChanged, this, &xcxbike::controllerStateChanged);
    connect(m_control, &QLowEnergyController::connected, this, [this]() { m_control->discoverServices(); });
    connect(m_control, &QLowEnergyController::disconnected, this, &xcxbike::disconnected);
    m_control->connectToDevice();
}

void xcxbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("XCX controller state") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        notificationEnabled = false;
        lastTelemetryTime = QDateTime();
        m_control->connectToDevice();
    }
}

bool xcxbike::connected() {
    return m_control && m_control->state() == QLowEnergyController::DiscoveredState && notificationEnabled;
}

uint16_t xcxbike::watts() { return Cadence.value() == 0 ? 0 : static_cast<uint16_t>(m_watt.value()); }
