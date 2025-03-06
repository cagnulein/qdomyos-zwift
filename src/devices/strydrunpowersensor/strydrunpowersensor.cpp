#include "strydrunpowersensor.h"

#include "virtualdevices/virtualtreadmill.h"
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

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

strydrunpowersensor::strydrunpowersensor(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &strydrunpowersensor::update);
    refresh->start(200ms);
}
/*
void strydrunpowersensor::writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool
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

void strydrunpowersensor::update() {
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
        QSettings settings;
        bool power_as_treadmill =
            settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();
        update_metrics(false, watts(), !power_as_treadmill);

        if (requestInclination != -100) {
            Inclination = treadmillInclinationOverrideReverse(requestInclination);
            qDebug() << QStringLiteral("writing incline ") << requestInclination;
            requestInclination = -100;
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }
    }
}

void strydrunpowersensor::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void strydrunpowersensor::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    qDebug() << "<<" << characteristic.uuid() << newValue.toHex(' ') << newValue.length();
    Q_UNUSED(characteristic);
    QDateTime now = QDateTime::currentDateTime();
    QSettings settings;
    bool power_as_treadmill =
        settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    if (characteristic.uuid() == QBluetoothUuid::CyclingPowerMeasurement) {
        lastPacket = newValue;

        uint16_t flags = (((uint16_t)((uint8_t)newValue.at(1)) << 8) | (uint16_t)((uint8_t)newValue.at(0)));
        bool cadence_present = false;
        bool wheel_revs = false;
        uint16_t time_division = 1024;
        uint8_t index = 4;

        if (newValue.length() > 3) {
            powerReceived = true;
            double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
            double vwatts = ((9.8 * weight) * (currentInclination().value() / 100.0));
            m_watt = (((uint16_t)((uint8_t)newValue.at(3)) << 8) | (uint16_t)((uint8_t)newValue.at(2))) + vwatts;
        }

        emit powerChanged(m_watt.value());
        emit debug(QStringLiteral("Current watt: ") + QString::number(m_watt.value()));

        if ((flags & 0x1) == 0x01) // Pedal Power Balance Present
        {
            index += 1;
        }
        if ((flags & 0x2) == 0x02) // Pedal Power Balance Reference
        {
        }
        if ((flags & 0x4) == 0x04) // Accumulated Torque Present
        {
            index += 2;
        }
        if ((flags & 0x8) == 0x08) // Accumulated Torque Source
        {
        }

        if ((flags & 0x10) == 0x10) // Wheel Revolution Data Present
        {
            cadence_present = true;
            wheel_revs = true;
            time_division = 2048;
        } else if ((flags & 0x20) == 0x20) // Crank Revolution Data Present
        {
            cadence_present = true;
        }

        if (watts())
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastRefreshPowerChanged.msecsTo(now))));
        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
        lastRefreshPowerChanged = now;
    } else if (characteristic.uuid() == QBluetoothUuid::HeartRateMeasurement) {
        if (newValue.length() > 1) {
            Heart = (uint8_t)newValue[1];
            emit onHeartRate((uint8_t)Heart.value());
        }

        emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACD)) {
        lastPacket = newValue;

        // inclination for NPE RUNN

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
            /*Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                    100.0;*/
            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
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
            /*
            if (firstDistanceCalculated)
                Distance += ((Speed.value() / 3600000.0) *
                             ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
            distanceEval = true;*/
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.inclination) {
            double inc =
                ((double)(((int16_t)((int8_t)newValue.at(index + 1)) << 8) | (int16_t)((uint8_t)newValue.at(index)))) /
                10.0;
            // steps of 0.5 only to send to the Inclination override function
            if(!areInclinationSettingsDefault()) {
                inc = qRound(inc * 2.0) / 2.0;
                Inclination = treadmillInclinationOverride(inc);
            } else {
                Inclination = inc;
            }
            index += 4; // the ramo value is useless
            emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
            bool stryd_inclination_instead_treadmill = settings.value(QZSettings::stryd_inclination_instead_treadmill, QZSettings::default_stryd_inclination_instead_treadmill).toBool();
            if(stryd_inclination_instead_treadmill)
                emit inclinationChanged(Inclination.value(), Inclination.value());
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
            /*KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));*/
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            /*
            if (firstDistanceCalculated &&
                watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal +=
                    ((((0.048 *
                            ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged.msecsTo(
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                    // kg * 3.5) / 200 ) / 60
            distanceEval = true;
*/
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        if (Flags.heartRate) {
            if (index < newValue.length()) {

                double heart = ((double)(((uint8_t)newValue.at(index))));
                emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
            } else {
                emit debug(QStringLiteral("Error on parsing heart!"));
            }
            // index += 1; //NOTE: clang-analyzer-deadcode.DeadStores
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
    } else if (characteristic.uuid() == QBluetoothUuid::RSCMeasurement) {
        uint8_t flags = (uint8_t)newValue.at(0);
        bool InstantaneousStrideLengthPresent = (flags & 0x01);
        bool TotalDistancePresent = (flags & 0x02) ? true : false;
        bool WalkingorRunningStatusbits = (flags & 0x04) ? true : false;
        bool double_cadence = settings
                                  .value(QZSettings::powr_sensor_running_cadence_double,
                                         QZSettings::default_powr_sensor_running_cadence_double)
                                  .toBool();
        double cadence_multiplier = 1.0;
        if (double_cadence)
            cadence_multiplier = 2.0;

        // Unit is in m/s with a resolution of 1/256
        uint16_t speedMs = (((uint16_t)((uint8_t)newValue.at(2)) << 8) | (uint16_t)((uint8_t)newValue.at(1)));
        double speed = (((double)speedMs) / 256.0) * 3.6; // km/h
        bool stryd_speed_instead_treadmill = settings.value(QZSettings::stryd_speed_instead_treadmill, QZSettings::default_stryd_speed_instead_treadmill).toBool();
        if(stryd_speed_instead_treadmill)
            emit speedChanged(speed);
        double cadence = (uint8_t)newValue.at(3) * cadence_multiplier;
        if (newValue.length() >= 6 && InstantaneousStrideLengthPresent) {
            instantaneousStrideLengthCMAvailableFromDevice = true;
            InstantaneousStrideLengthCM =
                (((uint16_t)((uint8_t)newValue.at(5)) << 8) | (uint16_t)((uint8_t)newValue.at(4))) / 2;
            emit instantaneousStrideLengthChanged(InstantaneousStrideLengthCM.value());
            qDebug() << QStringLiteral("Current InstantaneousStrideLengthCM:") << InstantaneousStrideLengthCM.value();
            if (InstantaneousStrideLengthCM.value() == 0) {
                GroundContactMS.setValue(0);
                VerticalOscillationMM.setValue(0);
                emit groundContactChanged(GroundContactMS.value());
                emit verticalOscillationChanged(VerticalOscillationMM.value());
                qDebug() << QStringLiteral("Current GroundContactMS:") << GroundContactMS.value();
                qDebug() << QStringLiteral("Current VerticalOscillationMM:") << VerticalOscillationMM.value();
            }
        }

        Cadence = cadence;
        emit cadenceChanged(cadence);
        if (power_as_treadmill) {
            Speed = speed;

            emit speedChanged(speed);
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCadenceChanged.msecsTo(now)));
            emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
            emit debug(QStringLiteral("Current Speed: ") + QString::number(speed));
            if (powerReceived == false) {
                m_watt = wattsCalc(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat(),
                                   Speed.value(), Inclination.value());
                emit debug(QStringLiteral("Current watt: ") + QString::number(m_watt.value()));
            }
        }
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(cadence));
        lastRefreshCadenceChanged = now;
    } else if (characteristic.uuid() == QBluetoothUuid::CSCMeasurement) {
        uint16_t _LastCrankEventTime = 0;
        double _CrankRevs = 0;
        uint16_t _LastWheelEventTime = 0;
        double _WheelRevs = 0;
        uint8_t battery = 0;
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
        if (power_as_treadmill) {
            Speed =
                Cadence.value() *
                settings.value(QZSettings::cadence_sensor_speed_ratio, QZSettings::default_cadence_sensor_speed_ratio)
                    .toDouble();

            emit speedChanged(Speed.value());
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCadenceChanged.msecsTo(now)));
            emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }
        lastRefreshCadenceChanged = now;

    } else if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("0000ff00-0000-1000-8000-00805f9b34fb"))) {
        if (newValue.length() == 5 && newValue.at(0) == 0x0f) {
            GroundContactMS = (((uint16_t)((uint8_t)newValue.at(2)) << 8) | (uint16_t)((uint8_t)newValue.at(1)));
            emit groundContactChanged(GroundContactMS.value());
            VerticalOscillationMM = (((uint16_t)((uint8_t)newValue.at(4)) << 8) | (uint16_t)((uint8_t)newValue.at(3)));
            emit verticalOscillationChanged(VerticalOscillationMM.value());
            qDebug() << QStringLiteral("Current GroundContactMS:") << GroundContactMS.value();
            qDebug() << QStringLiteral("Current VerticalOscillationMM:") << VerticalOscillationMM.value();
        }
    } else if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("7e78aa19-72cd-d3b8-a81f-5b7e589bea0f"))) {
        if (newValue.length() == 20 && newValue.at(0) == 0x32) {
            GroundContactMS = (((uint16_t)((uint8_t)newValue.at(5)) << 8) | (uint16_t)((uint8_t)newValue.at(4)));
            emit groundContactChanged(GroundContactMS.value());
            //VerticalOscillationMM = (((uint16_t)((uint8_t)newValue.at(4)) << 8) | (uint16_t)((uint8_t)newValue.at(3)));
            //emit verticalOscillationChanged(VerticalOscillationMM.value());
            qDebug() << QStringLiteral("Current GroundContactMS:") << GroundContactMS.value();
            qDebug() << QStringLiteral("Current VerticalOscillationMM:") << VerticalOscillationMM.value();
        }
    }

    if (!noVirtualDevice) {
#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
            Heart = (uint8_t)KeepAwakeHelper::heart();
            debug("Current Heart: " + QString::number(Heart.value()));
        }
