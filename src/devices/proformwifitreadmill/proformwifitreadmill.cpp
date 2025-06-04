#include "proformwifitreadmill.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <QtXml>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

proformwifitreadmill::proformwifitreadmill(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
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
    connect(refresh, &QTimer::timeout, this, &proformwifitreadmill::update);
    refresh->start(200ms);

    bool ok =
        connect(&websocket, &QWebSocket::binaryMessageReceived, this, &proformwifitreadmill::binaryMessageReceived);
    ok = connect(&websocket, &QWebSocket::textMessageReceived, this, &proformwifitreadmill::characteristicChanged);
    ok = connect(&websocket, &QWebSocket::connected, [&]() { qDebug() << "connected!"; });
    ok = connect(&websocket, &QWebSocket::disconnected, [&]() {
        qDebug() << "disconnected!";
        connectToDevice();
    });

    connectToDevice();

    initRequest = true;

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()) {
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        bool virtual_device_force_bike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                .toBool();
        if (virtual_device_enabled) {
            if (!virtual_device_force_bike) {
                debug("creating virtual treadmill interface...");
                auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &proformwifitreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &proformwifitreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                debug("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &proformwifitreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
            firstStateChanged = 1;
        }
    }
    // ********************************************************************************************************
}

void proformwifitreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    if (grade < 0)
        grade = 0;
    changeInclination(grade, percentage);
}

void proformwifitreadmill::connectToDevice() {
    QSettings settings;
    // https://github.com/dawsontoth/zwifit/blob/e846501149a6c8fbb03af8d7b9eab20474624883/src/ifit.js
    websocket.open(QUrl(
        "ws://" + settings.value(QZSettings::proformtreadmillip, QZSettings::default_proformtreadmillip).toString() +
        "/control"));
}

void proformwifitreadmill::forceSpeed(double requestSpeed) {
    QString send = "{\"type\":\"set\",\"values\":{\"KPH\":\"" + QString::number(requestSpeed) + "\"}}";
    qDebug() << "forceSpeed" << send;
    websocket.sendTextMessage(send);
    waitStatePkg = true;
}

void proformwifitreadmill::forceIncline(double requestIncline) {
    QString send = "{\"type\":\"set\",\"values\":{\"Incline\":\"" + QString::number(requestIncline) + "\"}}";
    qDebug() << "forceIncline" << send;
    websocket.sendTextMessage(send);
    waitStatePkg = true;
}

void proformwifitreadmill::update() {
    qDebug() << "websocket.state()" << websocket.state();

    if (initRequest) {
        initRequest = false;
        btinit();
        emit connectedAndDiscovered();
    } else if (websocket.state() == QAbstractSocket::ConnectedState) {
        update_metrics(true, watts());

        if (requestSpeed != -1 && waitStatePkg == false) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100 && waitStatePkg == false) {
            if (requestInclination < min_incline_supported)
                requestInclination = min_incline_supported;
            // only 0.5 steps ara available
            requestInclination = qRound(requestInclination * 2.0) / 2.0;
            if (requestInclination != currentInclination().value() && requestInclination >= min_incline_supported &&
                requestInclination <= max_incline_supported) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));

                forceIncline(requestInclination);
            }
            requestInclination = -100;
        }
    
        if (sec1Update++ == (2000 / refresh->interval())) {
            sec1Update = 0;
            if(waitStatePkg == false) {
                // keeping the connection alive
                qint64 msSinceLastMetrics = lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime());
                if (msSinceLastMetrics >= 2000) {
                    // keeping the connection alive - no metrics received for 2+ seconds
                    qDebug() << QStringLiteral("Keep-alive: No metrics for ") << msSinceLastMetrics << "ms";
                    forceSpeed(currentSpeed().value());
                }
            }
        }

        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

            // btinit();

            requestStart = -1;
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void proformwifitreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void proformwifitreadmill::binaryMessageReceived(const QByteArray &message) { characteristicChanged(message); }

