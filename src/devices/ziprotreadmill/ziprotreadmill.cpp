#include "ziprotreadmill.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

ziprotreadmill::ziprotreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                               double forceInitInclination) {

    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0)
        lastSpeed = forceInitSpeed;

    if (forceInitInclination > 0)
        lastInclination = forceInitInclination;

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &ziprotreadmill::update);
    refresh->start(500ms);
}

void ziprotreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                         bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &ziprotreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(400ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(400ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    // packets sent from the characChanged event, i don't want to block everything
    if (wait_for_response) {
        loop.exec();

        if (timeout.isActive() == false)
            emit debug(QStringLiteral(" exit for timeout"));
    }
}

void ziprotreadmill::updateDisplay(uint16_t elapsed) {
    Q_UNUSED(elapsed);
    // uint8_t noOpData[] = {0x55, 0x0d, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"));
}

void ziprotreadmill::forceIncline(double requestIncline) {
    // uint8_t incline[] = {0x55, 0x11, 0x01, 0x06};
    // incline[3] = (uint8_t)requestIncline;
    // writeCharacteristic(incline, sizeof(incline), QStringLiteral("forceIncline ") + QString::number(requestIncline));
}

double ziprotreadmill::minStepInclination() { return 1.0; }
double ziprotreadmill::minStepSpeed() { return 0.1; }

void ziprotreadmill::forceSpeed(double requestSpeed) {
    // uint8_t speed[] = {0x55, 0x0f, 0x02, 0x08, 0x00};
    // speed[3] = (uint8_t)requestSpeed;
    // writeCharacteristic(speed, sizeof(speed), QStringLiteral("forceSpeed ") + QString::number(requestSpeed));
}

void ziprotreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void ziprotreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    qDebug() << m_control->state() << bluetoothDevice.isValid() << gattCommunicationChannelService
             << gattWriteCharacteristic.isValid() << initDone << requestSpeed << requestInclination;

    if (initRequest) {
        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() && initDone) {
        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &ziprotreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &ziprotreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        uint8_t noop[] = {0xfb, 0x07, 0xa1, 0x02, 0x00, 0x00, 0x00, 0xaa, 0xfc};
        noop[5] = (uint8_t)(Speed.value() * 10.0);
        if (requestSpeed != -1) {
            noop[4] = 1; // force speed and inclination
            noop[5] = (uint8_t)(requestSpeed * 10.0);
            emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
            requestSpeed = -1;
        }
        noop[6] = (uint8_t)(Inclination.value());
        if (requestInclination != -100) {
            noop[4] = 1; // force speed and inclination
            noop[6] = (uint8_t)(requestInclination);
            emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
            requestInclination = -100;
        }
        noop[7] += noop[5] + noop[6] + noop[4];
        writeCharacteristic(noop, sizeof(noop), "noop", false, false);

        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            lastStart = QDateTime::currentMSecsSinceEpoch();
            uint8_t start[] = {0xfb, 0x05, 0xa2, 0x01, 0x01, 0xa9, 0xfc};
            writeCharacteristic(start, sizeof(start), "start", false, true);
            writeCharacteristic(start, sizeof(start), "start", false, true);
            if (lastSpeed == 0.0) {
                lastSpeed = 0.8;
            }
            requestStart = -1;
            emit tapeStarted();
        } else if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            uint8_t stop[] = {0xfb, 0x05, 0xa2, 0x04, 0x01, 0xac, 0xfc};
            writeCharacteristic(stop, sizeof(stop), "stop", false, true);
            writeCharacteristic(stop, sizeof(stop), "stop", false, true);
            writeCharacteristic(stop, sizeof(stop), "stop", false, true);
            requestStop = -1;
            lastStop = QDateTime::currentMSecsSinceEpoch();
        } else if (sec1Update++ >= (400 / refresh->interval())) {
            updateDisplay(elapsed.value());
            sec1Update = 0;
        }
    }
}

void ziprotreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void ziprotreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    emit packetReceived();

    if ((newValue.length() != 18))
        return;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        uint8_t heart = ((uint8_t)value.at(15));
        if (heart == 0 || disable_hr_frommachinery) {
            update_hr_from_external();
        } else {
            Heart = heart;
        }
    }

    double speed = ((double)((uint8_t)newValue.at(5))) / 10.0;
    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));

    Inclination = ((double)newValue.at(6));
    emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

    if (Speed.value() != speed) {
        emit speedChanged(speed);
    }
    Speed = speed;

    if (speed > 0) {
        lastSpeed = speed;
    }

    // this treadmill has a bug that always send 1km/h even if the tape is stopped
    if (speed > 1.0) {
        lastStart = 0;
    } else {
        lastStop = 0;
    }

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        Distance += ((Speed.value() / 3600.0) /
                     (1000.0 / (lastTimeCharacteristicChanged.msecsTo(now))));
    }

    cadenceFromAppleWatch();

    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    lastTimeCharacteristicChanged = now;
    firstCharacteristicChanged = false;
}

void ziprotreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)
    uint8_t initData1[] = {0xfb, 0x05, 0xa0, 0x00, 0x01, 0xa6, 0xfc};
    uint8_t initData2[] = {0xfb, 0x05, 0xa0, 0x01, 0x01, 0xa7, 0xfc};
    uint8_t initData3[] = {0xfb, 0x05, 0xa0, 0x02, 0x01, 0xa8, 0xfc};
    uint8_t initData4[] = {0xfb, 0x05, 0xa0, 0x03, 0x01, 0xa9, 0xfc};
    uint8_t initData5[] = {0xfb, 0x05, 0xa1, 0x00, 0x01, 0xa7, 0xfc};
    uint8_t initData6[] = {0xfb, 0x06, 0xa1, 0x00, 0x00, 0x00, 0xa7, 0xfc};
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
    writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
    writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
    writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);

    initDone = true;
}

void ziprotreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::ServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
        QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &ziprotreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &ziprotreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &ziprotreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &ziprotreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void ziprotreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void ziprotreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void ziprotreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &ziprotreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void ziprotreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("ziprotreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void ziprotreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("ziprotreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void ziprotreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &ziprotreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &ziprotreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &ziprotreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &ziprotreadmill::controllerStateChanged);

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

void ziprotreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool ziprotreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool ziprotreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool ziprotreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
