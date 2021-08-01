#include "horizontreadmill.h"

#include "ios/lockscreen.h"
#include "virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#include "keepawakehelper.h"
#include <chrono>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

horizontreadmill::horizontreadmill(bool noWriteResistance, bool noHeartService) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif

    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &horizontreadmill::update);
    refresh->start(200ms);
}
/*
void horizontreadmill::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool
wait_for_response)
{
    QEventLoop loop;
    QTimer timeout;
    if(wait_for_response)
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }
    else
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray((const char*)data,
data_len));

    if(!disable_log)
        debug(" >> " + QByteArray((const char*)data, data_len).toHex(' ') + " // " + info);

    loop.exec();
}*/

void horizontreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
    } else if (bluetoothDevice.isValid() //&&

               // m_control->state() == QLowEnergyController::DiscoveredState //&&
               // gattCommunicationChannelService &&
               // gattWriteCharacteristic.isValid() &&
               // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {

        QSettings settings;
        update_metrics(true, watts(settings.value(QStringLiteral("weight"), 75.0).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));

                double inc = Inclination.value();
                if (requestInclination != -1) {

                    inc = requestInclination;
                    requestInclination = -1;
                }
                forceSpeedOrIncline(requestSpeed, inc);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -1) {
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));

                double speed = currentSpeed().value();
                if (requestSpeed != -1) {

                    speed = requestSpeed;
                    requestSpeed = -1;
                }
                forceSpeedOrIncline(speed, requestInclination);
            }
            requestInclination = -1;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {

                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

            requestStop = -1;
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

void horizontreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline) {
    Q_UNUSED(requestSpeed)
    Q_UNUSED(requestIncline)
    // TODO
}

void horizontreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void horizontreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    double heart = 0; // NOTE : Should be initialized with a value to shut clang-analyzer's
                      // UndefinedBinaryOperatorResult
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && newValue.length() == 20 && newValue.at(0) == 0x00 &&
        ((uint8_t)newValue.at(1)) == 0xF4) {

        Inclination = (double)((uint8_t)newValue.at(3)) / 10.0;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        return;
    }

    if (characteristic.uuid() != QBluetoothUuid((quint16)0x2ACD)) {
        return;
    }

    lastPacket = newValue;

    // default flags for this treadmill is 84 04

    union flags {
        struct {

            uint16_t moreData : 1;
            uint16_t avgSpeed : 1;
            uint16_t totalDistance : 1;
            uint16_t inclination : 1;
            uint16_t elevation : 1;
            uint16_t instantPace : 1;
            uint16_t averagePace : 1;
            uint16_t expEnergy : 1;
            uint16_t heartRate : 1;
            uint16_t metabolic : 1;
            uint16_t elapsedTime : 1;
            uint16_t remainingTime : 1;
            uint16_t forceBelt : 1;
            uint16_t spare : 3;
        };

        uint16_t word_flags;
    };

    flags Flags;
    int index = 0;
    Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
    index += 2;

    if (!Flags.moreData) {
        Speed =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
            100.0;
        index += 2;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    }

    if (Flags.avgSpeed) {

        double avgSpeed;
        avgSpeed =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
            100.0;
        index += 2;
        emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
    }

    if (Flags.totalDistance) {

        // ignoring the distance, because it's a total life odometer
        // Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
        // (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
        index += 3;
    }
    // else
    {
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
    }

    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

    if (Flags.inclination) {
        Inclination =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
            10.0;
        index += 4; // the ramo value is useless
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
    }

    if (Flags.elevation) {

        index += 4; // TODO
    }

    if (Flags.instantPace) {

        index += 1; // TODO
    }

    if (Flags.averagePace) {

        index += 1; // TODO
    }

    if (Flags.expEnergy) {

        KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;

        // energy per hour
        index += 2;

        // energy per minute
        index += 1;
    } else {
        KCal += ((((0.048 * ((double)watts(settings.value(QStringLiteral("weight"), 75.0).toFloat())) + 1.19) *
                   settings.value(QStringLiteral("weight"), 75.0).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    }

    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
    if (settings.value("ant_heart", false).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (Flags.heartRate) {
            if (index < newValue.length()) {

                heart = ((double)((newValue.at(index))));
                emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
            } else {
                emit debug(QStringLiteral("Error on parsing heart!"));
            }
            // index += 1; //NOTE: clang-analyzer-deadcode.DeadStores
        }
    }

    if (Flags.metabolic) {

        // todo
    }

    if (Flags.elapsedTime) {

        // todo
    }

    if (Flags.remainingTime) {

        // todo
    }

    if (Flags.forceBelt) {

        // todo
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0 || settings.value(QStringLiteral("heart_ignore_builtin"), false).toBool()) {

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen h;
            long appleWatchHeartRate = h.heartRate();
            h.setKcal(KCal.value());
            h.setDistance(Distance.value());
            Heart = appleWatchHeartRate;
            debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
#endif
#endif
        } else {

            Heart = heart;
        }
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void horizontreadmill::stateChanged2(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::ServiceDiscovered) {
        return;
    }

    QBluetoothUuid _gattNotify2CharacteristicId((quint16)0xFFF4);
    gattNotify2Characteristic = gattCommunication2ChannelService->characteristic(_gattNotify2CharacteristicId);

    if (!gattNotify2Characteristic.isValid()) {
        qDebug() << QStringLiteral("invalid characteristic");

        return;
    }

    qDebug() << state;

    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);

    gattCommunication2ChannelService->writeDescriptor(
        gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

    connect(gattCommunication2ChannelService, &QLowEnergyService::characteristicChanged, this,
            &horizontreadmill::characteristicChanged);
}

void horizontreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::ServiceDiscovered) {
        return;
    }

    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0x2ACD);
    gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);

    if (!gattNotify1Characteristic.isValid()) {
        qDebug() << QStringLiteral("invalid characteristic");

        return;
    }

    qDebug() << state;

    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);

    gattCommunicationChannelService->writeDescriptor(
        gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
            &horizontreadmill::characteristicChanged);

    initRequest = false;
    emit connectedAndDiscovered();

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !virtualTreadmill
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {

        QSettings settings;
        bool virtual_device_enabled = settings.value(QStringLiteral("virtual_device_enabled"), true).toBool();
        if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual treadmill interface..."));

            virtualTreadmill = new virtualtreadmill(this, noHeartService);
            connect(virtualTreadmill, &virtualtreadmill::debug, this, &horizontreadmill::debug);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void horizontreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void horizontreadmill::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void horizontreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void horizontreadmill::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void horizontreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    gattCommunicationChannelService = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &horizontreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();

    gattCommunication2ChannelService = m_control->createServiceObject(QBluetoothUuid((quint16)0xfff0));
    if (gattCommunication2ChannelService) {
        connect(gattCommunication2ChannelService, &QLowEnergyService::stateChanged, this,
                &horizontreadmill::stateChanged2);

        gattCommunication2ChannelService->discoverDetails();
    }
}

void horizontreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("horizontreadmill::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void horizontreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("horizontreadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void horizontreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    // ***************************************************************************************************************
    // horizon treadmill and F80 treadmill, so if we want to add inclination support we have to separate the 2 devices
    // ***************************************************************************************************************
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &horizontreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &horizontreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &horizontreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &horizontreadmill::controllerStateChanged);

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

bool horizontreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *horizontreadmill::VirtualTreadmill() { return virtualTreadmill; }

void *horizontreadmill::VirtualDevice() { return VirtualTreadmill(); }

void horizontreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}
