#include "sunnyfitstepper.h"
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
#include <chrono>

using namespace std::chrono_literals;

sunnyfitstepper::sunnyfitstepper(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                 double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;
    this->pollDeviceTime = pollDeviceTime;

    refresh = new QTimer(this);
    initDone = false;
    frameBuffer.clear();
    expectingSecondPart = false;

    connect(refresh, &QTimer::timeout, this, &sunnyfitstepper::update);
    refresh->start(pollDeviceTime);
}

bool sunnyfitstepper::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void sunnyfitstepper::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &sunnyfitstepper::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("writeCharacteristic error because the connection is closed"));
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void sunnyfitstepper::sendPoll() {
    // Alternate between two poll commands
   
    counterPoll++;
}

void sunnyfitstepper::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void sunnyfitstepper::processDataFrame(const QByteArray &completeFrame) {
    if (completeFrame.length() != 32) {
        qDebug() << "ERROR: Frame length is not 32 bytes:" << completeFrame.length();
        return;
    }

    if ((uint8_t)completeFrame.at(0) != 0x5a) {
        qDebug() << "ERROR: Frame doesn't start with 0x5a";
        return;
    }

    if ((uint8_t)completeFrame.at(1) != 0x05) {
        qDebug() << "WARNING: Expected 0x05 at byte 1, got:" << QString::number((uint8_t)completeFrame.at(1), 16);
    }

    QDateTime now = QDateTime::currentDateTime();
    QSettings settings;

    // Extract cadence (bytes 6-7, little-endian)
    uint16_t rawCadence = ((uint8_t)completeFrame.at(7) << 8) | (uint8_t)completeFrame.at(6);
    Cadence = (double)rawCadence;

    // Extract step count (bytes 10-12, little-endian)
    uint32_t steps = ((uint32_t)(uint8_t)completeFrame.at(12) << 16) |
                     ((uint32_t)(uint8_t)completeFrame.at(11) << 8) |
                     (uint32_t)(uint8_t)completeFrame.at(10);
    StepCount = steps;

    // Calculate elevation manually (0.2 meters per step)
    elevationAcc = (double)steps * 0.20;

    // Calculate speed from cadence (stairclimber convention)
    Speed = Cadence.value() / 3.2;

    qDebug() << QStringLiteral("Current Cadence (SPM): ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current StepCount: ") + QString::number(StepCount.value());
    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Elevation: ") + QString::number(elevationAcc.value());

    // Calculate metrics
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

    qDebug() << QStringLiteral("Current Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current KCal: ") + QString::number(KCal.value());
    qDebug() << QStringLiteral("Current Watt: ") +
                    QString::number(watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

    if (m_control->error() != QLowEnergyController::NoError)
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();

    lastTimeCharacteristicChanged = now;
    firstCharacteristicChanged = false;
}

void sunnyfitstepper::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() && gattNotify1Characteristic.isValid() &&
               gattNotify4Characteristic.isValid() && initDone) {
        QSettings settings;

        // *********** virtual treadmill init *************************************
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
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &sunnyfitstepper::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &sunnyfitstepper::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &sunnyfitstepper::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
            }
        }
        // ************************************************************

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // Send poll every 2 seconds
        if (sec1Update++ >= (2000 / refresh->interval())) {
            sec1Update = 0;
            //sendPoll();
        }
    }
}

void sunnyfitstepper::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void sunnyfitstepper::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    // Handle command responses (Notify 1)
    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("fd710003-e950-458e-8a4d-a1cbc5aa4cce"))) {
        qDebug() << "Command response:" << newValue.toHex(' ');
        emit packetReceived();
        return;
    }

    // Handle main data stream (Notify 4) - SPLIT FRAME LOGIC
    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("fd710006-e950-458e-8a4d-a1cbc5aa4cce"))) {
        // First part: 20 bytes starting with 0x5a
        if (newValue.length() == 20 && (uint8_t)newValue.at(0) == 0x5a) {
            frameBuffer.clear();
            frameBuffer.append(newValue);
            expectingSecondPart = true;
            qDebug() << "First part of frame received (20 bytes)";
            return;
        }

        // Second part: 12 bytes
        if (newValue.length() == 12 && expectingSecondPart) {
            frameBuffer.append(newValue);
            expectingSecondPart = false;

            if (frameBuffer.length() == 32) {
                emit debug(QStringLiteral(" << COMPLETE FRAME >> ") + frameBuffer.toHex(' '));
                processDataFrame(frameBuffer);
                frameBuffer.clear();
            } else {
                qDebug() << "ERROR: Complete frame size mismatch:" << frameBuffer.length();
                frameBuffer.clear();
            }
            return;
        }

        // Unexpected frame structure
        qDebug() << "Unexpected frame - length:" << newValue.length() << "expecting second part:" << expectingSecondPart;
        frameBuffer.clear();
        expectingSecondPart = false;
    }
}

void sunnyfitstepper::btinit() {
    uint8_t init1[] = {0x5a, 0x02, 0x00, 0x08, 0x07, 0xa0, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0xe6, 0xa5};
    uint8_t init2[] = {0x5a, 0x02, 0x00, 0x03, 0x02, 0xa3, 0x00, 0xaa, 0xa5};
    uint8_t init3[] = {0x5a, 0x02, 0x00, 0x03, 0x02, 0xb4, 0x00, 0xbb, 0xa5};
    uint8_t init4[] = {0x5a, 0x04, 0x00, 0x03, 0x02, 0xf1, 0x00, 0xfa, 0xa5};

    writeCharacteristic(init1, sizeof(init1), QStringLiteral("init1"), false, true);
    writeCharacteristic(init2, sizeof(init2), QStringLiteral("init2"), false, true);
    writeCharacteristic(init3, sizeof(init3), QStringLiteral("init3"), false, false);
    writeCharacteristic(init4, sizeof(init4), QStringLiteral("init4"), false, false);

    initDone = true;
}

void sunnyfitstepper::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("fd710002-e950-458e-8a4d-a1cbc5aa4cce"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("fd710003-e950-458e-8a4d-a1cbc5aa4cce"));
    QBluetoothUuid _gattNotify4CharacteristicId(QStringLiteral("fd710006-e950-458e-8a4d-a1cbc5aa4cce"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify4Characteristic = gattCommunicationChannelService->characteristic(_gattNotify4CharacteristicId);

        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify4Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &sunnyfitstepper::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &sunnyfitstepper::characteristicWritten);
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &sunnyfitstepper::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify4Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);

        initRequest = true;
    }
}

void sunnyfitstepper::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));
    
    emit connectedAndDiscovered();
}

void sunnyfitstepper::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void sunnyfitstepper::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        qDebug() << s << "service found!";
    }

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("fd710001-e950-458e-8a4d-a1cbc5aa4cce"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << "invalid service";
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &sunnyfitstepper::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void sunnyfitstepper::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("sunnyfitstepper::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void sunnyfitstepper::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("sunnyfitstepper::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void sunnyfitstepper::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("sunnyfitstepper::controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState) {
        emit disconnected();
    }
}

void sunnyfitstepper::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &sunnyfitstepper::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &sunnyfitstepper::serviceScanDone);
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(error(QLowEnergyController::Error)));
        connect(m_control, &QLowEnergyController::stateChanged, this, &sunnyfitstepper::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
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
}

void sunnyfitstepper::startDiscover() {
    m_control->discoverServices();
}
