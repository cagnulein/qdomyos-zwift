#include "echelonrower.h"
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

echelonrower::echelonrower(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                           double bikeResistanceGain) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    speedRaw.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &echelonrower::update);
    refresh->start(200ms);
}

void echelonrower::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                        QStringLiteral(" // ") + info;
    }

    loop.exec();
}

void echelonrower::forceResistance(resistance_t requestResistance) {
    uint8_t noOpData[] = {0xf0, 0xb1, 0x01, 0x00, 0x00};

    noOpData[3] = requestResistance;

    for (uint8_t i = 0; i < sizeof(noOpData) - 1; i++) {
        noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("force resistance"), false, true);
}

void echelonrower::sendPoll() {
    uint8_t noOpData[] = {0xf0, 0xa0, 0x01, 0x00, 0x00};

    noOpData[3] = counterPoll;

    for (uint8_t i = 0; i < sizeof(noOpData) - 1; i++) {
        noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);

    counterPoll++;
    if (!counterPoll) {
        counterPoll = 1;
    }
}

void echelonrower::update() {
    if (m_control == nullptr)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && gattNotify2Characteristic.isValid() && initDone) {
        update_metrics(true, watts());

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

void echelonrower::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

resistance_t echelonrower::pelotonToBikeResistance(int pelotonResistance) {
    for (resistance_t i = 1; i < max_resistance - 1; i++) {
        if (bikeResistanceToPeloton(i) <= pelotonResistance && bikeResistanceToPeloton(i + 1) >= pelotonResistance) {
            return i;
        }
    }
    return Resistance.value();
}

resistance_t echelonrower::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    for (resistance_t i = 1; i < max_resistance - 1; i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i)
                     << wattsFromResistance(i + 1) << power;
            return i;
        }
    }
    return Resistance.value();
}

double echelonrower::bikeResistanceToPeloton(double resistance) {
    // 0,0097x3 - 0,4972x2 + 10,126x - 37,08
    double p = ((pow(resistance, 3) * 0.0097) - (0.4972 * pow(resistance, 2)) + (10.126 * resistance) - 37.08);
    if (p < 0) {
        p = 0;
    }
    return p;
}

void echelonrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newvalue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << QStringLiteral(" << ") + newvalue.toHex(' ');

    if (lastPacket.count() > 0 && lastPacket.count() + newvalue.count() == 21 && ((unsigned char)lastPacket.at(0)) == 0xf0) {
        lastPacket = lastPacket.append(newvalue);
        qDebug() << QStringLiteral(" << concatenated ") + lastPacket.toHex(' ');
    } else {
        lastPacket = newvalue;
    }

    // resistance value is in another frame
    if (lastPacket.length() == 5 && ((unsigned char)lastPacket.at(0)) == 0xf0 &&
        ((unsigned char)lastPacket.at(1)) == 0xd2) {
        Resistance = lastPacket.at(3);
        emit resistanceRead(Resistance.value());
        m_pelotonResistance = bikeResistanceToPeloton(Resistance.value());

        qDebug() << QStringLiteral("Current resistance: ") + QString::number(Resistance.value());
        return;
    }

    if (lastPacket.length() != 21) {
        return;
    }

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

    // double distance = GetDistanceFromPacket(newValue);

    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = ((uint8_t)lastPacket.at(11));
        StrokesCount += (Cadence.value()) *
                        ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())) / 60000;
    }
    // instant pace to km/h
    if ((((uint8_t)lastPacket.at(14)) > 0 || ((uint8_t)lastPacket.at(13)) > 0) && Cadence.value() > 0) {
        speedRaw = (60.0 / (double)((((uint16_t)lastPacket.at(13) << 8) & 0xFF00) | (((uint16_t)lastPacket.at(14)) & 0x00FF))) * 30.0;
        Speed = speedRaw.average5s();
    } else {
        Speed = 0;
        speedRaw = 0;
    }

    StrokesLength =
        ((Speed.value() / 60.0) * 1000.0) /
        Cadence.value(); // this is just to fill the tile, but it's quite useless since the machinery doesn't report it
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

    qDebug() << QStringLiteral("Current Local elapsed: ") + GetElapsedFromPacket(lastPacket).toString();
    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Pace: ") + currentPace().toString();
    qDebug() << QStringLiteral("Current Speed Raw: ") + QString::number(speedRaw.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    // qDebug() << QStringLiteral("Current Distance: ") + QString::number(distance);
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

QTime echelonrower::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    QTime t(0, convertedData / 60, convertedData % 60);
    return t;
}

