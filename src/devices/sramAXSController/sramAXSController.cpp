#include "sramAXSController.h"

#include "sramAXSCrypto.h"
#include "homeform.h"

#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QMetaEnum>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSettings>
#include <QTimer>

namespace {
const quint16 SRAM_COMPANY_ID = 0x0933;
const int SRAM_MAX_GEAR = 24;

bool readVarint(const QByteArray &bytes, int &offset, quint64 &value) {
    value = 0;
    int shift = 0;
    while (offset < bytes.size() && shift <= 63) {
        const uint8_t byte = static_cast<uint8_t>(bytes.at(offset++));
        value |= static_cast<quint64>(byte & 0x7f) << shift;
        if ((byte & 0x80) == 0) return true;
        shift += 7;
    }
    return false;
}

bool skipField(const QByteArray &bytes, int &offset, int wireType) {
    quint64 length = 0;
    switch (wireType) {
    case 0: return readVarint(bytes, offset, length);
    case 1: offset += 8; return offset <= bytes.size();
    case 2: return readVarint(bytes, offset, length) && length <= static_cast<quint64>(bytes.size() - offset) &&
                   ((offset += static_cast<int>(length)) >= 0);
    case 5: offset += 4; return offset <= bytes.size();
    default: return false;
    }
}
}

sramaxscontroller::sramaxscontroller() {
    reconnectTimer.setSingleShot(true);
    connect(&reconnectTimer, &QTimer::timeout, this, [this]() {
        if (m_control && m_control->state() == QLowEnergyController::UnconnectedState)
            m_control->connectToDevice();
    });
}

const QBluetoothUuid sramaxscontroller::liveStateUuid() {
    return QBluetoothUuid(QStringLiteral("d905000b-90aa-4c7c-b036-1e01fb8eb7ee"));
}

const QBluetoothUuid sramaxscontroller::liveStateChangedUuid() {
    return QBluetoothUuid(QStringLiteral("d9050054-90aa-4c7c-b036-1e01fb8eb7ee"));
}

const QBluetoothUuid sramaxscontroller::drivetrainConfigUuid() {
    return QBluetoothUuid(QStringLiteral("d9050025-90aa-4c7c-b036-1e01fb8eb7ee"));
}

const QBluetoothUuid sramaxscontroller::bondServiceUuid() {
    return QBluetoothUuid(QStringLiteral("d905ee51-90aa-4c7c-b036-1e01fb8eb7ee"));
}

const QBluetoothUuid sramaxscontroller::bondCharacteristicUuid() {
    return QBluetoothUuid(QStringLiteral("d905ee52-90aa-4c7c-b036-1e01fb8eb7ee"));
}

const QBluetoothUuid sramaxscontroller::serialUuid() {
    return QBluetoothUuid(QStringLiteral("d905fe54-90aa-4c7c-b036-1e01fb8eb7ee"));
}

bool sramaxscontroller::isCompatibleDevice(const QBluetoothDeviceInfo &device) {
    const QString name = device.name().trimmed();
    const bool namedSram = name.startsWith(QStringLiteral("SRAM "), Qt::CaseInsensitive) && name.size() > 5;
    return namedSram || device.manufacturerIds().contains(SRAM_COMPANY_ID);
}

int sramaxscontroller::decodeRearGear(const QByteArray &plaintext) {
    int offset = 0;
    while (offset < plaintext.size()) {
        quint64 key = 0;
        if (!readVarint(plaintext, offset, key)) return -1;
        const int field = static_cast<int>(key >> 3);
        const int wireType = static_cast<int>(key & 7);
        if (field <= 0) return -1;
        if (field == 21 && wireType == 0) {
            quint64 value = 0;
            if (!readVarint(plaintext, offset, value) || value < 1 || value > SRAM_MAX_GEAR) return -1;
            return static_cast<int>(value);
        }
        if (!skipField(plaintext, offset, wireType)) return -1;
    }
    return -1;
}

int sramaxscontroller::decodeRearGearCount(const QByteArray &plaintext) {
    int offset = 0;
    while (offset < plaintext.size()) {
        quint64 key = 0;
        if (!readVarint(plaintext, offset, key)) return -1;
        const int field = static_cast<int>(key >> 3);
        const int wireType = static_cast<int>(key & 7);
        if (field == 24 && wireType == 0) {
            quint64 value = 0;
            if (!readVarint(plaintext, offset, value) || value < 1 || value > SRAM_MAX_GEAR) return -1;
            return static_cast<int>(value);
        }
        if (!skipField(plaintext, offset, wireType)) return -1;
    }
    return -1;
}

