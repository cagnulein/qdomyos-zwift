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

#include "ic15dbike.h"
#include "virtualdevices/virtualbike.h"
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

#include "qzsettings.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#endif

using namespace std::chrono_literals;

const QString ic15dbike::CONSOLE_MODEL = QStringLiteral("BX_RK3326_A11_INSPIRE_IC15D");

bool ic15dbike::matchesConsoleModel(const QString &buildModel) { return buildModel == CONSOLE_MODEL; }

QString ic15dbike::androidBuildModel() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject model = QAndroidJniObject::getStaticObjectField<jstring>("android/os/Build", "MODEL");
    if (!model.isValid())
        return QString();
    return model.toString();
#else
    return QString();
#endif
}

bool ic15dbike::isIC15DConsole() { return matchesConsoleModel(androidBuildModel()); }

ic15dbike::ic15dbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                     double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    connect(refresh, &QTimer::timeout, this, &ic15dbike::update);
    refresh->start(200ms);

    myIC15D = new IC15DUART(this, QStringLiteral(IC15D_DEFAULT_SERIAL_PORT));
    myIC15D->start();

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
            connect(virtualBike, &virtualbike::changeInclination, this, &ic15dbike::changeInclination);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
}

void ic15dbike::update() {
    if (initRequest) {
        initRequest = false;
        btinit();
        emit connectedAndDiscovered();
        return;
    }

    int rpm = 0, level = 0;
    bool valid = false;

    myIC15D->getTelemetry(rpm, level, valid);

    if (valid) {
        Cadence = rpm;
        Resistance = level;
        m_pelotonResistance = level;

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        emit debug(QStringLiteral("Current Level: ") + QString::number(Resistance.value()));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    update_metrics(false, watts());

    // Intentionally no resistance/query write path: this is a receive-only driver (issue #4888).

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

void ic15dbike::btinit() { }

void ic15dbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

bool ic15dbike::connected() { return true; }

uint16_t ic15dbike::watts() { return (uint16_t)m_watt.value(); }
