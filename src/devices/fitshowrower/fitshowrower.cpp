#include "fitshowrower.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "qzsettings.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"

#include <QMetaEnum>
#include <QSettings>

namespace {
quint16 littleEndian16(const QByteArray &data, int offset) {
    return static_cast<quint8>(data.at(offset)) | (static_cast<quint16>(static_cast<quint8>(data.at(offset + 1))) << 8);
}
} // namespace

fitshowrower::fitshowrower(bool noWriteResistance, bool noHeartService)
    : noWriteResistance(noWriteResistance), noHeartService(noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    connect(&refresh, &QTimer::timeout, this, &fitshowrower::update);
    refresh.start(500);

    initializationFrames << QByteArray::fromHex("0241024303") << QByteArray::fromHex("0241024303")
                         << QByteArray::fromHex("0243014203") << QByteArray::fromHex("02424203")
                         << QByteArray::fromHex("0244014503") << QByteArray::fromHex("0244024603");
}

bool fitshowrower::isTopiomDeviceName(const QString &name) {
    return name.compare(QStringLiteral("FS-442900"), Qt::CaseInsensitive) == 0;
}

bool fitshowrower::validFrame(const QByteArray &frame) {
    if (frame.size() < 4 || static_cast<quint8>(frame.front()) != 0x02 || static_cast<quint8>(frame.back()) != 0x03) {
        return false;
    }
    quint8 checksum = 0;
    for (int i = 1; i < frame.size() - 2; ++i) {
        checksum ^= static_cast<quint8>(frame.at(i));
    }
    return checksum == static_cast<quint8>(frame.at(frame.size() - 2));
}

fitshowrower::Packet fitshowrower::parsePacket(const QByteArray &frame) {
    Packet packet;
    if (!validFrame(frame)) {
        return packet;
    }

    packet.command = static_cast<quint8>(frame.at(1));
    if (packet.command == 0x41 && frame.size() == 9 && static_cast<quint8>(frame.at(2)) == 0x02) {
        packet.subcommand = 0x02;
        packet.maxResistance = static_cast<quint8>(frame.at(3));
        packet.maxIncline = static_cast<quint8>(frame.at(4));
    } else if (packet.command == 0x42 && frame.size() == 5) {
        packet.status = static_cast<quint8>(frame.at(2));
    } else if (packet.command == 0x42 && frame.size() == 15) {
        packet.status = static_cast<quint8>(frame.at(2));
        packet.cadence = littleEndian16(frame, 6);
        packet.heartRate = static_cast<quint8>(frame.at(8));
        packet.power = littleEndian16(frame, 9) / 10.0;
    } else if (packet.command == 0x43 && frame.size() == 13 && static_cast<quint8>(frame.at(2)) == 0x01) {
        packet.subcommand = 0x01;
        packet.elapsedSeconds = littleEndian16(frame, 3);
        quint16 distance = littleEndian16(frame, 5);
        packet.distanceMeters = (distance & 0x7fff) * ((distance & 0x8000) ? 10 : 1);
        packet.calories = littleEndian16(frame, 7) / 10.0;
        packet.strokeCount = littleEndian16(frame, 9);
    } else if (packet.command == 0x44 && frame.size() >= 5) {
        packet.subcommand = static_cast<quint8>(frame.at(2));
    } else {
        return Packet();
    }
    packet.valid = true;
    return packet;
}

void fitshowrower::applyPacket(const Packet &packet) {
    if (packet.command == 0x41) {
        emit debug(QStringLiteral("FitShow capabilities: max resistance %1, max incline %2")
                       .arg(packet.maxResistance)
                       .arg(packet.maxIncline));
        return;
    }
    if (packet.command == 0x42) {
        const bool running = packet.status == 0x02;
        Cadence = running ? packet.cadence : 0;
        m_watt = running ? packet.power : 0;
        Speed = 0; // The Topiom speed field has no verified unit; distance comes from command 0x43.

        QSettings settings;
#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
            Heart = static_cast<quint8>(KeepAwakeHelper::heart());
        } else
#endif
            if (packet.heartRate &&
                settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
            Heart = packet.heartRate;
        }
        emit debug(QStringLiteral("FitShow status %1, cadence %2 SPM, power %3 W, heart %4")
                       .arg(packet.status)
                       .arg(Cadence.value())
                       .arg(m_watt.value())
                       .arg(Heart.value()));
    } else if (packet.command == 0x43) {
        if (packet.strokeCount > StrokesCount.value()) {
            lastStroke = QDateTime::currentDateTime();
        }
        elapsed = packet.elapsedSeconds;
        Distance = packet.distanceMeters / 1000.0;
        KCal = packet.calories;
        StrokesCount = packet.strokeCount;
        emit debug(QStringLiteral("FitShow cumulative: elapsed %1 s, distance %2 m, calories %3, strokes %4")
                       .arg(packet.elapsedSeconds)
                       .arg(packet.distanceMeters)
                       .arg(packet.calories)
                       .arg(packet.strokeCount));
    }
}