int sramaxscontroller::virtualGearDirectionForRearDelta(int delta) {
    if (delta == 0) return 0;
    return delta < 0 ? 1 : -1; // 1 = QZ gear up, -1 = QZ gear down
}

void sramaxscontroller::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    if (!isCompatibleDevice(device)) return;
    bluetoothDevice = device;
    deviceIdentifier = device.name().trimmed().mid(5).toUtf8();
    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &sramaxscontroller::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &sramaxscontroller::serviceScanDone);
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        emit debug(QStringLiteral("SRAM AXS connected; discovering services"));
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        emit debug(QStringLiteral("SRAM AXS disconnected; waiting to reconnect"));
        readyStarted = false;
        pairingStarted = false;
        lastRearGear = -1;
        services.clear();
        liveService = nullptr;
        bondService = nullptr;
        serialService = nullptr;
        configService = nullptr;
        emit disconnected();
        if (!reconnectTimer.isActive()) reconnectTimer.start(1500);
    });
    connect(m_control, &QLowEnergyController::stateChanged, this, &sramaxscontroller::controllerStateChanged);
    connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &sramaxscontroller::error);
    m_control->connectToDevice();
}

bool sramaxscontroller::connected() {
    return m_control && (m_control->state() == QLowEnergyController::DiscoveredState ||
                         m_control->state() == QLowEnergyController::ConnectedState);
}

void sramaxscontroller::serviceDiscovered(const QBluetoothUuid &gatt) {
    Q_UNUSED(gatt);
}

void sramaxscontroller::serviceScanDone() {
    if (!m_control) return;
    const QList<QBluetoothUuid> serviceUuids = m_control->services();
    for (const QBluetoothUuid &uuid : serviceUuids) {
        QLowEnergyService *service = m_control->createServiceObject(uuid, this);
        if (!service) continue;
        services.append(service);
        connect(service, &QLowEnergyService::stateChanged, this, &sramaxscontroller::stateChanged);
        connect(service, &QLowEnergyService::characteristicChanged, this, &sramaxscontroller::characteristicChanged);
        connect(service, &QLowEnergyService::characteristicRead, this, &sramaxscontroller::characteristicRead);
        connect(service, &QLowEnergyService::characteristicWritten, this, &sramaxscontroller::characteristicWritten);
        connect(service, &QLowEnergyService::descriptorWritten, this, &sramaxscontroller::descriptorWritten);
        connect(service, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &sramaxscontroller::errorService);
        service->discoverDetails();
    }
}

void sramaxscontroller::stateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::ServiceDiscovered) return;
    for (QLowEnergyService *service : qAsConst(services))
        if (service->state() != QLowEnergyService::ServiceDiscovered && service->state() != QLowEnergyService::InvalidService) return;
    if (readyStarted) return;

    for (QLowEnergyService *service : qAsConst(services)) {
        if (service->state() != QLowEnergyService::ServiceDiscovered) continue;
        for (const QLowEnergyCharacteristic &characteristic : service->characteristics()) {
            const QBluetoothUuid uuid = characteristic.uuid();
            if (uuid == liveStateUuid()) { liveService = service; liveCharacteristic = characteristic; }
            if (uuid == liveStateChangedUuid()) { liveStateChangedCharacteristic = characteristic; }
            if (uuid == drivetrainConfigUuid()) { configService = service; drivetrainConfigCharacteristic = characteristic; }
            if (uuid == bondCharacteristicUuid()) { bondService = service; bondCharacteristic = characteristic; }
            if (uuid == serialUuid()) { serialService = service; serialCharacteristic = characteristic; }

            if ((uuid == liveStateUuid() || uuid == liveStateChangedUuid() || uuid == bondCharacteristicUuid()) &&
                characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                const QByteArray ccc = (characteristic.properties() & QLowEnergyCharacteristic::Indicate) ? QByteArray::fromHex("0200") : QByteArray::fromHex("0100");
                service->writeDescriptor(characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), ccc);
            }
        }
    }
    readyStarted = true;
    QTimer::singleShot(500, this, &sramaxscontroller::beginReady);
}

void sramaxscontroller::beginReady() {
    if (!liveService || !liveCharacteristic.isValid()) {
        emit debug(QStringLiteral("SRAM AXS live-state characteristic not found"));
        return;
    }
    if (serialService && serialCharacteristic.properties() & QLowEnergyCharacteristic::Read) {
        serialService->readCharacteristic(serialCharacteristic);
        return;
    }
    loadStoredKey();
    if (deviceKey.isEmpty()) startPairing(); else startMonitoring();
}