void proformwifitreadmill::characteristicChanged(const QString &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    emit debug(QStringLiteral(" << ") + newValue);

    lastPacket = newValue;

    lastPacket = lastPacket.replace("à", "a");
    QByteArray payload = lastPacket.toLocal8Bit(); // JSON
    QJsonParseError parseError;
    QJsonDocument metrics = QJsonDocument::fromJson(payload, &parseError);

    QJsonObject json = metrics.object();
    QJsonValue values = json.value("values");

    if (!values[QStringLiteral("Current KPH")].isUndefined()) {
        double kph = values[QStringLiteral("Current KPH")].toString().toDouble();
        if(kph <= maximum_kph) {
            Speed = kph;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        } else {
            qDebug() << "filtering speed due to firmware bug";
        }
    } else if (!values[QStringLiteral("KPH")].isUndefined()) {
        double kph = values[QStringLiteral("KPH")].toString().toDouble();
        if(kph <= maximum_kph) {
            Speed = kph;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        } else {
            qDebug() << "filtering speed due to firmware bug";
        }
    }

    if (!values[QStringLiteral("Kilometers")].isUndefined()) {
        double odometer = values[QStringLiteral("Kilometers")].toString().toDouble();
        Distance = odometer;
        emit debug("Current Distance: " + QString::number(odometer));
    } else if (!values[QStringLiteral("Chilometri")].isUndefined()) {
        double odometer = values[QStringLiteral("Chilometri")].toString().toDouble();
        Distance = odometer;
        emit debug("Current Distance: " + QString::number(odometer));
    }

    if (!values[QStringLiteral("RPM")].isUndefined()) {
        double rpm = values[QStringLiteral("RPM")].toString().toDouble();
        Cadence = rpm;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

        if (Cadence.value() > 0) {
        }
    }

    if (!values[QStringLiteral("Current Watts")].isUndefined()) {
        double watt = values[QStringLiteral("Current Watts")].toString().toDouble();
        m_watts = watt;
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
    } else if (!values[QStringLiteral("Watt attuali")].isUndefined()) {
        double watt = values[QStringLiteral("Watt attuali")].toString().toDouble();
        m_watts = watt;
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
    }

    if (!values[QStringLiteral("Actual Incline")].isUndefined()) {
        double incline = values[QStringLiteral("Actual Incline")].toString().toDouble();
        Inclination = incline;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(incline));
    }

    if (!values[QStringLiteral("Incline")].isUndefined()) {
        double incline = values[QStringLiteral("Incline")].toString().toDouble();
        Inclination = incline;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(incline));
    }

    if (!values[QStringLiteral("Maximum Incline")].isUndefined()) {
        max_incline_supported = values[QStringLiteral("Maximum Incline")].toString().toDouble();
        emit debug(QStringLiteral("Maximum Incline Supported: ") + QString::number(max_incline_supported));
    }

    if (!values[QStringLiteral("Minimum Incline")].isUndefined()) {
        min_incline_supported = values[QStringLiteral("Minimum Incline")].toString().toDouble();
        emit debug(QStringLiteral("Minimum Incline Supported: ") + QString::number(min_incline_supported));
    }    

    if (!values[QStringLiteral("Maximum KPH")].isUndefined()) {
        maximum_kph = values[QStringLiteral("Maximum KPH")].toString().toDouble();
        emit debug(QStringLiteral("Maximum KPH: ") + QString::number(maximum_kph));
    }

    waitStatePkg = false;

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    /*
                                                                  Resistance = resistance;
                                                                  m_pelotonResistance = (100 / 32) * Resistance.value();
                                                                  emit resistanceRead(Resistance.value());    */

    if (!disable_hr_frommachinery && !values[QStringLiteral("Chest Pulse")].isUndefined()) {
        Heart = values[QStringLiteral("Chest Pulse")].toString().toDouble();
        // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
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
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    /*
    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));    */
}

void proformwifitreadmill::btinit() { initDone = true; }

void proformwifitreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

bool proformwifitreadmill::connected() { return websocket.state() == QAbstractSocket::ConnectedState; }

uint16_t proformwifitreadmill::watts() { return m_watts; }