void fitshowrower::characteristicChanged(const QLowEnergyCharacteristic &, const QByteArray &value) {
    emit debug(QStringLiteral(" << ") + value.toHex(' '));
    const Packet packet = parsePacket(value);
    if (!packet.valid) {
        emit debug(QStringLiteral("Ignoring malformed FitShow frame: ") + value.toHex(' '));
        return;
    }
    applyPacket(packet);
}

void fitshowrower::writeFrame(const QByteArray &frame, const QString &description) {
    if (!communicationService || !writeCharacteristic.isValid()) {
        emit debug(QStringLiteral("Cannot write FitShow frame: FFF2 is unavailable"));
        return;
    }
    const auto mode = (writeCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                          ? QLowEnergyService::WriteWithoutResponse
                          : QLowEnergyService::WriteWithResponse;
    communicationService->writeCharacteristic(writeCharacteristic, frame, mode);
    emit debug(QStringLiteral(" >> ") + frame.toHex(' ') + QStringLiteral(" // ") + description);
}

void fitshowrower::update() {
    if (!m_control || m_control->state() == QLowEnergyController::UnconnectedState) {
        return;
    }
    if (!notificationsEnabled) {
        return;
    }
    if (initializationIndex < initializationFrames.size()) {
        writeFrame(initializationFrames.at(initializationIndex++), QStringLiteral("initialization"));
        initialized = initializationIndex == initializationFrames.size();
        return;
    }
    if (initialized) {
        writeFrame(pollStatus ? QByteArray::fromHex("02424203") : QByteArray::fromHex("0243014203"),
                   pollStatus ? QStringLiteral("live status poll") : QStringLiteral("cumulative data poll"));
        pollStatus = !pollStatus;
        update_metrics(false, 0);
    }
}

void fitshowrower::serviceStateChanged(QLowEnergyService::ServiceState state) {
    emit debug(QStringLiteral("FitShow service state: ") + QString::number(state));
    if (state != QLowEnergyService::ServiceDiscovered)
        return;
    writeCharacteristic =
        communicationService->characteristic(QBluetoothUuid(QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb")));
    notifyCharacteristic =
        communicationService->characteristic(QBluetoothUuid(QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb")));
    if (!writeCharacteristic.isValid() || !notifyCharacteristic.isValid()) {
        emit debug(QStringLiteral("FitShow FFF1 notify or FFF2 write characteristic is missing"));
        return;
    }
    connect(communicationService, &QLowEnergyService::characteristicChanged, this,
            &fitshowrower::characteristicChanged);
    connect(communicationService, &QLowEnergyService::descriptorWritten, this, &fitshowrower::descriptorWritten);
    connect(communicationService,
            static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error), this,
            &fitshowrower::serviceError);
    const QLowEnergyDescriptor ccc = notifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!ccc.isValid()) {
        emit debug(QStringLiteral("FitShow FFF1 CCC descriptor is missing"));
        return;
    }
    communicationService->writeDescriptor(ccc, QByteArray::fromHex("0100"));
}

void fitshowrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
    emit debug(QStringLiteral("descriptorWritten %1 %2").arg(descriptor.name(), QString(value.toHex(' '))));
    notificationsEnabled = true;
    createVirtualDevice();
    emit connectedAndDiscovered();
}

void fitshowrower::createVirtualDevice() {
    if (hasVirtualDevice())
        return;
    QSettings settings;
    if (!settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool())
        return;
    if (settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool()) {
        setVirtualDevice(new virtualrower(this, noWriteResistance, noHeartService), VIRTUAL_DEVICE_MODE::PRIMARY);
    } else {
        auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
        connect(virtualBike, &virtualbike::changeInclination, this, &fitshowrower::changeInclination);
        setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
    }
}

void fitshowrower::serviceScanDone() {
    communicationService =
        m_control->createServiceObject(QBluetoothUuid(QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb")), this);
    if (!communicationService) {
        emit debug(QStringLiteral("FitShow FFF0 service not found"));
        return;
    }
    connect(communicationService, &QLowEnergyService::stateChanged, this, &fitshowrower::serviceStateChanged);
    communicationService->discoverDetails();
}

void fitshowrower::serviceDiscovered(const QBluetoothUuid &service) {
    emit debug(QStringLiteral("serviceDiscovered ") + service.toString());
}

void fitshowrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    emit debug(QStringLiteral("Found Topiom rower: ") + device.name());
    m_control = QLowEnergyController::createCentral(device, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &fitshowrower::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &fitshowrower::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &fitshowrower::controllerError);
    connect(m_control, &QLowEnergyController::connected, m_control, &QLowEnergyController::discoverServices);
    connect(m_control, &QLowEnergyController::disconnected, this, &fitshowrower::disconnected);
    m_control->connectToDevice();
}

void fitshowrower::controllerError(QLowEnergyController::Error error) {
    emit debug(QStringLiteral("FitShow controller error %1: %2").arg(error).arg(m_control->errorString()));
}

void fitshowrower::serviceError(QLowEnergyService::ServiceError error) {
    emit debug(QStringLiteral("FitShow service error %1").arg(error));
}

bool fitshowrower::connected() { return m_control && m_control->state() == QLowEnergyController::DiscoveredState; }
