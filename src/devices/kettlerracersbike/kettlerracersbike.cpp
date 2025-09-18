#include "kettlerracersbike.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif
#include "homeform.h"

#include <chrono>

using namespace std::chrono_literals;

kettlerracersbike::kettlerracersbike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &kettlerracersbike::update);
    refresh->start(200ms);
}

void kettlerracersbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {

    if(!gattKettlerService) {
        qDebug() << "gattKettlerService is null!";
        return;
    }

    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(gattKettlerService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattKettlerService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattKettlerService->writeCharacteristic(gattWriteCharKettlerId, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void kettlerracersbike::changePower(int32_t power) {
    RequestedPower = power;

    if (power < 0)
        power = 0;

    // Write power as 2-byte little-endian to 638a100e characteristic
    uint8_t powerData[2];
    powerData[0] = (uint8_t)(power & 0xFF);
    powerData[1] = (uint8_t)((power >> 8) & 0xFF);

    writeCharacteristic(powerData, sizeof(powerData), QStringLiteral("changePower ") + QString::number(power) + "W", false, false);
}

void kettlerracersbike::forceInclination(double inclination) {
    // Store inclination for SIM mode
    Inclination = inclination;

    // For grade mode, we need to send the grade value
    // Based on test logs, grade values are sent to the same characteristic as power
    // TODO: Analyze test logs to determine exact grade format
    // For now, using simple format similar to power
    int16_t gradeValue = (int16_t)(inclination * 100); // Convert percentage to integer format

    uint8_t gradeData[2];
    gradeData[0] = (uint8_t)(gradeValue & 0xFF);
    gradeData[1] = (uint8_t)((gradeValue >> 8) & 0xFF);

    writeCharacteristic(gradeData, sizeof(gradeData), QStringLiteral("forceInclination ") + QString::number(inclination) + "%", false, false);
}

uint16_t kettlerracersbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

double kettlerracersbike::bikeResistanceToPeloton(double resistance) {
    // Simple linear mapping for now
    return resistance;
}

resistance_t kettlerracersbike::pelotonToBikeResistance(int pelotonResistance) {
    // Simple linear mapping for now
    return pelotonResistance;
}

bool kettlerracersbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::ConnectedState;
}

void kettlerracersbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

void kettlerracersbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &kettlerracersbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &kettlerracersbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &kettlerracersbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &kettlerracersbike::controllerStateChanged);

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

        // Connect
        m_control->connectToDevice();
        return;
    }
}

void kettlerracersbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void kettlerracersbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    // Kettler custom service: 638af000-7bde-3e25-ffc5-9de9b2a0197a
    QBluetoothUuid kettlerServiceUuid(QStringLiteral("638af000-7bde-3e25-ffc5-9de9b2a0197a"));
    gattKettlerService = m_control->createServiceObject(kettlerServiceUuid);

    if (gattKettlerService == nullptr) {
        emit debug(QStringLiteral("invalid service") + kettlerServiceUuid.toString());
        return;
    }

    connect(gattKettlerService, &QLowEnergyService::stateChanged, this, &kettlerracersbike::stateChanged);
    gattKettlerService->discoverDetails();

    // CSC service: 00001816-0000-1000-8000-00805f9b34fb
    QBluetoothUuid cscServiceUuid(QStringLiteral("00001816-0000-1000-8000-00805f9b34fb"));
    gattCSCService = m_control->createServiceObject(cscServiceUuid);

    if (gattCSCService != nullptr) {
        connect(gattCSCService, &QLowEnergyService::stateChanged, this, &kettlerracersbike::stateChanged);
        gattCSCService->discoverDetails();
    }
}

void kettlerracersbike::error(QLowEnergyController::Error err) {
    qDebug() << QStringLiteral("controller ERROR ") << err;
}

void kettlerracersbike::errorService(QLowEnergyService::ServiceError err) {
    qDebug() << QStringLiteral("service ERROR ") << err;
}

void kettlerracersbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));
}

