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

#include "kettlerusbbike.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualbike.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

using namespace std::chrono_literals;

kettlerusbbike::kettlerusbbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                               double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    target_watts.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &kettlerusbbike::update);
    refresh->start(200ms); // Update every 200ms

    QString kettlerSerialPort =
        settings.value(QZSettings::kettler_usb_serialport, QZSettings::default_kettler_usb_serialport).toString();
    int kettlerBaudrate =
        settings.value(QZSettings::kettler_usb_baudrate, QZSettings::default_kettler_usb_baudrate).toInt();

    myKettler = new KettlerUSB(this, kettlerSerialPort, kettlerBaudrate);
    myKettler->start();

    ergModeSupported = true; // ERG mode supported

    // Try to discover joystick on adjacent serial ports
    QStringList possibleJoystickPorts = getAdjacentSerialPorts(kettlerSerialPort);
    for (const QString &port : possibleJoystickPorts) {
        qDebug() << "Trying to discover Kettler joystick on:" << port;
        myJoystick = new KettlerJoystick(this, port);
        if (myJoystick->discover(port)) {
            qDebug() << "Kettler joystick found on:" << port;
            // Connect joystick signals to gear functions
            connect(myJoystick, &KettlerJoystick::buttonPressed, this, &kettlerusbbike::handleJoystickButton);
            myJoystick->start();
            break;
        } else {
            delete myJoystick;
            myJoystick = nullptr;
        }
    }

    if (!myJoystick) {
        qDebug() << "Kettler joystick not found (this is optional)";
    }

    initRequest = true;

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
            connect(virtualBike, &virtualbike::changeInclination, this, &kettlerusbbike::changeInclination);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

resistance_t kettlerusbbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << power;

    QSettings settings;
    double watt_gain = settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
    double watt_offset = settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();

    // For Kettler, resistance is directly the power target in watts
    // Apply gain/offset correction
    double correctedPower = (power - watt_offset) / watt_gain;

    if (correctedPower < 0)
        correctedPower = 0;
    if (correctedPower > max_resistance)
        correctedPower = max_resistance;

    return correctedPower;
}

uint16_t kettlerusbbike::wattsFromResistance(resistance_t resistance) {
    // For Kettler ERG mode, resistance IS the power
    // This is a simplification since actual power depends on cadence
    // But Kettler bike controls power directly
    return (uint16_t)resistance;
}

void kettlerusbbike::forceResistance(double requestResistance) {
    // In Kettler, "resistance" is actually power in watts (ERG mode)
    myKettler->setPower(requestResistance);
    qDebug() << "forceResistance (power)" << requestResistance;
}

void kettlerusbbike::innerWriteResistance() {
    QSettings settings;
    bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();

    if (requestResistance != -1) {
        if (requestResistance > max_resistance) {
            requestResistance = max_resistance;
        } else if (requestResistance < min_resistance) {
            requestResistance = min_resistance;
        }

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
        myKettler->setPower(requestPower);
        qDebug() << "setting power = " << requestPower;
    }

    if (requestInclination != -100) {
        // Kettler USB doesn't support native inclination, but we use sim mode
        // to convert inclination to power (handled by forceInclination)
        emit debug(QStringLiteral("inclination change handled via sim mode: ") +
                   QString::number(requestInclination));
        forceInclination(requestInclination);
        requestInclination = -100;
    }
}

void kettlerusbbike::update() {
    if (initRequest) {
        initRequest = false;
        btinit();
        emit connectedAndDiscovered();
    } else {
        QSettings settings;
        QString heartRateBeltName =
            settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
        bool disable_hr_frommachinery =
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

        int Status;
        double Power, HeartRate, cadence, speed, distance;

        // Get latest telemetry from Kettler
        myKettler->getTelemetry(Power, HeartRate, cadence, speed, distance, Status);

        Speed = speed;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        Distance = distance / 1000.0; // Convert meters to km
        emit debug("Current Distance: " + QString::number(Distance.value()));

        Cadence = cadence;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        m_watt = Power;
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

        if (watts())
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));

        if (!disable_hr_frommachinery && HeartRate > 0) {
            Heart = HeartRate;
            emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
        }

        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (disable_hr_frommachinery && heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                update_hr_from_external();
            }
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

        update_metrics(false, watts());

        // updating the bike console every second
        if (sec1Update++ == (1000 / refresh->interval())) {
            sec1Update = 0;
        }

        // Update slope-based power if sim mode is active
        if (m_slopeControlEnabled && initDone) {
            updateSlopeTargetPower();
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
}

resistance_t kettlerusbbike::pelotonToBikeResistance(int pelotonResistance) {
    // Map Peloton resistance (0-100) to Kettler power (0-250W)
    // This is a simple linear mapping
    double power = (pelotonResistance / 100.0) * max_resistance;
    return power;
}

