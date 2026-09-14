/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include "daumbike.h"
#include "keepawakehelper.h"

#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

daumbike::daumbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                   double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &daumbike::update);
    refresh->start(200ms);

    QString serialPort =
        settings.value(QZSettings::daum_serialport, QZSettings::default_daum_serialport).toString();
    myDaum = new DaumUSB(this, serialPort, 9600);
    myDaum->start();

    ergModeSupported = true;
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
        bool cadenceSensor =
            settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadenceSensor) {
            h = new lockscreen();
            h->virtualbike_ios();
        } else
#endif
#endif
        if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual bike interface..."));
            virtualBike = new virtualbike(this, noWriteResistance, noHeartService,
                                          bikeResistanceOffset, bikeResistanceGain);
            connect(virtualBike, &virtualbike::changeInclination, this, &daumbike::changeInclination);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this,
                    &daumbike::ftmsCharacteristicChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
}

daumbike::~daumbike() {
    if (myDaum)
        myDaum->stop();
}

resistance_t daumbike::resistanceFromPowerRequest(uint16_t power) {
    if (power == 0)
        return 0;

    QSettings settings;
    double watt_gain = settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
    double watt_offset = settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
    double correctedPower = (power - watt_offset) / watt_gain;
    if (correctedPower < min_resistance)
        correctedPower = min_resistance;
    if (correctedPower > max_resistance)
        correctedPower = max_resistance;
    return correctedPower;
}

void daumbike::forceResistance(double requestResistance) {
    if (requestResistance < min_resistance)
        requestResistance = min_resistance;
    if (requestResistance > max_resistance)
        requestResistance = max_resistance;
    myDaum->setPower(requestResistance);
    qDebug() << "daumbike power target" << requestResistance;
}

void daumbike::innerWriteResistance() {
    if (requestResistance != -1) {
        if (requestResistance > max_resistance)
            requestResistance = max_resistance;
        else if (requestResistance < min_resistance)
            requestResistance = min_resistance;

        if (requestResistance != currentResistance().value()) {
            emit debug(QStringLiteral("writing Daum power target ") + QString::number(requestResistance));
            if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike) &&
                (requestPower == 0 || requestPower == -1)) {
                forceResistance(requestResistance);
            }
        }
        requestResistance = -1;
    }

    if (requestPower > 0) {
        forceResistance(requestPower);
        qDebug() << "Daum setting power" << requestPower;
    }

    if (requestInclination != -100) {
        forceInclination(requestInclination);
        requestInclination = -100;
    }
}

void daumbike::update() {
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

    int status;
    double power, heartRate, cadence, speed, distance;
    myDaum->getTelemetry(power, heartRate, cadence, speed, distance, status);

    Speed = speed;
    Distance = distance / 1000.0;
    Cadence = cadence;
    Resistance = power;
    m_watt = power;

    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += static_cast<uint16_t>(1024.0 / (Cadence.value() / 60.0));
    }

    if (watts()) {
        KCal += ((((0.048 * static_cast<double>(watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / static_cast<double>(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }

    if (!disable_hr_frommachinery && heartRate > 0)
        Heart = heartRate;

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = static_cast<uint8_t>(KeepAwakeHelper::heart());
    else
#endif
    if (disable_hr_frommachinery && heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        update_hr_from_external();
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadenceSensor =
        settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadenceSensor && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate(static_cast<uint8_t>(metrics_override_heartrate()));
    }
#endif
#endif

    update_metrics(false, watts());

    if (sec1Update++ == (1000 / refresh->interval()))
        sec1Update = 0;

    if (m_slopeControlEnabled && initDone) {
        if (m_lastFtmsCommand == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS)
            updateSlopeTargetPower();
        else
            m_slopeControlEnabled = false;
    }

    innerWriteResistance();

    if (requestStart != -1) {
        requestStart = -1;
        emit bikeStarted();
    }
    if (requestStop != -1)
        requestStop = -1;
}

resistance_t daumbike::pelotonToBikeResistance(int pelotonResistance) {
    double power = (pelotonResistance / 100.0) * max_resistance;
    if (power > 0 && power < min_resistance)
        power = min_resistance;
    return power;
}

void daumbike::btinit() {
    initDone = true;
}

void daumbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Daum serial device initialization triggered by: ") + device.name());
}

bool daumbike::connected() {
    return myDaum && myDaum->connected();
}

uint16_t daumbike::watts() {
    return m_watt.value();
}

void daumbike::changeInclination(double grade, double percentage) {
    bike::changeInclination(grade, percentage);
    Inclination = grade;
    m_currentSlopePercent = grade;
    m_slopeControlEnabled = true;
    updateSlopeTargetPower(true);
}

void daumbike::forceInclination(double inclination) {
    Inclination = inclination;
    m_currentSlopePercent = inclination;
    m_slopeControlEnabled = true;
    updateSlopeTargetPower(true);
}

void daumbike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                         const QByteArray &newValue) {
    Q_UNUSED(characteristic)
    if (!newValue.isEmpty())
        m_lastFtmsCommand = static_cast<FtmsControlPointCommand>(newValue.at(0));
}
