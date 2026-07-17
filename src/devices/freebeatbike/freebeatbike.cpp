/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "freebeatbike.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualbike.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

#include "qzsettings.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

using namespace std::chrono_literals;

freebeatbike::freebeatbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                           double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &freebeatbike::update);
    refresh->start(200ms);

    QString freebeatSerialPort =
        settings.value(QZSettings::freebeat_serialport, QZSettings::default_freebeat_serialport).toString();

    myFreebeat = new FreebeatUSB(this, freebeatSerialPort, FREEBEAT_BAUD);
    myFreebeat->start();

    initRequest = true;

    if (!firstStateChanged && !this->hasVirtualDevice()
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
            auto virtualBike =
                new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
            connect(virtualBike, &virtualbike::changeInclination, this, &freebeatbike::changeInclination);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
}

resistance_t freebeatbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << power;

    // Freebeat resistance is 1–100; power ≈ rpm * 4.56 * resistance/100
    // Without live RPM we do a simple linear mapping from power to resistance
    // assuming a cadence near 80 RPM: resistance ≈ power / (80 * 4.56 / 100)
    // Clamp to 1–100.
    double res = (double)power / (80.0 * 4.56 / 100.0);
    if (res < 1) res = 1;
    if (res > max_resistance) res = max_resistance;
    return (resistance_t)qRound(res);
}

void freebeatbike::forceResistance(resistance_t requestResistance) {
    if (!noWriteResistance && myFreebeat) {
        myFreebeat->setResistance((int)requestResistance);
        qDebug() << "freebeatbike: forceResistance" << requestResistance;
    }
}

void freebeatbike::innerWriteResistance() {
    if (requestResistance != -1) {
        if (requestResistance > max_resistance)
            requestResistance = max_resistance;
        else if (requestResistance < min_resistance)
            requestResistance = min_resistance;

        if (requestResistance != currentResistance().value()) {
            emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
            if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike) &&
                (requestPower == 0 || requestPower == -1)) {
                forceResistance(requestResistance);
            }
        }
        requestResistance = -1;
    }

    if (requestPower > 0) {
        resistance_t res = resistanceFromPowerRequest(requestPower);
        forceResistance(res);
        qDebug() << "freebeatbike: setting power =" << requestPower << "-> resistance" << res;
        requestPower = 0;
    }
}

void freebeatbike::update() {
    if (initRequest) {
        initRequest = false;
        btinit();
        emit connectedAndDiscovered();
        return;
    }

    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    int rpm = 0, res = 0;
    double speed = 0.0, watt = 0.0;
    bool valid = false;

    myFreebeat->getTelemetry(rpm, res, speed, watt, valid);

    if (valid) {
        Speed = speed;
        Cadence = rpm;
        m_watt = watt;
        Resistance = res;
        m_pelotonResistance = res;

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    }

    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (disable_hr_frommachinery && heartRateBeltName.startsWith(QStringLiteral("Disabled")))
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

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    update_metrics(false, watts());

    if (sec1Update++ == (1000 / refresh->interval())) {
        sec1Update = 0;
    }

    innerWriteResistance();

    if (requestStart != -1) {
        emit debug(QStringLiteral("starting..."));
        requestStart = -1;
        emit bikeStarted();
    }
    if (requestStop != -1) {
        emit debug(QStringLiteral("stopping..."));
        requestStop = -1;
    }
}

resistance_t freebeatbike::pelotonToBikeResistance(int pelotonResistance) {
    // Freebeat resistance 1–100 maps linearly from Peloton 0–100
    int res = pelotonResistance;
    if (res < 1) res = 1;
    if (res > (int)max_resistance) res = (int)max_resistance;
    return (resistance_t)res;
}

void freebeatbike::btinit() {
    initDone = true;
}

void freebeatbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

bool freebeatbike::connected() {
    return true;
}

uint16_t freebeatbike::watts() {
    return (uint16_t)m_watt.value();
}