void kettlerusbbike::btinit() {
    initDone = true;
}

void kettlerusbbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

bool kettlerusbbike::connected() {
    return true;
}

uint16_t kettlerusbbike::watts() {
    return m_watt.value();
}

QStringList kettlerusbbike::getAdjacentSerialPorts(const QString &basePort) {
    QStringList ports;

    if (basePort.isEmpty()) {
        return ports;
    }

#ifdef Q_OS_ANDROID
    // On Android, USB discovery is done via Java enumeration, not port names
    // Just add a placeholder to trigger the discovery process
    ports.append("usb-joystick"); // Placeholder - actual discovery happens in Java code
#elif defined(WIN32)
    // On Windows, COM ports are COMx
    if (basePort.startsWith("COM")) {
        QString numStr = basePort.mid(3);
        bool ok;
        int baseNum = numStr.toInt(&ok);
        if (ok) {
            // Try adjacent COM ports (±5)
            for (int offset = 1; offset <= 5; offset++) {
                if (baseNum + offset <= 256) {
                    ports.append("COM" + QString::number(baseNum + offset));
                }
                if (baseNum - offset >= 1) {
                    ports.append("COM" + QString::number(baseNum - offset));
                }
            }
        }
    }
#else
    // On Linux/Mac, serial devices are typically /dev/ttyUSBx, /dev/ttyACMx, /dev/cu.*
    if (basePort.startsWith("/dev/ttyUSB")) {
        QString baseName = "/dev/ttyUSB";
        QString numStr = basePort.mid(baseName.length());
        bool ok;
        int baseNum = numStr.toInt(&ok);
        if (ok) {
            // Try adjacent ttyUSB ports (±5)
            for (int offset = 1; offset <= 5; offset++) {
                if (baseNum + offset >= 0) {
                    ports.append(baseName + QString::number(baseNum + offset));
                }
                if (baseNum - offset >= 0) {
                    ports.append(baseName + QString::number(baseNum - offset));
                }
            }
        }
    } else if (basePort.startsWith("/dev/ttyACM")) {
        QString baseName = "/dev/ttyACM";
        QString numStr = basePort.mid(baseName.length());
        bool ok;
        int baseNum = numStr.toInt(&ok);
        if (ok) {
            // Try adjacent ttyACM ports (±5)
            for (int offset = 1; offset <= 5; offset++) {
                if (baseNum + offset >= 0) {
                    ports.append(baseName + QString::number(baseNum + offset));
                }
                if (baseNum - offset >= 0) {
                    ports.append(baseName + QString::number(baseNum - offset));
                }
            }
        }
    }
#ifdef Q_OS_MACX
    else if (basePort.startsWith("/dev/cu.")) {
        // On macOS, try other cu.* devices
        // This is a simplified approach - in production you'd enumerate /dev/
        for (int i = 0; i < 10; i++) {
            QString port = QString("/dev/cu.usbserial-%1").arg(i);
            if (port != basePort) {
                ports.append(port);
            }
        }
    }
#endif
#endif

    return ports;
}

void kettlerusbbike::handleJoystickButton(int button) {
    qDebug() << "Kettler joystick button pressed:" << button;

    switch (button) {
        case KETTLER_JOY_UP_ARROW:
            qDebug() << "Gear UP triggered by joystick";
            gearUp();
            break;
        case KETTLER_JOY_DOWN_ARROW:
            qDebug() << "Gear DOWN triggered by joystick";
            gearDown();
            break;
        case KETTLER_JOY_LEFT_ARROW:
            qDebug() << "Left arrow pressed (not mapped)";
            // Future: could be used for other functions
            break;
        case KETTLER_JOY_RIGHT_ARROW:
            qDebug() << "Right arrow pressed (not mapped)";
            // Future: could be used for other functions
            break;
        default:
            qDebug() << "Unknown joystick button:" << button;
            break;
    }
void kettlerusbbike::changeInclination(double grade, double percentage) {
    qDebug() << "kettlerusbbike::changeInclination" << grade << percentage;

    // Call base class implementation to handle signals
    bike::changeInclination(grade, percentage);

    // Store current slope and enable sim mode
    Inclination = grade;
    m_currentSlopePercent = grade;
    m_slopeControlEnabled = true;

    // Force immediate power update
    updateSlopeTargetPower(true);
}

void kettlerusbbike::forceInclination(double inclination) {
    qDebug() << "kettlerusbbike::forceInclination" << inclination;

    // Store current slope and enable sim mode
    Inclination = inclination;
    m_currentSlopePercent = inclination;
    m_slopeControlEnabled = true;

    // Force immediate power update
    updateSlopeTargetPower(true);
}
