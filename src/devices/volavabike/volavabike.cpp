#include "volavabike.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

// 128-bit service UUID: 00010203-0405-0607-0809-0a0b0c0d1910
static const QString VOLAVA_SERVICE_UUID    = QStringLiteral("00010203-0405-0607-0809-0a0b0c0d1910");
// Notify characteristic:  00010203-0405-0607-0809-0a0b0c0d2b10
static const QString VOLAVA_NOTIFY_UUID     = QStringLiteral("00010203-0405-0607-0809-0a0b0c0d2b10");
// Write characteristic:   00010203-0405-0607-0809-0a0b0c0d2b11  (WriteNoResponse)
static const QString VOLAVA_WRITE_UUID      = QStringLiteral("00010203-0405-0607-0809-0a0b0c0d2b11");

volavabike::volavabike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                       double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &volavabike::update);
    refresh->start(200ms);
}

void volavabike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info,
                                     bool disable_log, bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged,
                &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten,
                &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (!gattCommunicationChannelService ||
        gattCommunicationChannelService->state() != QLowEnergyService::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("volavabike::writeCharacteristic: connection closed");
        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("volavabike::writeCharacteristic: invalid characteristic");
        return;
    }

    if (writeBuffer)
        delete writeBuffer;
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic,
                                                             *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    if (!disable_log)
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info;

    loop.exec();
}

// Init sequence observed in btsnoop:
//   1. AA 0F 8A 03 [10 bytes user profile] CS
//   2. AA 06 80 E1 00 [CS]
// Both frames use checksum = (sum of all bytes) & 0xFF
void volavabike::btinit() {
    // User profile frame (values from captured trace: male, ~32y, 80kg, 48cm?)
    // We keep the exact bytes from the trace for now.
    uint8_t userProfile[] = {0xAA, 0x0F, 0x8A, 0x03,
                             0x01, 0x05, 0x02, 0x09, 0x06, 0x20, 0x50, 0x30, 0x80, 0x20,
                             0x9D};
    writeCharacteristic(userProfile, sizeof(userProfile), QStringLiteral("userProfile"));

    QThread::msleep(200);

    // Start command
    uint8_t startCmd[] = {0xAA, 0x06, 0x80, 0xE1, 0x00, 0x11};
    writeCharacteristic(startCmd, sizeof(startCmd), QStringLiteral("startCmd"));

    initDone = true;
    qDebug() << QStringLiteral("volavabike::btinit done");
}

void volavabike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() &&
               m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotifyCharacteristic.isValid() && initDone) {
        update_metrics(true, watts());

        if (requestResistance != -1) {
            requestResistance = -1;
        }
        if (requestStart != -1) {
            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            requestStop = -1;
        }
    }
}

void volavabike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("volavabike::serviceDiscovered ") + gatt.toString();
}

void volavabike::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                       const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name)
            .toString();

    qDebug() << QStringLiteral("volavabike <<") << newValue.toHex(' ');

    lastPacket = newValue;

    // Workout data frame: AA 10 00 80 04 01 PH PL 04 02 00 CC 02 03 XX CS
    if (newValue.length() < 16 ||
        (uint8_t)newValue[0] != 0xAA ||
        (uint8_t)newValue[2] != 0x00 ||
        (uint8_t)newValue[3] != 0x80) {
        return;
    }

    uint16_t power      = ((uint8_t)newValue[6] << 8) | (uint8_t)newValue[7];
    uint8_t  cadence    = (uint8_t)newValue[11];
    uint8_t  resistance = (uint8_t)newValue[14];

    if (cadence > 0) {
        Cadence = cadence;
    } else {
        Cadence = 0;
    }

    Resistance = resistance;
    m_watt = power;

    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based)
             .toBool()) {
        // derive speed from cadence using a fixed gear ratio (same constant as iconsolebike)
        Speed = Cadence.value() * 0.38;
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }

    if (watts()) {
        KCal += ((((0.048 * (double)watts() + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / (double)lastRefreshCharacteristicChanged.msecsTo(now)));
    }

    Distance += (Speed.value() / 3600000.0) *
                (double)lastRefreshCharacteristicChanged.msecsTo(now);

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / ((double)Cadence.value() / 60.0));
    }

    lastRefreshCharacteristicChanged = now;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QLatin1String("Disabled")))
            update_hr_from_external();
    }

    qDebug() << QStringLiteral("Power: ") + QString::number(power)
             << QStringLiteral("Cadence: ") + QString::number(Cadence.value())
             << QStringLiteral("Resistance: ") + QString::number(Resistance.value())
             << QStringLiteral("Speed: ") + QString::number(Speed.value())
             << QStringLiteral("KCal: ") + QString::number(KCal.value())
             << QStringLiteral("Distance: ") + QString::number(Distance.value());
}