#endif
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) && Heart.value() == 0) {
            update_hr_from_external();
        }
    }

    if (!noVirtualDevice) {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence =
            settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence && h && firstStateChanged) {
                    h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->workoutTrackingUpdate(Speed.value(), Cadence.value(), (uint16_t)m_watt.value(), calories().value());
            h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
        }
#endif
#endif
    }

    _ergTable.collectTreadmillData(Speed.value(), watts(), Inclination.value());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void strydrunpowersensor::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &strydrunpowersensor::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &strydrunpowersensor::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &strydrunpowersensor::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &strydrunpowersensor::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &strydrunpowersensor::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &strydrunpowersensor::descriptorRead);

            if(FORERUNNER && s->serviceUuid() != QBluetoothUuid::HeartRate && s->serviceUuid() != QBluetoothUuid::RunningSpeedAndCadence) {
                qDebug() << "skipping garmin services!" << s->serviceUuid();
                continue;
            }

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
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

    // ******************************************* virtual treadmill/bike init *************************************
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
        bool virtual_device_force_bike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                .toBool();
        if (virtual_device_enabled) {
            if (!virtual_device_force_bike) {
                debug("creating virtual treadmill interface...");
                auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &strydrunpowersensor::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &strydrunpowersensor::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                debug("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService,
                                              settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset).toInt(),
                                              settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f).toDouble());
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &strydrunpowersensor::changeInclinationRequested);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
            
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void strydrunpowersensor::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    if (grade < 0)
        grade = 0;
    changeInclination(grade, percentage);
}

void strydrunpowersensor::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void strydrunpowersensor::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void strydrunpowersensor::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void strydrunpowersensor::characteristicRead(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void strydrunpowersensor::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &strydrunpowersensor::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void strydrunpowersensor::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("strydrunpowersensor::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void strydrunpowersensor::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("strydrunpowersensor::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void strydrunpowersensor::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        if(bluetoothDevice.name().toUpper().startsWith("FORERUNNER")) {
            FORERUNNER = true;
            qDebug() << "FORERUNNER WORKAROUND!";
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &strydrunpowersensor::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &strydrunpowersensor::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &strydrunpowersensor::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &strydrunpowersensor::controllerStateChanged);

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

bool strydrunpowersensor::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t strydrunpowersensor::watts() { return m_watt.value(); }

void strydrunpowersensor::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
