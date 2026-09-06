#include "smartrowrower.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
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

smartrowrower::smartrowrower(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                             double bikeResistanceGain) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &smartrowrower::update);
    refresh->start(200ms);
}

void smartrowrower::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                        bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
    // one for the resistance changed event (spontaneous), and one for the other ones.
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info;
    }

    loop.exec();
}

void smartrowrower::forceResistance(resistance_t requestResistance) {
    Q_UNUSED(requestResistance)
    /*
                                   uint8_t noOpData[] = { 0xf0, 0xb1, 0x01, 0x00, 0x00 };

                                   noOpData[3] = requestResistance;

                                   for(uint8_t i=0; i<sizeof(noOpData)-1; i++)
                                   {
                                      noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
                                   }

                                   writeCharacteristic(noOpData, sizeof(noOpData), "force resistance", false, true);*/
}

void smartrowrower::sendPoll() {
    uint8_t noOpData[] = {0x24};
    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
}

void smartrowrower::queueSmartRowWrite(const QByteArray &data) { pendingSmartRowWrite.append(data); }

void smartrowrower::sendNextSmartRowWrite() {
    if (pendingSmartRowWrite.isEmpty()) {
        return;
    }

    uint8_t data = (uint8_t)pendingSmartRowWrite.at(0);
    pendingSmartRowWrite.remove(0, 1);
    writeCharacteristic(&data, sizeof(data), QStringLiteral("smartrow"), false, false);
}

void smartrowrower::update() {
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
        if (!pendingSmartRowWrite.isEmpty()) {
            sendNextSmartRowWrite();
            return;
        }

        update_metrics(false, watts());

        // sending poll every 2 seconds
        if (sec1Update++ >= (2000 / refresh->interval())) {
            sec1Update = 0;
            sendPoll();
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance)
                requestResistance = max_resistance;
            else if (requestResistance <= 0)
                requestResistance = 1;

            if (requestResistance != currentResistance().value()) {
                qDebug() << QStringLiteral("writing resistance ") + QString::number(requestResistance);
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestStart != -1) {
            qDebug() << QStringLiteral("starting...");

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            qDebug() << QStringLiteral("stopping...");
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void smartrowrower::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

resistance_t smartrowrower::pelotonToBikeResistance(int pelotonResistance) {
    for (int i = 1; i < max_resistance - 1; i++) {
        if (bikeResistanceToPeloton(i) <= pelotonResistance && bikeResistanceToPeloton(i + 1) >= pelotonResistance) {
            return i;
        }
    }
    return Resistance.value();
}

resistance_t smartrowrower::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    for (int i = 1; i < max_resistance - 1; i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i)
                     << wattsFromResistance(i + 1) << power;
            return i;
        }
    }
    return Resistance.value();
}

double smartrowrower::bikeResistanceToPeloton(double resistance) {
    // 0,0097x3 - 0,4972x2 + 10,126x - 37,08
    double p = ((pow(resistance, 3) * 0.0097) - (0.4972 * pow(resistance, 2)) + (10.126 * resistance) - 37.08);
    if (p < 0)
        p = 0;
    return p;
}

void smartrowrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << " << " + newValue.toHex(' ');

    if (newValue.contains("V3.")) {
        smartRowV3 = true;
        if (!smartRowV3ChallengeRequested) {
            smartRowV3ChallengeRequested = true;
            queueSmartRowWrite(QByteArray(1, (char)0x23));
        }
    } else if (newValue.contains("KEYLOCK")) {
        smartRowV3 = true;
        queueSmartRowWrite(calculateSmartRowV3ChallengeResponse(newValue));
    }

    if (newValue.length() != 17)
        return;

    QByteArray packet = decodeSmartRowPacket(newValue);
    if (packet != newValue) {
        qDebug() << " << decoded " + packet.toHex(' ');
    }

    lastPacket = packet;

    double distance = GetDistanceFromPacket(packet);
    QTime localTime;
    int pace_inst;

    // https://github.com/inonoob/pirowflo/blob/6ea5f3a9d224ed594b23c25c186737bc0cae7ac3/src/adapters/smartrow/smartrowtobleant.py
    switch (packet.at(0)) {
    case 'a':
        // elapsed time
        localTime = QTime(atoi(packet.mid(6, 2)), atoi(packet.mid(8, 2)), atoi(packet.mid(10, 2)));
        break;
    case 'b':
        // work per stroke[6:11] / 10, stroke length [11:13]
        StrokesLength = atoi(packet.mid(11, 3));
        break;
    case 'c':
        // actual power
        m_watt = atoi(packet.mid(6, 3));
        // average power / 10
        // ignore it
        break;
    case 'd':
        // strokes per minute
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            Cadence = atoi(packet.mid(6, 3)) / 10.0;
        // strokes count [9:11]
        StrokesCount = atoi(packet.mid(9, 4));
        break;
    case 'e':
        // actual split time
        // pace_inst = int(event[6])*60 + int(event[7:9])
        // 3243 = 180 + 243 = 713
        // speed = int(500 * 100 / pace_inst) # speed in cm/s
        pace_inst = (atoi(packet.mid(6, 1)) * 60) + atoi(packet.mid(7, 2));
        qDebug() << QStringLiteral("pace_inst") << pace_inst;
        if (pace_inst > 0) {
            Speed = (500.0 * 100.0 / pace_inst) * 0.036;
        } else {
            Speed = 0;
        }

        // average split time
        break;
    case 'f':
        // no row packet.at(5) == '!'
        break;
    case 'x':
        // curve points
        break;
    case 'y':
        // curve points
        break;
    case 'z':
        // curve points
        break;
    }

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            now)))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    // Distance += ((Speed.value() / 3600000.0) *
    // ((double)lastRefreshCharacteristicChanged.msecsTo(now)) );
    Distance = distance;

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = now;

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
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    qDebug() << QStringLiteral("Current Local elapsed: ") + localTime.toString();
    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current Distance: ") + QString::number(distance);
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError)
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
}