void kettlerracersbike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << newValue.toHex(' ');
}

void kettlerracersbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void kettlerracersbike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid().toString() << newValue.toHex(' ');
}

void kettlerracersbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("638a100e-7bde-3e25-ffc5-9de9b2a0197a")); // Power control
    QBluetoothUuid _gattNotifyCharacteristicKettlerRPMId(QStringLiteral("638a1002-7bde-3e25-ffc5-9de9b2a0197a")); // RPM
    QBluetoothUuid _gattNotifyCharacteristicKettler1Id(QStringLiteral("638a100c-7bde-3e25-ffc5-9de9b2a0197a"));
    QBluetoothUuid _gattNotifyCharacteristicKettler2Id(QStringLiteral("638a1010-7bde-3e25-ffc5-9de9b2a0197a"));
    QBluetoothUuid _gattWriteCharacteristicCSCId(QStringLiteral("00002a55-0000-1000-8000-00805f9b34fb"));
    QBluetoothUuid _gattNotifyCharacteristicCSCId(QStringLiteral("00002a5b-0000-1000-8000-00805f9b34fb"));

    qDebug() << QStringLiteral("BTLE stateChanged ") << state;

    if (state == QLowEnergyService::ServiceDiscovered) {

        // Kettler service characteristics
        if (gattKettlerService && gattKettlerService->serviceUuid().toString() == QStringLiteral("{638af000-7bde-3e25-ffc5-9de9b2a0197a}")) {
            emit debug(QStringLiteral("Kettler service connected"));

            // Power control characteristic
            gattWriteCharKettlerId = gattKettlerService->characteristic(_gattWriteCharacteristicId);
            if (!gattWriteCharKettlerId.isValid()) {
                emit debug(QStringLiteral("gattWriteCharKettlerId invalid"));
            }

            // Subscribe to RPM notifications
            auto rpmChar = gattKettlerService->characteristic(_gattNotifyCharacteristicKettlerRPMId);
            if (rpmChar.isValid()) {
                connect(gattKettlerService, &QLowEnergyService::characteristicChanged, this, &kettlerracersbike::characteristicChanged);
                auto descriptor = rpmChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (descriptor.isValid()) {
                    gattKettlerService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
                    emit debug(QStringLiteral("RPM notification subscribed"));
                }
            }

            // Subscribe to other Kettler characteristics
            auto char1 = gattKettlerService->characteristic(_gattNotifyCharacteristicKettler1Id);
            if (char1.isValid()) {
                auto descriptor = char1.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (descriptor.isValid()) {
                    gattKettlerService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
                    emit debug(QStringLiteral("Kettler char1 notification subscribed"));
                }
            }

            auto char2 = gattKettlerService->characteristic(_gattNotifyCharacteristicKettler2Id);
            if (char2.isValid()) {
                auto descriptor = char2.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (descriptor.isValid()) {
                    gattKettlerService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
                    emit debug(QStringLiteral("Kettler char2 notification subscribed"));
                }
            }
        }

        // CSC service characteristics
        if (gattCSCService && gattCSCService->serviceUuid().toString() == QStringLiteral("{00001816-0000-1000-8000-00805f9b34fb}")) {
            emit debug(QStringLiteral("CSC service connected"));

            auto cscChar = gattCSCService->characteristic(_gattNotifyCharacteristicCSCId);
            if (cscChar.isValid()) {
                connect(gattCSCService, &QLowEnergyService::characteristicChanged, this, &kettlerracersbike::characteristicChanged);
                auto descriptor = cscChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (descriptor.isValid()) {
                    gattCSCService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
                    emit debug(QStringLiteral("CSC notification subscribed"));
                }
            }
        }

        initDone = true;
    }
}

void kettlerracersbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' ') + QStringLiteral(" // ") + characteristic.uuid().toString());

    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("00002a5b-0000-1000-8000-00805f9b34fb"))) {
        // CSC measurement characteristic
        cscPacketReceived(newValue);
    } else if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("638a1002-7bde-3e25-ffc5-9de9b2a0197a"))) {
        // Kettler RPM characteristic
        kettlerPacketReceived(newValue);
    }

    QSettings settings;
    QString heartRateBeltName = settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        update_hr_from_external();
    }

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) &&
            heartRateBeltName != QStringLiteral("Disabled")) {
            update_hr_from_external();
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    lastRefreshCharacteristicChangedKettler = now;
}

void kettlerracersbike::cscPacketReceived(const QByteArray &packet) {
    // Standard CSC packet parsing
    if (packet.length() < 11)
        return;

    uint8_t flags = packet.at(0);

    if (flags & 0x01) { // Wheel revolution data present
        uint32_t wheelRevolutions = (packet.at(4) << 24) | (packet.at(3) << 16) | (packet.at(2) << 8) | packet.at(1);
        uint16_t wheelEventTime = (packet.at(6) << 8) | packet.at(5);

        // Calculate speed from wheel data if available
        static uint32_t lastWheelRevolutions = 0;
        static uint16_t lastWheelEventTime = 0;

        if (lastWheelRevolutions > 0) {
            uint32_t wheelRevDelta = wheelRevolutions - lastWheelRevolutions;
            uint16_t wheelTimeDelta = wheelEventTime - lastWheelEventTime;

            if (wheelTimeDelta > 0) {
                // Speed calculation (assuming wheel circumference of 2.1m)
                double speed = (wheelRevDelta * 2.1 * 1024.0) / (wheelTimeDelta * 3.6);
                Speed = speed;
            }
        }

        lastWheelRevolutions = wheelRevolutions;
        lastWheelEventTime = wheelEventTime;
    }

    if (flags & 0x02) { // Crank revolution data present
        uint16_t crankRevolutions = (packet.at(8) << 8) | packet.at(7);
        uint16_t crankEventTime = (packet.at(10) << 8) | packet.at(9);

        // Calculate cadence from CSC data
        if (oldCrankRevs > 0) {
            uint16_t crankRevsDelta = crankRevolutions - oldCrankRevs;
            uint16_t crankTimeDelta = crankEventTime - oldLastCrankEventTime;

            if (crankTimeDelta > 0 && crankRevsDelta > 0) {
                // Cadence calculation: (revolutions * 1024 * 60) / (time_delta)
                // 1024 is the time resolution, 60 converts to RPM
                double cadence = (double(crankRevsDelta) * 1024.0 * 60.0) / double(crankTimeDelta);
                if (cadence > 0 && cadence < 255) {
                    Cadence = cadence;
                    lastGoodCadence = QDateTime::currentDateTime();
                }
            }
        }

        oldCrankRevs = crankRevolutions;
        oldLastCrankEventTime = crankEventTime;
        CrankRevsRead = crankRevolutions;
    }
}

void kettlerracersbike::kettlerPacketReceived(const QByteArray &packet) {
    // Parse Kettler RPM data
    if (packet.length() >= 2) {
        uint16_t rpm = packet.at(0) | (packet.at(1) << 8);
        // RPM value is already in the correct format based on test logs
        Cadence = rpm;
        emit debug(QStringLiteral("Kettler RPM: ") + QString::number(rpm));
    }
}

void kettlerracersbike::powerPacketReceived(const QByteArray &b) {
    // Power data parsing if needed
    Q_UNUSED(b)
}

void kettlerracersbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattKettlerService && gattKettlerService->state() == QLowEnergyService::ServiceDiscovered &&
               gattWriteCharKettlerId.isValid() && initDone) {
        update_metrics(true, watts());

        // Check if we need to send power or grade commands
        if (requestPower != -1) {
            changePower(requestPower);
            requestPower = -1;
        }
        if (requestInclination != -100) {
            forceInclination(requestInclination);
            requestInclination = -100;
        }
    }
}

void kettlerracersbike::startDiscover() {
    // Called by bluetooth class
    qDebug() << QStringLiteral("kettlerracersbike::startDiscover");
}