#include "antbike.h"
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

antbike::antbike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &antbike::update);
    refresh->start(200ms);
}

void antbike::update() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    lockscreen hh;
    Cadence = hh.getFootCad();
    m_watt = hh.getPower();
    qDebug() << QStringLiteral("Current Garmin Cadence: ") << QString::number(Cadence.value());
    if (settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = metric::calculateSpeedFromPower(
            m_watt.value(), 0, Speed.value(), fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0),
            speedLimit());
    } else if (settings.value(QZSettings::speed_sensor_name, QZSettings::default_speed_sensor_name)
                   .toString()
                   .startsWith(QStringLiteral("Disabled"))) {
        Speed = hh.getSpeed();
    }
#endif
#endif

#ifdef Q_OS_ANDROID
    Cadence = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Garmin", "getFootCad", "()I");
    m_watt = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Garmin", "getPower", "()I");
    if (settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = metric::calculateSpeedFromPower(
            m_watt.value(), 0, Speed.value(), fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0),
            speedLimit());
    } else if (settings.value(QZSettings::speed_sensor_name, QZSettings::default_speed_sensor_name)
                   .toString()
                   .startsWith(QStringLiteral("Disabled"))) {
        Speed = QAndroidJniObject::callStaticMethod<jdouble>("org/cagnulen/qdomyoszwift/Garmin", "getSpeed", "()D");
    }
    qDebug() << QStringLiteral("Current Garmin Cadence: ") << QString::number(Cadence.value());
#endif
    
    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    if (requestInclination != -100) {
        Inclination = requestInclination;
        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
        requestInclination = -100;
    }

    update_metrics(false, watts());

    Distance += ((Speed.value() / (double)3600.0) /
                 ((double)1000.0 / (double)(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();    

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice() && !noVirtualDevice
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {
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
            connect(virtualBike, &virtualbike::changeInclination, this, &antbike::changeInclinationRequested);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &antbike::ftmsCharacteristicChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    if (!firstStateChanged)
        emit connectedAndDiscovered();
    firstStateChanged = 1;
    // ********************************************************************************************************

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

    if (Heart.value()) {
        static double lastKcal = 0;
        if (KCal.value() < 0) // if the user pressed stop, the KCAL resets the accumulator
            lastKcal = abs(KCal.value());
        KCal = metric::calculateKCalfromHR(Heart.average(), elapsed.value()) + lastKcal;
    }

    if (requestResistance != -1 && requestResistance != currentResistance().value()) {
        Resistance = requestResistance;
        m_pelotonResistance = requestResistance;
    }
}

void antbike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QByteArray b = newValue;
    qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');
}

void antbike::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

uint16_t antbike::wattsFromResistance(double resistance) {
    return _ergTable.estimateWattage(Cadence.value(), resistance);
}

resistance_t antbike::resistanceFromPowerRequest(uint16_t power) {
    return _ergTable.resistanceFromPowerRequest(power, Cadence.value(), maxResistance());
}


uint16_t antbike::watts() { return m_watt.value(); }
bool antbike::connected() { return true; }