void sramaxscontroller::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    if (characteristic.uuid() == serialUuid() && newValue.size() >= 4) {
        const uint32_t serial = static_cast<uint8_t>(newValue.at(0)) |
                                (static_cast<uint32_t>(static_cast<uint8_t>(newValue.at(1))) << 8) |
                                (static_cast<uint32_t>(static_cast<uint8_t>(newValue.at(2))) << 16) |
                                (static_cast<uint32_t>(static_cast<uint8_t>(newValue.at(3))) << 24);
        deviceIdentifier = QByteArray::number(serial);
        loadStoredKey();
        if (deviceKey.isEmpty()) startPairing(); else startMonitoring();
    } else if (characteristic.uuid() == liveStateUuid()) {
        processLiveFrame(newValue);
    } else if (characteristic.uuid() == drivetrainConfigUuid()) {
        const QByteArray plaintext = sramaxscrypto::eaxDecrypt(deviceKey, newValue.left(16), newValue.mid(16));
        const int count = sramaxscontroller::decodeRearGearCount(plaintext);
        if (count > 0) totalRearGears = count;
    }
}

void sramaxscontroller::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    emit packetReceived();
    if (characteristic.uuid() == bondCharacteristicUuid()) {
        bondResponses.enqueue(newValue);
        emit bondResponseReceived();
        return;
    }
    if (characteristic.uuid() == liveStateUuid() && newValue.size() >= 32) processLiveFrame(newValue);
    if (characteristic.uuid() == liveStateUuid() || characteristic.uuid() == liveStateChangedUuid())
        QTimer::singleShot(0, this, &sramaxscontroller::readLiveState);
}

void sramaxscontroller::readLiveState() {
    if (liveService && liveCharacteristic.isValid() && (liveCharacteristic.properties() & QLowEnergyCharacteristic::Read))
        liveService->readCharacteristic(liveCharacteristic);
}

void sramaxscontroller::processLiveFrame(const QByteArray &frame) {
    if (frame.size() < 32 || deviceKey.size() != 16) return;
    bool authenticated = false;
    const QByteArray plaintext = sramaxscrypto::eaxDecrypt(deviceKey, frame.left(16), frame.mid(16), &authenticated);
    if (authenticated) processPlaintext(plaintext);
}

void sramaxscontroller::processPlaintext(const QByteArray &plaintext) {
    const int rearGear = decodeRearGear(plaintext);
    if (rearGear < 1) return;
    if (lastRearGear < 0) { lastRearGear = rearGear; return; }
    const int delta = rearGear - lastRearGear;
    if (delta == 0 || qAbs(delta) > totalRearGears) return;
    lastRearGear = rearGear;
    const int direction = virtualGearDirectionForRearDelta(delta);
    for (int i = 0; i < qAbs(delta); ++i) {
        if (direction > 0) emit plus(); else emit minus();
    }
}

QString sramaxscontroller::deviceIdentifierString() const {
    return QString::fromUtf8(deviceIdentifier).trimmed().isEmpty()
               ? QStringLiteral("unknown") : QString::fromUtf8(deviceIdentifier).trimmed();
}

QString sramaxscontroller::keySettingName() const {
    QString id = deviceIdentifierString();
    id.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")), QStringLiteral("_"));
    return QStringLiteral("sram_axs_key/") + id;
}

void sramaxscontroller::loadStoredKey() {
    QSettings settings;
    const QByteArray stored = QByteArray::fromHex(settings.value(keySettingName()).toByteArray());
    deviceKey = stored.size() == 16 ? stored : QByteArray();
    emit debug(deviceKey.isEmpty() ? QStringLiteral("SRAM AXS key not found") : QStringLiteral("SRAM AXS key loaded"));
}

void sramaxscontroller::startMonitoring() {
    if (deviceKey.size() != 16) return;
    emit debug(QStringLiteral("SRAM AXS drivetrain monitoring started"));
    if (drivetrainConfigCharacteristic.isValid() && (drivetrainConfigCharacteristic.properties() & QLowEnergyCharacteristic::Read))
        configService->readCharacteristic(drivetrainConfigCharacteristic);
    readLiveState();
}