void volavabike::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("volavabike stateChanged ")
             + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state != QLowEnergyService::ServiceDiscovered)
        return;

    QBluetoothUuid notifyUuid(VOLAVA_NOTIFY_UUID);
    QBluetoothUuid writeUuid(VOLAVA_WRITE_UUID);

    gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(notifyUuid);
    gattWriteCharacteristic  = gattCommunicationChannelService->characteristic(writeUuid);

    if (!gattNotifyCharacteristic.isValid()) {
        qDebug() << QStringLiteral("volavabike: notify characteristic not found");
        return;
    }
    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("volavabike: write characteristic not found");
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged,
            this, &volavabike::characteristicChanged);
    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten,
            this, &volavabike::characteristicWritten);
    connect(gattCommunicationChannelService,
            static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(
                &QLowEnergyService::error),
            this, &volavabike::errorService);
    connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten,
            this, &volavabike::descriptorWritten);

    if (!firstStateChanged && !this->hasVirtualDevice()) {
        QSettings settings;
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled,
                           QZSettings::default_virtual_device_enabled).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence = settings.value(QZSettings::bike_cadence_sensor,
                                       QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround,
                           QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence) {
            h = new lockscreen();
            h->virtualbike_ios();
        } else
#endif
#endif
            if (virtual_device_enabled) {
            auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService,
                                               bikeResistanceOffset, bikeResistanceGain);
            connect(virtualBike, &virtualbike::changeInclination, this,
                    &volavabike::changeInclination);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;

    // Enable CCCD notifications
    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);
    gattCommunicationChannelService->writeDescriptor(
        gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration),
        descriptor);
}

void volavabike::descriptorWritten(const QLowEnergyDescriptor &descriptor,
                                   const QByteArray &newValue) {
    qDebug() << QStringLiteral("volavabike descriptorWritten ")
             + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' ');
    initRequest = true;
    emit connectedAndDiscovered();
}

void volavabike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                       const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("volavabike characteristicWritten ") + newValue.toHex(' ');
}

void volavabike::serviceScanDone(void) {
    qDebug() << QStringLiteral("volavabike serviceScanDone");

    QBluetoothUuid serviceUuid(VOLAVA_SERVICE_UUID);
    gattCommunicationChannelService = m_control->createServiceObject(serviceUuid);

    if (!gattCommunicationChannelService) {
        qDebug() << QStringLiteral("volavabike: service not found");
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged,
            this, &volavabike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void volavabike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("volavabike::errorService")
             + QString::fromLocal8Bit(metaEnum.valueToKey(err))
             + m_control->errorString();
}

void volavabike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("volavabike::error")
             + QString::fromLocal8Bit(metaEnum.valueToKey(err))
             + m_control->errorString();
}

void volavabike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("volavabike found: ") + device.name()
             + QStringLiteral(" (") + device.address().toString() + ')';

    bluetoothDevice = device;

    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &volavabike::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &volavabike::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(
                &QLowEnergyController::error),
            this, &volavabike::error);
    connect(m_control, &QLowEnergyController::stateChanged, this,
            &volavabike::controllerStateChanged);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(
                &QLowEnergyController::error),
            this, [this](QLowEnergyController::Error error) {
                Q_UNUSED(error);
                Q_UNUSED(this);
                qDebug() << QStringLiteral("volavabike: cannot connect to remote device.");
                emit disconnected();
            });
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        Q_UNUSED(this);
        qDebug() << QStringLiteral("volavabike: controller connected, discovering services...");
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        Q_UNUSED(this);
        qDebug() << QStringLiteral("volavabike: controller disconnected");
        emit disconnected();
    });

    m_control->connectToDevice();
}

bool volavabike::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t volavabike::watts() {
    if (Cadence.value() == 0)
        return 0;
    return m_watt.value();
}

void volavabike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("volavabike controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("volavabike: reconnecting...");
        initDone = false;
        m_control->connectToDevice();
    }
}
