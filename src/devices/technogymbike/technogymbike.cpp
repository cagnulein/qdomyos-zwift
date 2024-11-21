#include "technogymbike.h"
#include "homeform.h"
#include "virtualdevices/virtualbike.h"
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
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include <chrono>
#include "wheelcircumference.h"

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

using namespace std::chrono_literals;

technogymbike::technogymbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                   double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &technogymbike::update);
    refresh->start(settings.value(QZSettings::poll_device_time, QZSettings::default_poll_device_time).toInt());
    wheelCircumference::GearTable g;
    g.printTable();
}

bool technogymbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                   bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if(!customService) {
        qDebug() << QStringLiteral("customService is null!");
        return false;
    }
    
    if (wait_for_response) {
        connect(gattFTMSService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattFTMSService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharControlPointId.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        customService->writeCharacteristic(customWriteChar, *writeBuffer,
                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        customService->writeCharacteristic(customWriteChar, *writeBuffer);
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    return true;
}

void technogymbike::init() {
    if (initDone)
        return;

    initDone = true;
    initRequest = false;
}

void technogymbike::forcePower(int16_t requestPower) {
    // TODO
    uint8_t write[] = {FTMS_SET_TARGET_POWER, 0x00, 0x00};

    write[1] = ((uint16_t)requestPower) & 0xFF;
    write[2] = ((uint16_t)requestPower) >> 8;

    writeCharacteristic(write, sizeof(write), QStringLiteral("forcePower ") + QString::number(requestPower));

    powerForced = true;
}

uint16_t technogymbike::wattsFromResistance(double resistance) {
    return _ergTable.estimateWattage(Cadence.value(), resistance);
}

resistance_t technogymbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    if (Cadence.value() == 0)
        return 1;

    for (resistance_t i = 1; i < max_resistance; i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i)
                     << wattsFromResistance(i + 1) << power;
            return i;
        }
    }
    if (power < wattsFromResistance(1))
        return 1;
    else
        return max_resistance;
}

void technogymbike::forceResistance(resistance_t requestResistance) {

    uint8_t write[] = {0xf9, 0x01, 0x00, 0x00, 0xe4, 0x02, 0x18, 0x01, 0x03, 0x19, 0x00};

    double fr = (((double)requestResistance) * bikeResistanceGain) + ((double)bikeResistanceOffset);
    requestResistance = fr;

    write[2] = ((uint16_t)requestResistance * 10) & 0xFF;
    write[3] = ((uint16_t)requestResistance * 10) >> 8;

    writeCharacteristic(write, sizeof(write),
                        QStringLiteral("forceResistance ") + QString::number(requestResistance));
}

void technogymbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
    } else if (bluetoothDevice.isValid() &&
               m_control->state() == QLowEnergyController::DiscoveredState //&&
                                                                           // gattCommunicationChannelService &&
                                                                           // gattWriteCharacteristic.isValid() &&
                                                                           // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {
        update_metrics(false, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (powerForced && !autoResistance()) {
            qDebug() << QStringLiteral("disabling resistance ") << QString::number(currentResistance().value());
            powerForced = false;
            requestPower = -1;
            init();
            forceResistance(currentResistance().value());
        }

        auto virtualBike = this->VirtualBike();

        if (requestResistance != -1 || lastGearValue != gears()) {
            if (requestResistance > 100) {
                requestResistance = 100;
            } // TODO, use the bluetooth value
            else if (requestResistance == 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value() || lastGearValue != gears()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                // if the FTMS is connected, the ftmsCharacteristicChanged event will do all the stuff because it's a
                // FTMS bike. This condition handles the peloton requests                
                if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike) &&
                    (requestPower == 0 || requestPower == -1)) {
                    init();
                    forceResistance(requestResistance + (gears() * 5));
                }
            }
            requestResistance = -1;
        }
        if((virtualBike && virtualBike->ftmsDeviceConnected()) && lastGearValue != gears() && lastRawRequestedInclinationValue != -100 && lastPacketFromFTMS.length() >= 7) {
            qDebug() << "injecting fake ftms frame in order to send the new gear value ASAP" << lastPacketFromFTMS.toHex(' ');
            ftmsCharacteristicChanged(QLowEnergyCharacteristic(), lastPacketFromFTMS);
        }

        lastGearValue = gears();

        if (requestPower != -1) {
            qDebug() << QStringLiteral("writing power") << requestPower;
            init();
            forcePower(requestPower);
            requestPower = -1;
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

void technogymbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void technogymbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    bool heart = false;

    qDebug() << characteristic.uuid() << newValue.length() << QStringLiteral(" << ") << newValue.toHex(' ');

    lastPacket = newValue;

    if (characteristic.uuid() == charNotification) {


        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            uint16_t cad = lastCadenceValue = ((((uint16_t)((uint8_t)newValue.at(11)) << 8) |
                                                (uint16_t)((uint8_t)newValue.at(10))));
            QDateTime now = QDateTime::currentDateTime();
            Cadence = (((double)qAbs(lastCadenceChanged.msecsTo(now)) / (double)(cad - lastCadenceValue) / 1000.0) * 60.0);
            lastCadenceChanged = now;
            lastCadenceValue = cad;
        }
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged2AD2.msecsTo(now)));

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        double ac = 0.01243107769;
        double bc = 1.145964912;
        double cc = -23.50977444;

        double ar = 0.1469553975;
        double br = -5.841344538;
        double cr = 97.62165482;

        if (Cadence.value() && m_watt.value()) {
            m_pelotonResistance =
                (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                           (cr - (m_watt.value() * 132.0 /
                                                  (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
                   br) /
                  (2.0 * ar)) *
                 settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();

            Resistance = m_pelotonResistance;
            emit resistanceRead(Resistance.value());
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        }


        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                .startsWith(QStringLiteral("Disabled"))) {
                m_watt =  ((double)(((uint16_t)((uint8_t)newValue.at(3)) << 8) |
                               (uint16_t)((uint8_t)newValue.at(2))));
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
        }

        if (watts())
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged2AD2.msecsTo(
                          now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {

        }

        lastRefreshCharacteristicChanged2AD2 = now;
    } else {
        return;
    }

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) &&
        (!heart || Heart.value() == 0 || disable_hr_frommachinery)) {
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

    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void technogymbike::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    if (state != QLowEnergyService::ServiceState::ServiceDiscovered) {
        qDebug() << QStringLiteral("ignoring this state");
        return;
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &technogymbike::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &technogymbike::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &technogymbike::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &technogymbike::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &technogymbike::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &technogymbike::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            if (s->serviceUuid() != serviceUuid) {
                continue;
            }

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle() << c.properties();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == charWrite) {
                    qDebug() << QStringLiteral("custom service and Control Point found");
                    customWriteChar = c;
                    customService = s;
                }
            }
        }
    }

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
            auto virtualBike =
                new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
            // connect(virtualBike,&virtualbike::debug ,this,&technogymbike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &technogymbike::changeInclination);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &technogymbike::ftmsCharacteristicChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void technogymbike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    if (!autoResistance()) {
        qDebug() << "ignoring routing FTMS packet to the bike from virtualbike because of auto resistance OFF or resistance lvl mode is on"
                 << characteristic.uuid() << newValue.toHex(' ');
        return;
    }

    QByteArray b = newValue;

    {
        QSettings settings;
        qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');

        // handling gears
        if (b.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) {
            double min_inclination = settings.value(QZSettings::min_inclination, QZSettings::default_min_inclination).toDouble();
            lastPacketFromFTMS.clear();
            for(int i=0; i<b.length(); i++)
                lastPacketFromFTMS.append(b.at(i));
            qDebug() << "lastPacketFromFTMS" << lastPacketFromFTMS.toHex(' ');
            int16_t slope = (((uint8_t)b.at(3)) + (b.at(4) << 8));
            slope /= 10;
            if (gears() != 0) {
                slope += (gears() * 5);
            }

            if(min_inclination > (((double)slope) / 10.0)) {
                slope = min_inclination * 10;
                qDebug() << "grade override due to min_inclination " << min_inclination;
            }         

            qDebug() << "applying gears mod" << gears() << slope;

            uint8_t write[] = {0xf9, 0x01, 0x00, 0x00, 0xe4, 0x02, 0x18, 0x01, 0x03, 0x19, 0x00};

            write[2] = ((uint16_t)slope) & 0xFF;
            write[3] = ((uint16_t)slope) >> 8;

            writeCharacteristic(write, sizeof(write),
                                QStringLiteral("bridgingSlope ") + QString::number(requestResistance));
        } else if(b.at(0) == FTMS_SET_TARGET_POWER && b.length() > 2) {
            lastPacketFromFTMS.clear();
            for(int i=0; i<b.length(); i++)
                lastPacketFromFTMS.append(b.at(i));
            qDebug() << "lastPacketFromFTMS" << lastPacketFromFTMS.toHex(' ');
            int16_t power = (((uint8_t)b.at(1)) + (b.at(2) << 8));
            if (gears() != 0) {
                power += (gears() * 10);
            }

            qDebug() << "applying gears mod" << gears() << gearsZwiftRatio() << power;

            uint8_t write[] = {0xf8, 0x01, 0x55, 0x00, 0xe4, 0x02, 0x18, 0x01, 0x03, 0x19, 0x00};

            write[2] = ((uint16_t)power) & 0xFF;
            write[3] = ((uint16_t)power) >> 8;

            writeCharacteristic(write, sizeof(write),
                                QStringLiteral("bridgingPower ") + QString::number(requestResistance));
        }
    }
}

void technogymbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void technogymbike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void technogymbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void technogymbike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void technogymbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    initRequest = false;
    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &technogymbike::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void technogymbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("technogymbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void technogymbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("technogymbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

resistance_t technogymbike::pelotonToBikeResistance(int pelotonResistance) {
    return (pelotonResistance * max_resistance) / 100;
}

void technogymbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &technogymbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &technogymbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &technogymbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &technogymbike::controllerStateChanged);

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

bool technogymbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t technogymbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void technogymbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
