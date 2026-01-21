#include "trxappgateusbrower.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "qzsettings.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

trxappgateusbrower::trxappgateusbrower(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                                 double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    mearch_novarow_r50 = settings.value(QZSettings::merach_novarow_r50, QZSettings::default_merach_novarow_r50).toBool();
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &trxappgateusbrower::update);
    refresh->start(200ms);
}

void trxappgateusbrower::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void trxappgateusbrower::forceResistance(resistance_t requestResistance) {
    uint8_t noOpData1[] = {0xf0, 0xa6, 0x35, 0x01, 0x02, 0xce};
    noOpData1[4] = requestResistance + 1;
    noOpData1[5] = noOpData1[4] + 0xcc;
    writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("writingResistance"));
}

void trxappgateusbrower::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        QSettings settings;
        update_metrics(true, watts());

        {
            if (requestResistance != -1) {
                if (requestResistance < 1)
                    requestResistance = 1;
                if (requestResistance != currentResistance().value() && requestResistance >= 1 &&
                    requestResistance <= 15) {
                    emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                    forceResistance(requestResistance);
                }
                requestResistance = -1;
            } else {
                // Send polling command based on device type
                if (mearch_novarow_r50) {
                    // Mearch NovaRow R50: request rowing data every 2 seconds (500ms * 4 = 2000ms)
                    uint8_t pollData[] = {0xf0, 0xa2, 0x01, 0xe8, 0x7b};
                    writeCharacteristic(pollData, sizeof(pollData), QStringLiteral("mearch_poll"));
                } else {
                    // Original I-CONSOLE+ polling
                    uint8_t noOpData1[] = {0xf0, 0xa2, 0x01, 0xe7, 0x7a};
                    writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                }
            }
        }

               // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }
        /*
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

             // btinit();

              requestStart = -1;
              emit tapeStarted();
          }*/
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void trxappgateusbrower::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void trxappgateusbrower::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double trxappgateusbrower::GetSpeedFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(7) - 1) + ((packet.at(6) - 1) * 100);
    double data = (double)(convertedData) / 10.0f;
    return data;
}

double trxappgateusbrower::GetCadenceFromPacket(const QByteArray &packet) {

    uint16_t convertedData = ((uint16_t)packet.at(20)) - 1;
    return convertedData;
}

double trxappgateusbrower::GetWattFromPacket(const QByteArray &packet) {

    uint16_t convertedData = ((packet.at(17) - 1) * 100) + (packet.at(18) - 1);
    double data = ((double)(convertedData)) / 10.0f;
    return data;
}

void trxappgateusbrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    double cadence_gain = settings.value(QZSettings::cadence_gain, QZSettings::default_cadence_gain).toDouble();
    double cadence_offset = settings.value(QZSettings::cadence_offset, QZSettings::default_cadence_offset).toDouble();
    bool heart_ignore_builtin = settings.value(QZSettings::heart_ignore_builtin,
                                                QZSettings::default_heart_ignore_builtin).toBool();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if(newValue.length() != 23) {
        return;
    }

    // Parse data based on device type
    uint8_t deviceHeartRate = 0;  // Used for Mearch NovaRow R50

    if (mearch_novarow_r50) {
        // Mearch NovaRow R50 data format parsing
        // elapsedSecond = (values[4] - 1) * 60 + values[5] - 1
        int elapsedSeconds = (newValue.at(4) - 1) * 60 + (newValue.at(5) - 1);

        // strokeCount = (values[6] - 1) * 99 + (values[7] - 1)
        int strokeCount = (newValue.at(6) - 1) * 99 + (newValue.at(7) - 1);

        // strokePerMinute = (values[8] - 1) * 99 + (values[9] - 1)
        Cadence = ((newValue.at(8) - 1) * 99 + (newValue.at(9) - 1)) * cadence_gain + cadence_offset;

        // distance = (values[10] - 1) * 99 + (values[11] - 1)
        Distance = ((newValue.at(10) - 1) * 99 + (newValue.at(11) - 1)) / 1000.0; // convert to km

        // calories = (values[12] - 1) * 99 + (values[13] - 1)
        KCal = (newValue.at(12) - 1) * 99 + (newValue.at(13) - 1);

        // heartbeat = (values[14] - 1) * 99 + (values[15] - 1)
        deviceHeartRate = (newValue.at(14) - 1) * 99 + (newValue.at(15) - 1);

        // Use device heart rate only if it's valid (> 0) and not ignoring builtin
        if (!heart_ignore_builtin && deviceHeartRate > 0) {
            Heart = deviceHeartRate;
        }
        // Otherwise, external heart rate will be handled below

        // power = (values[16] - 1) * 99 + (values[17] - 1)
        m_watt = (newValue.at(16) - 1) * 99 + (newValue.at(17) - 1);

        // gear = values[20] - 1
        Resistance = newValue.at(20) - 1;

        // Speed calculation based on distance/time (approximate)
        if (elapsedSeconds > 0) {
            Speed = (Distance.value() / elapsedSeconds) * 3.6; // convert m/s to km/h
        }
    } else {
        // Original I-CONSOLE+ data format
        Resistance = newValue.at(18) - 1;
        Speed = GetSpeedFromPacket(newValue);
        Cadence = (GetCadenceFromPacket(newValue) * cadence_gain) + cadence_offset;
        m_watt = GetWattFromPacket(newValue);
        if (watts())
            KCal += ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                     (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                    QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                      // kg * 3.5) / 200 ) / 60
        // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            // Update from external sources only if device HR is 0 or ignoring builtin
            if (deviceHeartRate == 0 || heart_ignore_builtin) {
                update_hr_from_external();
            }
            // If deviceHeartRate > 0 and not ignoring builtin, Heart was already set above in parsing section
        }
    }

    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    // debug("Current Distance: " + QString::number(distance));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void trxappgateusbrower::btinit() {

    // Check if Mearch NovaRow R50 setting is enabled
    if (mearch_novarow_r50) {
        // Mearch NovaRow R50 initialization sequence to switch to Bluetooth mode
        uint8_t initData1[] = {0xf0, 0xa5, 0x44, 0x01, 0x04, 0xde};
        uint8_t initData2[] = {0xf0, 0xa0, 0x44, 0x01, 0xd5};
        uint8_t initData3[] = {0xf0, 0xa0, 0x01, 0xe8, 0x79};
        uint8_t initData4[] = {0xf0, 0xa5, 0x01, 0xe8, 0x02, 0x80};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("mearch_init1"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("mearch_init2"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("mearch_init3"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("mearch_init4"), false, true);
    } else {
        // Original I-CONSOLE+ initialization
        uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x00, 0x91};
        uint8_t initData2[] = {0xf0, 0xa0, 0x01, 0xe7, 0x78};
        uint8_t initData3[] = {0xf0, 0xa1, 0x01, 0xe7, 0x79};
        uint8_t initData4[] = {0xf0, 0xa3, 0x01, 0xe7, 0x01, 0x7c};
        uint8_t initData5[] = {0xf0, 0xa4, 0x01, 0xe7, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x83};
        uint8_t initData6[] = {0xf0, 0xa5, 0x01, 0xe7, 0x02, 0x7f};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
    }

    initDone = true;
}

void trxappgateusbrower::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        QString uuidWrite = QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb");
        QString uuidNotify1 = QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb");
        QString uuidNotify2 = QStringLiteral("49535343-4c8a-39b3-2f49-511cff073b7e");


        QBluetoothUuid _gattWriteCharacteristicId(uuidWrite);
        QBluetoothUuid _gattNotify1CharacteristicId(uuidNotify1);
        QBluetoothUuid _gattNotify2CharacteristicId(uuidNotify2);

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);

        if (!gattWriteCharacteristic.isValid()) {
            uuidWrite = QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3");
            uuidNotify1 = QStringLiteral("49535343-1E4D-4BD9-BA61-23C647249616");
            uuidNotify2 = QStringLiteral("49535343-4c8a-39b3-2f49-511cff073b7e");

            gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
            gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        }


               // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &trxappgateusbrower::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &trxappgateusbrower::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &trxappgateusbrower::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &trxappgateusbrower::descriptorWritten);

               // ******************************************* virtual rower init *************************************
        QSettings settings;
        if (!firstStateChanged && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_rower =
                settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
            bool virtual_device_force_treadmill =
                settings.value(QZSettings::virtual_device_force_treadmill, QZSettings::default_virtual_device_force_treadmill).toBool();
            if (virtual_device_enabled) {
                if (virtual_device_force_treadmill) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &trxappgateusbrower::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &trxappgateusbrower::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else if (!virtual_device_rower) {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual rower interface...");
                    auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                    this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
                }
                firstStateChanged = 1;
            }
        }
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void trxappgateusbrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void trxappgateusbrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void trxappgateusbrower::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QString uuid = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");
    QString uuid2 = QStringLiteral("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
    QString uuid3 = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");

    QBluetoothUuid _gattCommunicationChannelServiceId(uuid);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << QStringLiteral("invalid service") << uuid;
        uuid = uuid2;
        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &trxappgateusbrower::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void trxappgateusbrower::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("trxappgateusbrower::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void trxappgateusbrower::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("trxappgateusbrower::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void trxappgateusbrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');

    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &trxappgateusbrower::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &trxappgateusbrower::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &trxappgateusbrower::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &trxappgateusbrower::controllerStateChanged);

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

bool trxappgateusbrower::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void trxappgateusbrower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

uint16_t trxappgateusbrower::watts() { return m_watt.value(); }


void trxappgateusbrower::searchingStop() { searchStopped = true; }

