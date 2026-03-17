#include "trxappgateusbelliptical.h"
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
#include <QThread>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

trxappgateusbelliptical::trxappgateusbelliptical(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                                 double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &trxappgateusbelliptical::update);
    refresh->start(200ms);
}

void trxappgateusbelliptical::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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

void trxappgateusbelliptical::forceResistance(resistance_t requestResistance) {
    if (elliptical_type == TYPE::DCT2000I || elliptical_type == TYPE::JTX_FITNESS || elliptical_type == TYPE::TAURUS_FX99) {
        uint8_t noOpData1[] = {0xf0, 0xa6, 0x01, 0x01, 0x03, 0x9b};
        noOpData1[4] = requestResistance + 1;
        noOpData1[5] = noOpData1[4] + 0x98;
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("writingResistance"));
    } else {
        uint8_t noOpData1[] = {0xf0, 0xa6, 0x35, 0x01, 0x02, 0xce};
        noOpData1[4] = requestResistance + 1;
        noOpData1[5] = noOpData1[4] + 0xcc;
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("writingResistance"));
    }
}

void trxappgateusbelliptical::update() {
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

        // Restore resistance after reconnection and init
        if (needsResistanceRestore && lastResistanceBeforeDisconnection > 0) {
            qDebug() << QStringLiteral("Restoring resistance after reconnection:") << lastResistanceBeforeDisconnection;
            forceResistance(lastResistanceBeforeDisconnection);
            needsResistanceRestore = false;
            lastResistanceBeforeDisconnection = -1;
        }

        // Calculate time since last valid packet
        qint64 msSinceLastValidPacket = lastValidPacketTime.msecsTo(QDateTime::currentDateTime());

        // If we haven't received a valid packet for more than 5 seconds, reinitialize
        if (msSinceLastValidPacket > 5000) {
            qDebug() << QStringLiteral("NO VALID PACKETS for") << (msSinceLastValidPacket / 1000.0)
                     << QStringLiteral("seconds. Reinitializing connection...");

            // Reset timer
            lastValidPacketTime = QDateTime::currentDateTime();

            m_control->disconnectFromDevice();
        }


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
                if (elliptical_type == TYPE::DCT2000I || elliptical_type == TYPE::JTX_FITNESS || elliptical_type == TYPE::TAURUS_FX99) {
                    uint8_t noOpData1[] = {0xf0, 0xa2, 0x01, 0x01, 0x94};
                    writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                } else {
                    uint8_t noOpData1[] = {0xf0, 0xa2, 0x35, 0x01, 0xc8};
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

void trxappgateusbelliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void trxappgateusbelliptical::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double trxappgateusbelliptical::GetSpeedFromPacket(const QByteArray &packet) {

    if (elliptical_type == TYPE::JTX_FITNESS) {
        // JTX Fitness doesn't send speed via bluetooth, calculate from cadence using settings ratio
        QSettings settings;
        double cadence_speed_ratio = settings.value(QZSettings::cadence_sensor_speed_ratio, QZSettings::default_cadence_sensor_speed_ratio).toDouble();
        double cadence = GetCadenceFromPacket(packet);
        return cadence * cadence_speed_ratio;
    } else {
        uint16_t convertedData = (packet.at(7) - 1) + ((packet.at(6) - 1) * 100);
        double data = (double)(convertedData) / 10.0f;
        return data;
    }
}

double trxappgateusbelliptical::GetCadenceFromPacket(const QByteArray &packet) {

    uint16_t convertedData;
    if (elliptical_type == TYPE::JTX_FITNESS) {
        // JTX Fitness uses only byte 5 for cadence
        convertedData = packet.at(5);
    } else {
        convertedData = ((uint16_t)packet.at(9)) + ((uint16_t)packet.at(8) * 100);
    }
    return convertedData;
}

double trxappgateusbelliptical::GetWattFromPacket(const QByteArray &packet) {

    if (elliptical_type == TYPE::JTX_FITNESS) {
        // JTX Fitness doesn't send watts via bluetooth, use classic elliptical calculation
        return 0; // Will be calculated in characteristicChanged using wattsFromResistance
    } else {
        uint16_t convertedData = ((packet.at(16) - 1) * 100) + (packet.at(17) - 1);
        double data = ((double)(convertedData)) / 10.0f;
        return data;
    }
}

void trxappgateusbelliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    double cadence_gain = settings.value(QZSettings::cadence_gain, QZSettings::default_cadence_gain).toDouble();
    double cadence_offset = settings.value(QZSettings::cadence_offset, QZSettings::default_cadence_offset).toDouble();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    lastValidPacketTime = QDateTime::currentDateTime();
  
    // Check for invalid packet length first
    bool isValidPacket = (newValue.length() == 21);

    if (!isValidPacket) {
        // Invalid packet length - log and return
        qDebug() << QStringLiteral("Invalid packet length:") << newValue.length();
        return;
    }

    // Log controller errors but don't block processing of valid packets
    bool hasError = (m_control->error() != QLowEnergyController::NoError);
    if (hasError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
        // Continue processing - the packet is still valid
    }

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

    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    // debug("Current Distance: " + QString::number(distance));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
}

void trxappgateusbelliptical::btinit() {

    if (elliptical_type == TYPE::DCT2000I || elliptical_type == TYPE::JTX_FITNESS || elliptical_type == TYPE::TAURUS_FX99) {
        uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x00, 0x91};
        uint8_t initData2[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        uint8_t initData3[] = {0xf0, 0xa1, 0x01, 0x01, 0x93};
        uint8_t initData4[] = {0xf0, 0xa3, 0x01, 0x01, 0x01, 0x96};
        uint8_t initData5[] = {0xf0, 0xa4, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xa0};
        uint8_t initData6[] = {0xf0, 0xa5, 0x01, 0x01, 0x02, 0x99};

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
    } else {
        uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x00, 0x91};
        uint8_t initData2[] = {0xf0, 0xa0, 0x35, 0x01, 0xc6};
        uint8_t initData3[] = {0xf0, 0xa1, 0x35, 0x01, 0xc7};
        uint8_t initData4[] = {0xf0, 0xa3, 0x35, 0x01, 0x01, 0xca};
        uint8_t initData5[] = {0xf0, 0xa4, 0x35, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xd4};
        uint8_t initData6[] = {0xf0, 0xa5, 0x35, 0x01, 0x02, 0xcd};

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

void trxappgateusbelliptical::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        QString uuidWrite = QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb");
        QString uuidNotify1 = QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb");
        QString uuidNotify2 = QStringLiteral("49535343-4c8a-39b3-2f49-511cff073b7e");

        if (elliptical_type == TYPE::DCT2000I || elliptical_type == TYPE::JTX_FITNESS) {
            uuidWrite = QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3");
            uuidNotify1 = QStringLiteral("49535343-1E4D-4BD9-BA61-23C647249616");
            uuidNotify2 = QStringLiteral("49535343-4c8a-39b3-2f49-511cff073b7e");
        }
        // TAURUS_FX99 uses standard 0000fff0 characteristics

        QBluetoothUuid _gattWriteCharacteristicId(uuidWrite);
        QBluetoothUuid _gattNotify1CharacteristicId(uuidNotify1);
        QBluetoothUuid _gattNotify2CharacteristicId(uuidNotify2);

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);

        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

               // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &trxappgateusbelliptical::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &trxappgateusbelliptical::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &trxappgateusbelliptical::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &trxappgateusbelliptical::descriptorWritten);

               // ******************************************* virtual treadmill init *************************************
        QSettings settings;
        if (!firstStateChanged && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &trxappgateusbelliptical::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &trxappgateusbelliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &trxappgateusbelliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
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

void trxappgateusbelliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void trxappgateusbelliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void trxappgateusbelliptical::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QString uuid = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");
    QString uuid2 = QStringLiteral("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
    QString uuid3 = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");

    if (elliptical_type == TYPE::DCT2000I) {
        uuid = uuid2;
    }

    // Fallback logic: try to find the service in discovered services
    bool found = false;
    foreach (QBluetoothUuid s, m_control->services()) {
        if (s == QBluetoothUuid::fromString(uuid)) {
            found = true;
            break;
        }
    }
    
    // If primary service not found, try fallback service
    if (!found) {
        if (elliptical_type == TYPE::DCT2000I) {
            // I-CONSOLE+ device but DCT2000I service not found, try 0000fff0 service (Taurus FX9.9)
            bool found_fff0 = false;
            foreach (QBluetoothUuid s, m_control->services()) {
                if (s == QBluetoothUuid::fromString(uuid3)) {
                    found_fff0 = true;
                    break;
                }
            }
            if (found_fff0) {
                uuid = uuid3;
                elliptical_type = TYPE::TAURUS_FX99;
                qDebug() << QStringLiteral("I-CONSOLE+ device detected as Taurus FX9.9 with 0000fff0 service");
            } else {
                qDebug() << QStringLiteral("DCT2000I service not found");
            }
        } else {
            // Try DCT2000I/JTX Fitness service as fallback
            uuid = uuid2;
            elliptical_type = TYPE::JTX_FITNESS;
            qDebug() << QStringLiteral("Standard service not found, trying JTX Fitness service as fallback");
        }
    }

    QBluetoothUuid _gattCommunicationChannelServiceId(uuid);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << QStringLiteral("invalid service") << uuid;

        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &trxappgateusbelliptical::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void trxappgateusbelliptical::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("trxappgateusbelliptical::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void trxappgateusbelliptical::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("trxappgateusbelliptical::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void trxappgateusbelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    if (device.name().toUpper().startsWith(QStringLiteral("I-CONSOLE+"))) {
        elliptical_type = DCT2000I;
        qDebug() << "DCT2000I workaround activacted!";
    }

    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &trxappgateusbelliptical::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &trxappgateusbelliptical::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &trxappgateusbelliptical::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &trxappgateusbelliptical::controllerStateChanged);

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

bool trxappgateusbelliptical::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void trxappgateusbelliptical::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again in 3 seconds...");

        // Save current resistance before disconnection
        if (Resistance.value() > 0) {
            lastResistanceBeforeDisconnection = Resistance.value();
            needsResistanceRestore = true;
            qDebug() << QStringLiteral("Saved resistance before disconnection:") << lastResistanceBeforeDisconnection;
        }

        initDone = false;

        // Schedule reconnection after 3 seconds
        QTimer::singleShot(3000, this, [this]() {
            if (m_control && m_control->state() == QLowEnergyController::UnconnectedState) {
                qDebug() << QStringLiteral("Reconnection timer fired, attempting to reconnect...");
                // Reset the last valid packet timer
                lastValidPacketTime = QDateTime::currentDateTime();
                m_control->connectToDevice();
            }
        });
    }
}

uint16_t trxappgateusbelliptical::watts() { 
    if (elliptical_type == TYPE::JTX_FITNESS) {
        // For JTX Fitness, always use the elliptical class generic calculation
        return elliptical::watts();
    }
    return m_watt.value(); 
}


void trxappgateusbelliptical::searchingStop() { searchStopped = true; }

bool trxappgateusbelliptical::inclinationAvailableByHardware() {
    // actually it has but i don't have the bluetooth code to change inclination
    return false;
}