double echelonrower::GetDistanceFromPacket(const QByteArray &packet) {
    uint32_t convertedData = (packet.at(15) << 16) | (packet.at(16) << 8) | packet.at(17);
    double data = ((double)convertedData) / 1000.0f;
    return data;
}

void echelonrower::btinit() {
    uint8_t initData1[] = {0xf0, 0xa1, 0x00, 0x91};
    uint8_t initData2[] = {0xf0, 0xa3, 0x00, 0x93};
    uint8_t initData3[] = {0xf0, 0xb0, 0x01, 0x01, 0xa2};
    // uint8_t initData4[] = { 0xf0, 0x60, 0x00, 0x50 }; // get sleep command

    // useless i guess
    // writeCharacteristic(initData4, sizeof(initData4), "get sleep", false, true);

    // in the snoof log it repeats this frame 4 times, i will have to analyze the response to understand if 4 times are
    // enough
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void echelonrower::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("0bf669f2-45f2-11e7-9598-0800200c9a66"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("0bf669f3-45f2-11e7-9598-0800200c9a66"));
    QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &echelonrower::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &echelonrower::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &echelonrower::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &echelonrower::descriptorWritten);

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
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset,
                                                       bikeResistanceGain);
                    // connect(virtualBike,&virtualbike::debug ,this,&echelonrower::debug);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    qDebug() << QStringLiteral("creating virtual rower interface...");
                    auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                    // connect(virtualRower,&virtualrower::debug ,this,&echelonrower::debug);
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
        gattCommunicationChannelService->writeDescriptor(
            gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void echelonrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void echelonrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void echelonrower::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("0bf669f1-45f2-11e7-9598-0800200c9a66"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &echelonrower::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void echelonrower::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("echelonrower::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void echelonrower::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << "echelonrower::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString();
}

void echelonrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << "Found new device: " + device.name() + " (" + device.address().toString() + ')';
    bluetoothDevice = device;

    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &echelonrower::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &echelonrower::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &echelonrower::error);
    connect(m_control, &QLowEnergyController::stateChanged, this, &echelonrower::controllerStateChanged);

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

bool echelonrower::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t echelonrower::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return wattsFromResistance(Resistance.value());
}

