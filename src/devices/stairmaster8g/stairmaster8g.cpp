#include "stairmaster8g.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

stairmaster8g::stairmaster8g(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                             double forceInitInclination) {
    Q_UNUSED(forceInitSpeed);
    Q_UNUSED(forceInitInclination);

    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;
    this->pollDeviceTime = pollDeviceTime;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &stairmaster8g::update);
    refresh->start(pollDeviceTime);
}

bool stairmaster8g::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void stairmaster8g::btinit() {
    if (!gattCommunicationChannelService || !gattCommandCharacteristic.isValid())
        return;

    gattCommunicationChannelService->readCharacteristic(gattCommandCharacteristic);
    initDone = true;
}

void stairmaster8g::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void stairmaster8g::processPrimaryFrame(const QByteArray &frame) {
    if (frame.length() != 20 || (uint8_t)frame.at(0) != 0x53 || (uint8_t)frame.at(1) != 0xfb ||
        (uint8_t)frame.at(2) != 0x02) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    QSettings settings;

    const uint16_t totalSteps = (uint8_t)frame.at(11) | ((uint8_t)frame.at(12) << 8);
    StepCount = totalSteps;
    elevationAcc = StepCount.value() * 0.20;

    const qint64 nowMs = now.toMSecsSinceEpoch();
    cadenceSamples.enqueue(qMakePair(nowMs, (int)totalSteps));
    while (cadenceSamples.size() > 2 && cadenceSamples.head().first < nowMs - 5000) {
        cadenceSamples.dequeue();
    }

    if (cadenceSamples.size() >= 2) {
        const auto oldest = cadenceSamples.head();
        const auto newest = cadenceSamples.last();
        const qint64 elapsedMs = newest.first - oldest.first;
        if (elapsedMs > 0) {
            Cadence = ((double)(newest.second - oldest.second) * 60000.0) / (double)elapsedMs;
        }
    } else if (totalSteps == 0) {
        Cadence = 0;
    }

    Speed = Cadence.value() / 3.2;

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) {
            KCal += ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(now))));
        }
        Distance += ((Speed.value() / 3600.0) / (1000.0 / (lastTimeCharacteristicChanged.msecsTo(now))));
    }

    lastTimeCharacteristicChanged = now;
    firstCharacteristicChanged = false;
}

void stairmaster8g::processSecondaryFrame(const QByteArray &frame) {
    if (frame.length() != 14 || (uint8_t)frame.at(0) != 0x26 || (uint8_t)frame.at(1) != 0x9c ||
        (uint8_t)frame.at(2) != 0x06) {
        return;
    }
}

void stairmaster8g::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattCommandCharacteristic.isValid() && gattDataCharacteristic.isValid() && initDone) {
        QSettings settings;

        if (!this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &stairmaster8g::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &stairmaster8g::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &stairmaster8g::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
            }
        }

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));
    }
}

void stairmaster8g::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void stairmaster8g::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("a026e01e-0a7d-4ab3-97fa-f1500f9feb8b"))) {
        emit packetReceived();
        return;
    }

    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("a026e01d-0a7d-4ab3-97fa-f1500f9feb8b"))) {
        if (newValue.length() == 20) {
            processPrimaryFrame(newValue);
        } else if (newValue.length() == 14) {
            processSecondaryFrame(newValue);
        }
    }
}

void stairmaster8g::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid gattCommandCharacteristicId(QStringLiteral("a026e01e-0a7d-4ab3-97fa-f1500f9feb8b"));
    QBluetoothUuid gattDataCharacteristicId(QStringLiteral("a026e01d-0a7d-4ab3-97fa-f1500f9feb8b"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        gattCommandCharacteristic = gattCommunicationChannelService->characteristic(gattCommandCharacteristicId);
        gattDataCharacteristic = gattCommunicationChannelService->characteristic(gattDataCharacteristicId);

        Q_ASSERT(gattCommandCharacteristic.isValid());
        Q_ASSERT(gattDataCharacteristic.isValid());

        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &stairmaster8g::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &stairmaster8g::characteristicWritten);
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &stairmaster8g::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattCommandCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattDataCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

        initRequest = true;
    }
}

void stairmaster8g::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    emit connectedAndDiscovered();
}

void stairmaster8g::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void stairmaster8g::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        qDebug() << s << "service found!";
    }

    QBluetoothUuid gattCommunicationChannelServiceId(QStringLiteral("a026ee07-0a7d-4ab3-97fa-f1500f9feb8b"));

    gattCommunicationChannelService = m_control->createServiceObject(gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << "invalid service";
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &stairmaster8g::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void stairmaster8g::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("stairmaster8g::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void stairmaster8g::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("stairmaster8g::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void stairmaster8g::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("stairmaster8g::controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState) {
        emit disconnected();
    }
}

void stairmaster8g::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;

    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &stairmaster8g::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &stairmaster8g::serviceScanDone);
    connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(error(QLowEnergyController::Error)));
    connect(m_control, &QLowEnergyController::stateChanged, this, &stairmaster8g::controllerStateChanged);

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
        emit debug(QStringLiteral("QLowEnergyController disconnected"));
        emit disconnected();
    });

    m_control->connectToDevice();
}

void stairmaster8g::startDiscover() { m_control->discoverServices(); }
