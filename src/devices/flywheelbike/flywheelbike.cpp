#include "flywheelbike.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

flywheelbike::flywheelbike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &flywheelbike::update);
    refresh->start(200ms);
}

void flywheelbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                       bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
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
}

void flywheelbike::update() {
    if (!m_control) {
        emit disconnected();
        return;
    }

    qDebug() << m_control->state() << bluetoothDevice.isValid() << gattCommunicationChannelService
             << gattWriteCharacteristic.isValid() << gattNotify1Characteristic.isValid() << initDone;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattNotify1Characteristic.isValid() && initDone) {
        update_metrics(true, watts());

        // updating the treadmill console every second
        /*if(sec1Update++ == (500 / refresh->interval()))
        {
            sec1Update = 0;
            //updateDisplay(elapsed);
        }*/

        if (requestResistance != -1) {
            if (requestResistance > 15) {
                requestResistance = 15;
            } else if (requestResistance == 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                // forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void flywheelbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void flywheelbike::flushDataframe(BikeDataframe *dataFrame) {
    memset(dataFrame->buffer, 0, 256);

    dataFrame->crc = 0;
    dataFrame->len = 0;
    dataFrame->message_id = 0;
}

void flywheelbike::decodeReceivedData(QByteArray buffer) {
    for (int i = 0; i < buffer.length(); ++i) {
        int8_t b = buffer[i];

        qDebug() << QStringLiteral("rxState") << rxState << b;
        switch (rxState) {
        case WFSYNC_1:
            if (b == -1) {
                flushDataframe(&bikeData);
                errorState = MSG_NO_ERROR;
                rxState = WFLENGTH;
            } else {
                errorState = MSG_NO_ERROR;
                rxState = WFSYNC_1;
            }
            break;
        case WFLENGTH:
            if (b == 0) {
                errorState = MSG_WFSIZE;
                rxState = WFSYNC_1;
            } else {
                dataPacketIndex = 0;
                dataPacketLength = b;
                bikeData.len = b - 2;
                rxState = WFID;
            }
            break;
        case WFID:
            if (b >= 0) {
                bikeData.message_id = b;
                if (dataPacketLength != 0) {
                    rxState = DATA;
                } else {
                    rxState = CHECKSUM;
                }
            } else {
                errorState = MSG_UNKNOWN_ID;
                rxState = WFSYNC_1;
            }
            break;
        case DATA:
            if (dataPacketLength == 0) {
                rxState = CHECKSUM;
            }

            bikeData.buffer[dataPacketIndex] = b;
            dataPacketIndex++;
            dataPacketLength--;
            break;
        case CHECKSUM:
            /* TODO: Implement checksum */
            rxState = EOF_1;
            break;
        case EOF_1:
            /* End of frame byte is 0x55 */
            if (b == 0x55) {
                errorState = MSG_COMPLETE;
                rxState = WFSYNC_1;
            }
            break;
        }
    }
}

void flywheelbike::updateStats() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    // calculate the accumulator every time on the current data, in order to avoid holes in peloton or strava
    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        update_hr_from_external();
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

    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    emit debug("Current Cadence: " + QString::number(Cadence.value()));
    // debug("Current Distance: " + QString::number(distance));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void flywheelbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    static uint8_t zero_fix_filter = 0;
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    //    QString heartRateBeltName = settings.value(QZSettings::heart_rate_belt_name,
    //    QZSettings::default_heart_rate_belt_name)
    //                                    .toString(); // NOTE: clazy-unused-non-trivial-variable

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    decodeReceivedData(newValue);

    qDebug() << QStringLiteral("errorState") << errorState;

    if (errorState == MSG_COMPLETE) {
        ICGLiveStreamData *parsedData = (ICGLiveStreamData *)bikeData.buffer;

        if (bikeData.message_id == SEND_ICG_LIVE_STREAM_DATA) {
            // double distance = GetDistanceFromPacket(newValue); //Note: clang-analyzer-deadcode.DeadStores

#ifdef Q_OS_ANDROID
            if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
                Heart = (uint8_t)KeepAwakeHelper::heart();
#endif

            uint16_t power = ((parsedData->power >> 8) & 0xFF);
            power += ((parsedData->power & 0xFF) << 8);
            uint16_t speed = ((parsedData->speed >> 8) & 0xFF);
            speed += ((parsedData->speed & 0xFF) << 8);

            if (zero_fix_filter <
                    settings.value(QZSettings::flywheel_filter, QZSettings::default_flywheel_filter).toUInt() &&
                (parsedData->cadence == 0 || speed == 0 || power == 0)) {
                qDebug() << QStringLiteral("filtering crappy values");
                zero_fix_filter++;
            } else {
                zero_fix_filter = 0;

                Resistance = parsedData->brake_level;
                emit resistanceRead(Resistance.value());
                if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                        .toString()
                        .startsWith(QStringLiteral("Disabled"))) {
                    Cadence = parsedData->cadence;
                }
                m_watts = power;
                if (settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                    Speed = metric::calculateSpeedFromPower(
                        watts(), Inclination.value(), Speed.value(),
                        fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
                } else if (settings.value(QZSettings::speed_sensor_name, QZSettings::default_speed_sensor_name)
                               .toString()
                               .startsWith(QStringLiteral("Disabled"))) {
                    Speed = ((double)speed) / 10.0;
                }

                // https://www.facebook.com/groups/149984563348738/permalink/174268944253633/?comment_id=174366620910532&reply_comment_id=174666314213896
                m_pelotonResistance = (Resistance.value() * 0.8173) + 9.2712;
            }
            updateStats();
            return;
        }

        if (bikeData.message_id == BRAKE_CALIBRATION_RESET) {
            //[_delegate didUpdateStatus:BIKE_NEEDS_CALIBRATION];
            return;
        }

        if (bikeData.message_id == SEND_ICG_AGGREGATED_STREAM_DATA) {
        }

        if (bikeData.message_id == REQUEST_DISCONNECT) {
            //[self disconnectBike];
            return;
        }

        //[_delegate didUpdateStatus:BIKE_MESSAGE_UNKNOWN];
    }
    updateStats();
}

double flywheelbike::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(2) << 8) | packet.at(3);
    double data = ((double)convertedData) / 100.0f;
    return data;
}

void flywheelbike::btinit() {
    uint8_t initData1[] = {0xf5, 0x20, 0x20, 0x40, 0xf6};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    initDone = true;
}

void flywheelbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattWriteCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &flywheelbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &flywheelbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &flywheelbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &flywheelbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
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
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                // connect(virtualBike,&virtualbike::debug ,this,&flywheelbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &flywheelbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void flywheelbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void flywheelbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void flywheelbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &flywheelbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void flywheelbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("flywheelbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void flywheelbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("flywheelbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void flywheelbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("Flywheel")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &flywheelbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &flywheelbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &flywheelbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &flywheelbike::controllerStateChanged);

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

bool flywheelbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t flywheelbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watts;
}

void flywheelbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
