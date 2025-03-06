#include "cscbike.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif
#include <chrono>
#include "homeform.h"

using namespace std::chrono_literals;

cscbike::cscbike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &cscbike::update);
    refresh->start(200ms);
}
/*
void cscbike::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool
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

void cscbike::update() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    if (!noVirtualDevice) {
#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
            Heart = (uint8_t)KeepAwakeHelper::heart();
            debug("Current Heart: " + QString::number(Heart.value()));
        }
#endif
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            update_hr_from_external();
        }
    }

    m_watt = wattFromHR(false);
    emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));

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
        bool cadence_sensor_as_bike =
            settings.value(QZSettings::cadence_sensor_as_bike, QZSettings::default_cadence_sensor_as_bike).toBool();
        update_metrics(false, watts(), !cadence_sensor_as_bike);

        if(lastGoodCadence.secsTo(QDateTime::currentDateTime()) > 5 && !charNotified) {
            readMethod = true;
            qDebug() << "no cadence for 5 secs, switching to reading method";
        }

        if(readMethod && cadenceService) {
            cadenceService->readCharacteristic(cadenceChar);
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

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

void cscbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void cscbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    qDebug() << "characteristicChanged << " << characteristic.uuid() << newValue.toHex(' ') << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    // QString heartRateBeltName = //unused QString
    // settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    uint16_t _LastCrankEventTime = 0;
    double _CrankRevs = 0;
    uint16_t _LastWheelEventTime = 0;
    double _WheelRevs = 0;
    uint8_t battery = 0;

    charNotified = true;

    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2A19)) {
        battery = newValue.at(0);
        if(battery != battery_level)
            if(homeform::singleton())
                homeform::singleton()->setToastRequested(bluetoothDevice.name() + QStringLiteral(" Battery Level ") + QString::number(battery) + " %");
        battery_level = battery;
        qDebug() << QStringLiteral("battery: ") << battery;
        return;
    }

    if (characteristic.uuid() != QBluetoothUuid((quint16)0x2A5B)) {
        return;
    }

    lastPacket = newValue;

    bool CrankPresent = (newValue.at(0) & 0x02) == 0x02;
    bool WheelPresent = (newValue.at(0) & 0x01) == 0x01;
    qDebug() << QStringLiteral("CrankPresent: ") << CrankPresent;
    qDebug() << QStringLiteral("WheelPresent: ") << WheelPresent;

    uint8_t index = 1;

    if (WheelPresent) {
        _WheelRevs =
            (((uint32_t)((uint8_t)newValue.at(index + 3)) << 24) | ((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
             ((uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)));
        emit debug(QStringLiteral("Current Wheel Revs: ") + QString::number(_WheelRevs));
        index += 4;

        _LastWheelEventTime =
            (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
        emit debug(QStringLiteral("Current Wheel Event Time: ") + QString::number(_LastWheelEventTime));
        index += 2;
    }
    if (CrankPresent) {
        _CrankRevs = (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
        emit debug(QStringLiteral("Current Crank Revs: ") + QString::number(_CrankRevs));
        index += 2;
        _LastCrankEventTime =
            (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
        emit debug(QStringLiteral("Current Crank Event Time: ") + QString::number(_LastCrankEventTime));
    }

    if ((!CrankPresent || _CrankRevs == 0) && WheelPresent) {
        CrankRevs = _WheelRevs;
        LastCrankEventTime = _LastWheelEventTime;
    } else {
        CrankRevs = _CrankRevs;
        LastCrankEventTime = _LastCrankEventTime;
    }

    int16_t deltaT = LastCrankEventTime - oldLastCrankEventTime;
    if (deltaT < 0) {
        deltaT = LastCrankEventTime + 65535 - oldLastCrankEventTime;
    }

    if (CrankRevs != oldCrankRevs && deltaT) {
        double cadence = ((CrankRevs - oldCrankRevs) / deltaT) * 1024 * 60;
        if ((cadence >= 0 && cadence < 256 && CrankPresent) || (!CrankPresent && WheelPresent))
            Cadence = cadence;
        lastGoodCadence = now;
    } else if (lastGoodCadence.msecsTo(now) > 2000) {
        Cadence = 0;
    }
    emit cadenceChanged(Cadence.value());
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

    oldLastCrankEventTime = LastCrankEventTime;
    oldCrankRevs = CrankRevs;

    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = Cadence.value() *
                settings.value(QZSettings::cadence_sensor_speed_ratio, QZSettings::default_cadence_sensor_speed_ratio)
                    .toDouble();
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

    double ac = 0.01243107769;
    double bc = 1.145964912;
    double cc = -23.50977444;

    double ar = 0.1469553975;
    double br = -5.841344538;
    double cr = 97.62165482;

    if (Cadence.value() > 0) {
        m_pelotonResistance =
            (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                       (cr - (m_watt.value() * 132.0 /
                                              (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
               br) /
              (2.0 * ar)) *
             settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
            settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
        Resistance = m_pelotonResistance;
    } else {
        m_pelotonResistance = 0;
        Resistance = 0;
    }
    emit resistanceRead(Resistance.value());

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            now)))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = now;

    if (!noVirtualDevice) {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence =
            settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence && h && firstStateChanged) {
                    h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
        }
#endif
#endif
    }

    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void cscbike::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    QBluetoothUuid CyclingSpeedAndCadence(QBluetoothUuid::CyclingSpeedAndCadence);
    QBluetoothUuid Battery(QBluetoothUuid::BatteryService);
    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
#ifdef Q_OS_WINDOWS
        qDebug() << "windows workaround, check only CyclingSpeedAndCadence ftms service"
                 << (s->serviceUuid() == CyclingSpeedAndCadence);
        if (s->serviceUuid() == CyclingSpeedAndCadence)
#endif
        {
            if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
                qDebug() << QStringLiteral("not all services discovered");
                return;
            }
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {

            if(s->serviceUuid() == CyclingSpeedAndCadence) {
                qDebug() << "CyclingSpeedAndCadence found";
                cadenceService = s;
            }

            if(s->serviceUuid() != CyclingSpeedAndCadence && s->serviceUuid() != Battery) {
                //  No data from sensors and avatar won’t move in Zwift (even when data showed on first try) (Issue #2178)
                qDebug() << "avoid unwaned service";
                continue;
            }

            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &cscbike::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &cscbike::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &cscbike::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &cscbike::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &cscbike::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &cscbike::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                if(c.uuid() == QBluetoothUuid((quint16)0x2A5B)) {
                    qDebug() << "CyclingSpeedAndCadence char found";
                    cadenceChar = c;
                }
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle() << QStringLiteral("properties") << c.properties();
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
            }
        }
    }

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice() && !noVirtualDevice
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
            connect(virtualBike, &virtualbike::changeInclination, this, &cscbike::changeInclination);
            // connect(virtualBike,&virtualbike::debug ,this,&cscbike::debug);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void cscbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void cscbike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void cscbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void cscbike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');

    characteristicChanged(characteristic, newValue);
}

void cscbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif
    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
#ifdef Q_OS_WINDOWS
        QBluetoothUuid CyclingSpeedAndCadence(QBluetoothUuid::CyclingSpeedAndCadence);
        qDebug() << "windows workaround, check only the CyclingSpeedAndCadence service" << s << CyclingSpeedAndCadence
                 << (s == CyclingSpeedAndCadence);
        if (s == CyclingSpeedAndCadence)
#endif
        {
            gattCommunicationChannelService.append(m_control->createServiceObject(s));
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &cscbike::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails();
        }
    }
}

void cscbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("cscbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void cscbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("cscbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void cscbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &cscbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &cscbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &cscbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &cscbike::controllerStateChanged);

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

bool cscbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t cscbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void cscbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << "controllerStateChanged" << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