void sramaxscontroller::writeBondValue(const QByteArray &value) {
    if (!bondService || !bondCharacteristic.isValid()) return;
    const QLowEnergyService::WriteMode mode = (bondCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                                                  ? QLowEnergyService::WriteWithoutResponse : QLowEnergyService::WriteWithResponse;
    if (mode == QLowEnergyService::WriteWithResponse) {
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        const QMetaObject::Connection connection = connect(bondService, &QLowEnergyService::characteristicWritten, &loop,
                                                            [&](const QLowEnergyCharacteristic &characteristic, const QByteArray &) {
                                                                if (characteristic.uuid() == bondCharacteristicUuid()) loop.quit();
                                                            });
        connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        bondService->writeCharacteristic(bondCharacteristic, value, mode);
        timeout.start(2000);
        loop.exec();
        disconnect(connection);
        return;
    }
    bondService->writeCharacteristic(bondCharacteristic, value, mode);
}

QByteArray sramaxscontroller::waitForBondValue(int minimumLength, int timeoutMs, int exactLength) {
    QByteArray accumulated;
    auto take = [this, minimumLength, exactLength, &accumulated]() {
        while (!bondResponses.isEmpty()) {
            const QByteArray value = bondResponses.dequeue();
            if (exactLength != 0) {
                if (value.size() == exactLength) return value;
            } else {
                accumulated.append(value);
                if (accumulated.size() >= minimumLength) return accumulated.left(minimumLength);
            }
        }
        return QByteArray();
    };
    QByteArray result = take();
    if (!result.isEmpty()) return result;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    const QMetaObject::Connection connection = connect(this, &sramaxscontroller::bondResponseReceived, &loop, [&]() {
        result = take();
        if (!result.isEmpty()) loop.quit();
    });
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start(timeoutMs);
    loop.exec();
    disconnect(connection);
    return result;
}

void sramaxscontroller::startPairing() {
    if (pairingStarted || !bondService || !bondCharacteristic.isValid()) return;
    pairingStarted = true;
    emit debug(QStringLiteral("SRAM AXS has no stored key; hold the AXS button until its light blinks"));
    if (homeform::singleton()) homeform::singleton()->setToastRequested(QStringLiteral("Hold SRAM AXS button until it blinks"));
    QTimer::singleShot(3000, this, [this]() {
        bondResponses.clear();
        writeBondValue(QByteArray::fromHex("000102030405060708090a0b0c0d0e0f"));
        QByteArray privateKey(16, Qt::Uninitialized);
        for (int i = 0; i < privateKey.size(); ++i) privateKey[i] = static_cast<char>(QRandomGenerator::system()->generate() & 0xff);
        const QByteArray publicKey = sramaxscrypto::computePublicKey(privateKey);
        writeBondValue(publicKey);
        const QByteArray devicePublicKey = waitForBondValue(16, 5000, 16);
        const QByteArray sharedSecret = sramaxscrypto::computeSharedSecret(privateKey, devicePublicKey);
        const QByteArray transportBlob = waitForBondValue(48);
        bool authenticated = false;
        const QByteArray newKey = sramaxscrypto::decryptTransportedKey(sharedSecret, transportBlob, &authenticated);
        if (!authenticated || newKey.size() != 16) {
            emit debug(QStringLiteral("SRAM AXS pairing failed (no authenticated key)"));
            pairingStarted = false;
            return;
        }
        writeBondValue(QByteArray(1, static_cast<char>(0x73)));
        deviceKey = newKey;
        QSettings settings;
        settings.setValue(keySettingName(), deviceKey.toHex());
        emit debug(QStringLiteral("SRAM AXS key generated and stored"));
        startMonitoring();
    });
}

void sramaxscontroller::disconnectBluetooth() {
    reconnectTimer.stop();
    if (m_control) m_control->disconnectFromDevice();
}

void sramaxscontroller::controllerStateChanged(QLowEnergyController::ControllerState state) {
    if (state == QLowEnergyController::UnconnectedState && m_control && !reconnectTimer.isActive()) reconnectTimer.start(1500);
}

void sramaxscontroller::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic); Q_UNUSED(newValue);
}

void sramaxscontroller::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    Q_UNUSED(descriptor); Q_UNUSED(newValue);
}

void sramaxscontroller::error(QLowEnergyController::Error err) {
    Q_UNUSED(err);
    emit debug(QStringLiteral("SRAM AXS controller error: ") + m_control->errorString());
}

void sramaxscontroller::errorService(QLowEnergyService::ServiceError err) {
    Q_UNUSED(err);
    emit debug(QStringLiteral("SRAM AXS service error"));
}
