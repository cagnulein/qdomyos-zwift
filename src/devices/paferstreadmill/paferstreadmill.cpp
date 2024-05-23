#include "paferstreadmill.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

paferstreadmill::paferstreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                 double forceInitInclination) {

#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif

    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0)
        lastSpeed = forceInitSpeed;

    if (forceInitInclination > 0)
        lastInclination = forceInitInclination;

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &paferstreadmill::update);
    refresh->start(500ms);
}

void paferstreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &paferstreadmill::packetReceived, &loop, &QEventLoop::quit);
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
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    // packets sent from the characChanged event, i don't want to block everything
    if (wait_for_response) {
        loop.exec();

        if (timeout.isActive() == false)
            emit debug(QStringLiteral(" exit for timeout"));
    }
}

void paferstreadmill::updateDisplay(uint16_t elapsed) {
    Q_UNUSED(elapsed);
    uint8_t noOpData[] = {0x55, 0x0d, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"));
}

void paferstreadmill::forceIncline(double requestIncline) {
    uint8_t incline[] = {0x55, 0x11, 0x01, 0x06};
    incline[3] = (uint8_t)requestIncline;
    writeCharacteristic(incline, sizeof(incline), QStringLiteral("forceIncline ") + QString::number(requestIncline));
}

double paferstreadmill::minStepInclination() { return 1.0; }
double paferstreadmill::minStepSpeed() { return 1.0; }

void paferstreadmill::forceSpeed(double requestSpeed) {
    uint8_t speed[] = {0x55, 0x0f, 0x02, 0x08, 0x00};
    speed[3] = (uint8_t)requestSpeed;
    writeCharacteristic(speed, sizeof(speed), QStringLiteral("forceSpeed ") + QString::number(requestSpeed));
}

void paferstreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void paferstreadmill::update() {
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
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &paferstreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &paferstreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
                QThread::msleep(400);
                forceIncline(Inclination.value());
            }
            requestSpeed = -1;
        } else if (requestInclination != -100) {
            if (requestInclination < 0)
                requestInclination = 0;
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                forceSpeed(Speed.value());
                QThread::msleep(400);
                forceIncline(requestInclination);
            }
            requestInclination = -100;
        } else if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            lastStart = QDateTime::currentMSecsSinceEpoch();
            uint8_t start[] = {0x55, 0x0a, 0x01, 0x01};
            writeCharacteristic(start, sizeof(start), "start", false, true);
            if (lastSpeed == 0.0) {
                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();
        } else if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            uint8_t stop[] = {0x55, 0x0a, 0x01, 0x02};
            writeCharacteristic(stop, sizeof(stop), "stop", false, true);
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape", false, true);
            requestStop = -1;
            lastStop = QDateTime::currentMSecsSinceEpoch();
        } else if (sec1Update++ >= (400 / refresh->interval())) {
            updateDisplay(elapsed.value());
            sec1Update = 0;
        }
    }
}

void paferstreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void paferstreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));

    emit packetReceived();

    if ((newValue.length() != 13))
        return;

    double speed = GetSpeedFromPacket(value);
    double incline = GetInclinationFromPacket(value);

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

    cadenceFromAppleWatch();

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current incline: ") + QString::number(incline));

    if (Speed.value() != speed) {
        emit speedChanged(speed);
    }
    Speed = speed;
    if (Inclination.value() != incline) {
        emit inclinationChanged(0.0, incline);
    }
    Inclination = incline;

    if (speed > 0) {
        lastSpeed = speed;
        lastInclination = incline;
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
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        Distance += ((Speed.value() / 3600.0) /
                     (1000.0 / (lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }

    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    lastTimeCharacteristicChanged = QDateTime::currentDateTime();
    firstCharacteristicChanged = false;
}

double paferstreadmill::GetSpeedFromPacket(const QByteArray &packet) {
    uint8_t convertedData = (uint8_t)packet.at(9);
    uint8_t convertedDataDecimal = (uint8_t)packet.at(10);
    double dataDecimal = ((double)convertedDataDecimal) / 100.0;
    double data = (double)convertedData + dataDecimal;

    // this treadmill always sends speed 1 even if the tape is stopped
    if (data == 1)
        data = 0;

    return data;
}

double paferstreadmill::GetInclinationFromPacket(const QByteArray &packet) {
    uint16_t convertedData = packet.at(11);
    double data = convertedData;

    return data;
}

void paferstreadmill::btinit(bool startTape) {
    Q_UNUSED(startTape)
    QSettings settings;
    bool pafers_treadmill_bh_iboxster_plus =
        settings
            .value(QZSettings::pafers_treadmill_bh_iboxster_plus, QZSettings::default_pafers_treadmill_bh_iboxster_plus)
            .toBool();
    uint8_t initData1[] = {0x55, 0xbb, 0x01, 0xff};
    uint8_t initData2[] = {0x55, 0x0c, 0x01, 0xff};
    uint8_t initData3[] = {0x55, 0x1f, 0x01, 0xff};
    uint8_t initData3b[] = {0x55, 0x0d, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t initData4[] = {0x55, 0x0a, 0x01, 0x02};
    uint8_t initData5[] = {0x55, 0x01, 0x06, 0x39, 0x01, 0x32, 0x19, 0xa5, 0x32};
    uint8_t initData6[] = {0x55, 0x15, 0x01, 0x00};
    uint8_t initData7[] = {0x55, 0x0f, 0x02, 0x01, 0x00};
    uint8_t initData7b[] = {0x55, 0x0f, 0x02, 0x00, 0x0a};
    uint8_t initData8[] = {0x55, 0x11, 0x01, 0x00};
    uint8_t initData9[] = {0x55, 0x08, 0x01, 0x01};
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    if (pafers_treadmill_bh_iboxster_plus)
        writeCharacteristic(initData3b, sizeof(initData3b), QStringLiteral("init"), false, true);
    writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
    writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
    writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
    if (pafers_treadmill_bh_iboxster_plus)
        writeCharacteristic(initData7b, sizeof(initData7b), QStringLiteral("init"), false, true);
    else
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, true);
    writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, true);

    initDone = true;
}

void paferstreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::ServiceDiscovered) {
        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("72d70002-501f-46f7-95f9-23846ee1aba3"));
        QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("72d70003-501f-46f7-95f9-23846ee1aba3"));

        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &paferstreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &paferstreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &paferstreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &paferstreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void paferstreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void paferstreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void paferstreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("72d70001-501f-46f7-95f9-23846ee1aba3"));
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &paferstreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void paferstreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("paferstreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void paferstreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("paferstreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void paferstreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &paferstreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &paferstreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &paferstreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &paferstreadmill::controllerStateChanged);

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

void paferstreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

bool paferstreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

bool paferstreadmill::autoPauseWhenSpeedIsZero() {
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool paferstreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}
