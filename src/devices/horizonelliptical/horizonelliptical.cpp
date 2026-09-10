#include "horizonelliptical.h"
#include "homeform.h"
#include "qzsettings.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

horizonelliptical::horizonelliptical(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED, deviceType());
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    profile.testProfileCRC();
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &horizonelliptical::update);
    refresh->start(200ms);
}

void horizonelliptical::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                            uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                            bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (!service) {
        emit debug(QStringLiteral("no service for ") + info);
        return;
    }

    if (wait_for_response) {
        connect(this, &horizonelliptical::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(3000ms, &loop, &QEventLoop::quit);
    } else {
        connect(service, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(3000ms, &loop, &QEventLoop::quit);
    }

    if (characteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        service->writeCharacteristic(characteristic, QByteArray((const char *)data, data_len),
                                     QLowEnergyService::WriteWithoutResponse);
    } else {
        service->writeCharacteristic(characteristic, QByteArray((const char *)data, data_len));
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void horizonelliptical::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &horizonelliptical::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000ms, &loop, &QEventLoop::quit);
    loop.exec();
}

void horizonelliptical::btinit() {
    // The machine will not stream telemetry until the profile upload completes. The byte sequence
    // is identical to the treadmill's, so it comes from the shared table rather than being
    // duplicated here; see horizonprofile::handshakeFrames().
    profile.updateProfileCRC();

    const std::vector<horizonprofile::frame> frames = profile.handshakeFrames();
    int n = 0;
    for (const horizonprofile::frame &f : frames) {
        writeCharacteristic(gattCustomService, gattWriteCharCustomService, f.first, f.second,
                            QStringLiteral("init"), true, false);
        // The machine paces itself; give it a chance to answer every so often rather than after
        // every frame, which is what the treadmill driver does at its checkpoints.
        if ((++n % 40) == 0) {
            waitForAPacket();
            if (homeform::singleton())
                homeform::singleton()->setToastRequested(
                    QStringLiteral("Elliptical initialization in progress...%1%")
                        .arg((int)((100.0 * n) / frames.size())));
        }
    }

    initDone = true;
    initRequest = false;
    emit debug(QStringLiteral("init completed"));
}

void horizonelliptical::update() {
    if (m_control == nullptr)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest && firstStateChanged) {
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCustomService && initDone) {
        QSettings settings;
        update_metrics(true, watts());

        if (firstDistanceCalculated) {
            QDateTime now = QDateTime::currentDateTime();
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
            lastRefreshCharacteristicChanged = now;
        }
        firstDistanceCalculated = true;

        if (requestResistance != -1) {
            emit debug(QStringLiteral("resistance change is not supported by this machine"));
            requestResistance = -1;
        }
    }
}

void horizonelliptical::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

/**
 * The 0x12 telemetry frame. Offsets were established by sweeping one control at a time on a
 * 7.0 AE and correlating against its console:
 *   61..62  speed, little endian, /1000 -> mph
 *   63      inclination: steps by 5 per console level, caps at 100 -> 20 levels
 *   73      instantaneous power
 *   81      cadence in RPM, 1:1 with the console
 *   85      resistance, 1:1 with the console's 1..20
 */
void horizonelliptical::parseCustomFrame(const QByteArray &frame) {
    if (frame.length() <= 85)
        return;

    Speed = (((double)(((uint16_t)((uint8_t)frame.at(62)) << 8) | (uint16_t)((uint8_t)frame.at(61)))) / 1000.0) *
            1.60934;
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

    Inclination = (double)((uint8_t)frame.at(63)) / 5.0;
    emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

    double cadence = (uint8_t)frame.at(81);
    if (cadence > 0 && cadence < 250) {
        Cadence = cadence;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    }

    double resistance = (uint8_t)frame.at(85);
    if (resistance > 0 && resistance <= 40) {
        Resistance = resistance;
        m_pelotonResistance = pelotonToEllipticalResistance(Resistance.value());
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    }

    m_watt = (uint8_t)frame.at(73);
    emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
}

void horizonelliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));
    emit packetReceived();
    initPacketRecv = true;

    // Frames arrive in 20-byte chunks: 55 aa <seq16> <?> <type> <len16>, a 9-byte header followed
    // by `len` payload bytes (the 0x12 telemetry frame is 9 + 88 = 97).
    if (newValue.length() >= 2 && ((uint8_t)newValue.at(0)) == 0x55 && ((uint8_t)newValue.at(1)) == 0xAA)
        lastPacket.clear();
    lastPacket.append(newValue);

    if (lastPacket.length() < 9)
        return;
    uint16_t payload = (uint16_t)((uint8_t)lastPacket.at(6)) | ((uint16_t)((uint8_t)lastPacket.at(7)) << 8);
    uint16_t expected = 9 + payload;
    if (lastPacket.length() < expected)
        return;

    if (((uint8_t)lastPacket.at(5)) == 0x12)
        parseCustomFrame(lastPacket);
    lastPacket.clear();

    if (!heartRateBeltName.startsWith(QStringLiteral("Disabled")))
        return;
    update_hr_from_external();
}

void horizonelliptical::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattCustomService((quint16)0xFFF0);
    QBluetoothUuid _gattWriteCharCustomService((quint16)0xFFF3);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s == nullptr || s->state() != QLowEnergyService::ServiceDiscovered)
            continue;

        auto characteristics_list = s->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            if (s->serviceUuid() == _gattCustomService) {
                gattCustomService = s;
                if (c.uuid() == _gattWriteCharCustomService)
                    gattWriteCharCustomService = c;

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify &&
                    c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    notificationSubscribed++;
                }
            }
        }
    }

    firstStateChanged = 1;
}

void horizonelliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    if (notificationSubscribed)
        notificationSubscribed--;

    if (!notificationSubscribed) {
        if (homeform::singleton())
            homeform::singleton()->setToastRequested("Elliptical ready");
        initRequest = true;
        emit connectedAndDiscovered();
    }
}

void horizonelliptical::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;
    firstStateChanged = 0;

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &horizonelliptical::stateChanged);
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::characteristicChanged, this,
                &horizonelliptical::characteristicChanged);
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::descriptorWritten, this,
                &horizonelliptical::descriptorWritten);
        connect(gattCommunicationChannelService.constLast(),
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &horizonelliptical::errorService);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void horizonelliptical::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("horizonelliptical::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)));
}

void horizonelliptical::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("horizonelliptical::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)));
}

void horizonelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    bluetoothDevice = device;

    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &horizonelliptical::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &horizonelliptical::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &horizonelliptical::error);
    connect(m_control, &QLowEnergyController::stateChanged, this, &horizonelliptical::controllerStateChanged);

    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, [this](QLowEnergyController::Error error) {
                Q_UNUSED(error);
                Q_UNUSED(this);
                emit debug(QStringLiteral("Cannot connect to remote device."));
                emit disconnected();
            });
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        Q_UNUSED(this);
        emit debug(QStringLiteral("Controller connected. Search services..."));
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        Q_UNUSED(this);
        emit debug(QStringLiteral("LowEnergy controller disconnected"));
        emit disconnected();
    });

    m_control->connectToDevice();
}

bool horizonelliptical::connected() {
    if (m_control == nullptr)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void horizonelliptical::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
