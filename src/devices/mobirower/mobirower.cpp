#include "mobirower.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

mobirower::mobirower(bool noWriteResistance, bool noHeartService) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &mobirower::update);
    refresh->start(200ms);
}

void mobirower::update() {
    if (m_control == nullptr)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
        gattCommunicationChannelService && gattNotifyCharacteristic.isValid() && initDone) {
        update_metrics(true, watts());

        if (requestStart != -1) {
            qDebug() << QStringLiteral("starting...");
            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            qDebug() << QStringLiteral("stopping...");
            requestStop = -1;
        }
    }
}

void mobirower::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void mobirower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << QStringLiteral(" << ") + newValue.toHex(' ');

    // Validate packet: 13 bytes, starts with 0xab 0x04
    if (newValue.length() < 13 ||
        (uint8_t)newValue.at(0) != 0xab ||
        (uint8_t)newValue.at(1) != 0x04) {
        qDebug() << QStringLiteral("Invalid packet format");
        return;
    }

    // Parse power from bytes 9-10 (big-endian uint16)
    uint16_t power = ((uint8_t)newValue.at(9) << 8) | (uint8_t)newValue.at(10);

    // Parse stroke count from bytes 11-12 (big-endian uint16)
    uint16_t strokeCount = ((uint8_t)newValue.at(11) << 8) | (uint8_t)newValue.at(12);

    // Calculate cadence from stroke delta
    double timeDelta = lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime());
    if (timeDelta > 0 && strokeCount >= lastStrokeCount) {
        uint16_t strokeDelta = strokeCount - lastStrokeCount;
        // Convert to strokes per minute (SPM)
        double cadence = (strokeDelta / (timeDelta / 60000.0));
        if (cadence < 200) { // sanity check
            Cadence = cadence;
        }
    }
    lastStrokeCount = strokeCount;

    m_watt = power;
    StrokesCount = strokeCount;

    // Calculate speed from strokes (standard rower formula)
    // Using a simplified formula: speed in km/h derived from cadence
    if (Cadence.value() > 0) {
        // Typical rower: ~10m per stroke at normal pace
        // Speed = (cadence * meters_per_stroke * 60) / 1000 for km/h
        double metersPerStroke = 8.0; // approximate
        Speed = (Cadence.value() * metersPerStroke * 60.0) / 1000.0;
    } else {
        Speed = 0;
    }

    StrokesLength =
        ((Speed.value() / 60.0) * 1000.0) /
        Cadence.value(); // this is just to fill the tile

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime()))));
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            update_hr_from_external();
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    bool virtual_device_rower =
        settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
    if (ios_peloton_workaround && cadence && !virtual_device_rower && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    qDebug() << QStringLiteral("Current Power: ") + QString::number(m_watt.value());
    qDebug() << QStringLiteral("Current Stroke Count: ") + QString::number(StrokesCount.value());
    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void mobirower::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // Find the notify characteristic (0xffe4)
        QBluetoothUuid notifyCharUuid((quint16)0xffe4);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(notifyCharUuid);

        if (!gattNotifyCharacteristic.isValid()) {
            qDebug() << QStringLiteral("gattNotifyCharacteristic not valid, trying to find by properties");
            auto characteristics_list = gattCommunicationChannelService->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("c -> ") << c.uuid() << c.properties();
                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    gattNotifyCharacteristic = c;
                    break;
                }
            }
        }

        if (!gattNotifyCharacteristic.isValid()) {
            qDebug() << QStringLiteral("gattNotifyCharacteristic still not valid");
            return;
        }

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &mobirower::characteristicChanged);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &mobirower::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &mobirower::descriptorWritten);

        // ******************************************* virtual bike/rower init *************************************
        if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            && !h
#endif
#endif
        ) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_rower =
                settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence && !virtual_device_rower) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                if (!virtual_device_rower) {
                    qDebug() << QStringLiteral("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    qDebug() << QStringLiteral("creating virtual rower interface...");
                    auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                    this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
                }
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void mobirower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');

    initDone = true;
    emit connectedAndDiscovered();
}

void mobirower::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    // Service UUID 0xffe0
    QBluetoothUuid serviceUuid((quint16)0xffe0);

    gattCommunicationChannelService = m_control->createServiceObject(serviceUuid);
    if (!gattCommunicationChannelService) {
        qDebug() << "service 0xffe0 not found, trying to find any service";
        auto services = m_control->services();
        for (const QBluetoothUuid &s : qAsConst(services)) {
            qDebug() << QStringLiteral("service ") << s.toString();
        }
        if (!services.isEmpty()) {
            gattCommunicationChannelService = m_control->createServiceObject(services.first());
        }
    }

    if (!gattCommunicationChannelService) {
        qDebug() << "no service found";
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &mobirower::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void mobirower::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("mobirower::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void mobirower::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << "mobirower::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString();
}

void mobirower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << "Found new device: " + device.name() + " (" + device.address().toString() + ')';
    bluetoothDevice = device;

    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &mobirower::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &mobirower::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &mobirower::error);
    connect(m_control, &QLowEnergyController::stateChanged, this, &mobirower::controllerStateChanged);

    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, [this](QLowEnergyController::Error error) {
                Q_UNUSED(error);
                Q_UNUSED(this);
                qDebug() << QStringLiteral("Cannot connect to remote device.");
                emit disconnected();
            });
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        Q_UNUSED(this);
        qDebug() << QStringLiteral("Controller connected. Search services...");
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        Q_UNUSED(this);
        qDebug() << QStringLiteral("LowEnergy controller disconnected");
        emit disconnected();
    });

    // Connect
    m_control->connectToDevice();
    return;
}

bool mobirower::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t mobirower::watts() {
    return m_watt.value();
}

void mobirower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