uint16_t echelonrower::wattsFromResistance(double resistance) {
    // https://github.com/cagnulein/qdomyos-zwift/issues/62#issuecomment-736913564
    /*if(currentCadence().value() < 90)
        return (uint16_t)((3.59 * exp(0.0217 * (double)(currentCadence().value()))) * exp(0.095 *
    (double)(currentResistance().value())) ); else return (uint16_t)((3.59 * exp(0.0217 *
    (double)(currentCadence().value()))) * exp(0.088 * (double)(currentResistance().value())) );*/

    const double Epsilon = 4.94065645841247E-324;
    const int wattTableFirstDimension = 33;
    const int wattTableSecondDimension = 12;

    double wattTable[wattTableFirstDimension][wattTableSecondDimension] = {
        {Epsilon, Epsilon, 30.0, 62.0, 88.0, 108.0, 119.0, 128.0, 141.0, 153.0, 162.0, 174.0},
        {Epsilon, Epsilon, 30.0, 62.0, 88.0, 108.0, 119.0, 128.0, 141.0, 153.0, 162.0, 174.0},
        {Epsilon, Epsilon, 31.0, 64.0, 90.0, 113.0, 123.0, 133.0, 144.0, 155.0, 164.0, 176.0},
        {Epsilon, Epsilon, 31.5, 65.0, 93.0, 117.0, 129.0, 137.0, 148.0, 158.0, 167.0, 180.0},
        {Epsilon, Epsilon, 32.0, 66.0, 96.0, 122.0, 134.0, 143.0, 153.0, 161.0, 169.0, 182.0},
        {Epsilon, Epsilon, 32.5, 67.0, 98.0, 127.0, 138.0, 151.0, 161.0, 168.0, 170.0, 184.0},
        {Epsilon, Epsilon, 33.0, 68.0, 103.0, 133.0, 145.0, 157.0, 167.0, 172.0, 174.0, 188.0},
        {Epsilon, Epsilon, 33.5, 69.0, 105.0, 136.0, 151.0, 160.0, 174.0, 179.0, 180.0, 193.0},
        {Epsilon, Epsilon, 34.0, 70.0, 107.0, 140.0, 159.0, 164.0, 177.0, 184.0, 186.0, 198.0},
        {Epsilon, Epsilon, 34.5, 71.0, 110.0, 144.0, 164.0, 168.0, 182.0, 188.0, 190.0, 205.0},
        {Epsilon, Epsilon, 35.0, 72.0, 113.0, 148.0, 169.0, 178.0, 194.0, 202.0, 209.0, 220.0},
        {Epsilon, Epsilon, 35.5, 73.5, 117.0, 155.0, 174.0, 185.0, 199.0, 209.0, 217.0, 227.0},
        {Epsilon, Epsilon, 36.0, 75.0, 120.0, 158.0, 179.0, 188.0, 205.0, 217.0, 225.0, 234.0},
        {Epsilon, Epsilon, 37.0, 76.5, 122.0, 162.0, 186.0, 202.0, 216.0, 225.0, 231.0, 243.0},
        {Epsilon, Epsilon, 38.0, 77.0, 124.0, 165.0, 189.0, 217.0, 224.0, 232.0, 240.0, 254.0},
        {Epsilon, Epsilon, 38.5, 78.0, 128.0, 168.0, 201.0, 221.0, 231.0, 243.0, 249.0, 262.0},
        {Epsilon, Epsilon, 39.5, 79.0, 132.0, 175.0, 208.0, 228.0, 239.0, 251.0, 261.0, 272.0},
        {Epsilon, Epsilon, 40.5, 80.5, 138.0, 180.0, 219.0, 234.0, 246.0, 259.0, 271.0, 281.0},
        {Epsilon, Epsilon, 41.0, 82.0, 145.0, 187.0, 229.0, 246.0, 252.0, 264.0, 278.0, 291.0},
        {Epsilon, Epsilon, 41.5, 85.0, 148.0, 202.0, 239.0, 253.0, 261.0, 270.0, 284.0, 299.0},
        {Epsilon, Epsilon, 42.0, 87.0, 153.0, 205.0, 249.0, 259.0, 266.0, 278.0, 291.0, 310.0},
        {Epsilon, Epsilon, 47.0, 92.0, 160.0, 208.0, 255.0, 266.0, 272.0, 283.0, 296.0, 315.0},
        {Epsilon, Epsilon, 48.0, 95.0, 168.0, 215.0, 269.0, 276.0, 282.0, 298.0, 307.0, 325.0},
        {Epsilon, Epsilon, 51.0, 100.0, 173.0, 218.0, 279.0, 283.0, 290.0, 305.0, 320.0, 341.0},
        {Epsilon, Epsilon, 56.0, 102.0, 177.0, 228.0, 289.0, 294.0, 301.0, 317.0, 330.0, 351.0},
        {Epsilon, Epsilon, 59.0, 108.0, 184.0, 236.0, 298.0, 303.0, 312.0, 323.0, 337.0, 367.0},
        {Epsilon, Epsilon, 61.0, 115.0, 188.0, 238.0, 309.0, 317.0, 325.0, 339.0, 353.0, 380.0},
        {Epsilon, Epsilon, 63.0, 125.0, 193.0, 248.0, 319.0, 329.0, 338.0, 347.0, 364.0, 389.0},
        {Epsilon, Epsilon, 70.0, 132.0, 196.0, 256.0, 324.0, 340.0, 351.0, 375.0, 389.0, 408.0},
        {Epsilon, Epsilon, 75.0, 136.0, 208.0, 258.0, 329.0, 350.0, 360.0, 382.0, 402.0, 421.0},
        {Epsilon, Epsilon, 82.0, 142.0, 213.0, 262.0, 335.0, 357.0, 367.0, 396.0, 414.0, 431.0},
        {Epsilon, Epsilon, 86.0, 152.0, 216.0, 266.0, 339.0, 362.0, 372.0, 415.0, 430.0, 445.0},
        {Epsilon, Epsilon, 90.0, 158.0, 223.0, 268.0, 344.0, 368.0, 399.0, 430.0, 444.0, 460.0}};

    int level = resistance;
    if (level < 0) {
        level = 0;
    }
    if (level >= wattTableFirstDimension) {
        level = wattTableFirstDimension - 1;
    }
    double *watts_of_level = wattTable[level];
    int watt_setp = (Cadence.value() / 5.0);
    if (watt_setp >= 11) {
        return (((double)Cadence.value()) / 55.0) * watts_of_level[wattTableSecondDimension - 1];
    }
    double watt_base = watts_of_level[watt_setp];
    return (((watts_of_level[watt_setp + 1] - watt_base) / 5.0) * ((double)(((int)(Cadence.value())) % 5))) + watt_base;
}

void echelonrower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