QByteArray smartrowrower::calculateSmartRowV3ChallengeResponse(const QByteArray &keylock) const {
    QByteArray challenge = keylock.trimmed();
    const int keylockIndex = challenge.indexOf("KEYLOCK=");
    if (keylockIndex >= 0) {
        challenge = challenge.mid(keylockIndex);
    }

    if (challenge.length() < 16) {
        return QByteArray(1, (char)0x23);
    }

    const QByteArray key = challenge.left(14);
    const QByteArray checksum = challenge.mid(14, 2).toUpper();
    int checksumValue = 0;
    for (char c : key) {
        checksumValue += (uint8_t)c;
    }

    const QByteArray calculatedChecksum =
        QByteArray::number(checksumValue, 16).rightJustified(4, '0').right(2).toUpper();
    if (calculatedChecksum != checksum) {
        qDebug() << QStringLiteral("SmartRow V3 challenge checksum error") << challenge << calculatedChecksum
                 << checksum;
        return QByteArray(1, (char)0x23);
    }

    bool ok = false;
    const uint16_t seed = challenge.mid(10, 4).toUShort(&ok, 16);
    if (!ok) {
        return QByteArray(1, (char)0x23);
    }

    const uint32_t responseValue = ((uint32_t)seed * 17923U) / 256U;
    const QByteArray response =
        QByteArray::number(responseValue, 16).rightJustified(6, '0').right(4).toLower();

    QByteArray result;
    result.append((char)0x0d);
    result.append(response);
    result.append((char)0x0d);
    return result;
}

QByteArray smartrowrower::decodeSmartRowPacket(const QByteArray &packet) const {
    if (!smartRowV3 || packet.length() != 17) {
        return packet;
    }

    switch (packet.at(0)) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'x':
    case 'y':
    case 'z':
        break;
    default:
        return packet;
    }

    QByteArray decoded = packet;
    for (int i = 1; i <= 5; i++) {
        decoded[i] = (char)(((uint8_t)packet.at(i) & 0x0f) | 0x30);
    }
    return decoded;
}

double smartrowrower::GetDistanceFromPacket(const QByteArray &packet) {
    uint32_t convertedData = atoi(packet.mid(1, 5));
    double data = ((double)convertedData) / 1000.0;
    return data;
}

void smartrowrower::btinit() {
    QByteArray initData;
    initData.append((char)0x24);
    initData.append((char)0x0d);
    initData.append((char)0x56);
    initData.append((char)0x40);
    initData.append((char)0x0d);
    queueSmartRowWrite(initData);

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void smartrowrower::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0x1235);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0x1236);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &smartrowrower::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &smartrowrower::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &smartrowrower::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &smartrowrower::descriptorWritten);

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
            bool virtual_device_rower =
                settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
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
                if (!virtual_device_rower) {
                    qDebug() << QStringLiteral("creating virtual bike interface...");
                    auto virtualBike =
                        new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                    // connect(virtualBike,&virtualbike::debug ,this,&smartrowrower::debug);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    qDebug() << QStringLiteral("creating virtual rower interface...");
                    auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                    // connect(virtualRower,&virtualrower::debug ,this,&smartrowrower::debug);
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
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void smartrowrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void smartrowrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void smartrowrower::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0x1234);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &smartrowrower::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void smartrowrower::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("smartrowrower::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void smartrowrower::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("smartrowrower::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void smartrowrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
                    device.address().toString() + ')';
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &smartrowrower::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &smartrowrower::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &smartrowrower::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &smartrowrower::controllerStateChanged);

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
}

bool smartrowrower::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t smartrowrower::watts() {
    if (currentCadence().value() == 0)
        return 0;
    return wattsFromResistance(Resistance.value());
}

uint16_t smartrowrower::wattsFromResistance(double resistance) {
    Q_UNUSED(resistance)
    // TODO: add lookup table in case we need to add the compatibility with peloton
    return m_watt.value();
}

void smartrowrower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        smartRowV3 = false;
        smartRowV3ChallengeRequested = false;
        pendingSmartRowWrite.clear();
        m_control->connectToDevice();
    }
}
