#include "proformwifitreadmill.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "virtualbike.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <QtXml>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

proformwifitreadmill::proformwifitreadmill(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
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
    initspeed();

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
    if (!firstStateChanged && !virtualTreadMill && !virtualBike) {
        bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        bool virtual_device_force_bike = settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike).toBool();
        if (virtual_device_enabled) {
            if (!virtual_device_force_bike) {
                debug("creating virtual treadmill interface...");
                virtualTreadMill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &proformwifitreadmill::debug);
                connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                        &proformwifitreadmill::changeInclinationRequested);
            } else {
                debug("creating virtual bike interface...");
                virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &proformwifitreadmill::changeInclinationRequested);
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
    qDebug() << "changeInclinationRequested" << percentage << grade;
    changeInclination(grade, percentage);
}

void proformwifitreadmill::connectToDevice() {
    QSettings settings;
    // https://github.com/dawsontoth/zwifit/blob/e846501149a6c8fbb03af8d7b9eab20474624883/src/ifit.js
    websocket.open(QUrl("ws://" + settings.value(QZSettings::proformtreadmillip, QZSettings::default_proformtreadmillip).toString() + "/control"));
}

void proformwifitreadmill::forceSpeed(double requestSpeed) {
    QString send = "{\"type\":\"set\",\"values\":{\"KPH\":\"" + QString::number(requestSpeed) + "\"}}";
    qDebug() << "forceSpeed" << send;
    websocket.sendTextMessage(send);
}

void proformwifitreadmill::forceIncline(double requestIncline) {
    QString send = "{\"type\":\"set\",\"values\":{\"Incline\":\"" + QString::number(requestIncline) + "\"}}";
    qDebug() << "forceIncline" << send;
    websocket.sendTextMessage(send);
}

void proformwifitreadmill::update() {
    qDebug() << "websocket.state()" << websocket.state();

    if (initRequest) {
        initRequest = false;
        btinit();
        initspeed();
        emit connectedAndDiscovered();
    } else if (websocket.state() == QAbstractSocket::ConnectedState) {
        update_metrics(true, watts());

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= speed_km_min  && requestSpeed <= speed_km_max ) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            if (requestInclination < incline_min )
                requestInclination = incline_min ;
            // only 0.5 steps ara available
            requestInclination = qRound(requestInclination * 2.0) / 2.0;
            if (requestInclination != currentInclination().value() && requestInclination >= incline_min  &&
                requestInclination <= incline_max ) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));

                forceIncline(requestInclination);
            }
            requestInclination = -100;
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
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

void proformwifitreadmill::initspeed() {
    for(int i=0; i<MAXPOINTS; i++) {
        ptime[i]=QDateTime::currentMSecsSinceEpoch();
        points[i]=0.0;
    }
}
double proformwifitreadmill::averagespeed(double kph) {
    QSettings settings;
    bool treadmill_speed_average = settings.value(QZSettings::treadmill_speed_average, QZSettings::default_treadmill_speed_average).toBool();
    if(!treadmill_speed_average) return kph;

    if (kph==0.0) initspeed();
    else {
        for (int i=MAXPOINTS-1; i>0; i--) {
            ptime[i]=ptime[i-1];
            points[i]=points[i-1];
        }
        points[0]=kph;
        ptime[0]=QDateTime::currentMSecsSinceEpoch();
        kph=0.0;
        qint64 time=0;
        for(int i=0; i<MAXPOINTS-1; i++) {
            kph+=points[i]*(ptime[i]-ptime[i+1]);
            time+=ptime[i]-ptime[i+1];
        }
        kph=round((kph/time)*10)/10;
        
    }
    return kph;
}
void proformwifitreadmill::characteristicChanged(const QString &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery = settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    if (QString::compare(newValue,QStringLiteral("{ }"))){
    	emit debug(QStringLiteral(" << ") + newValue);
    }

    lastPacket = newValue;

    lastPacket = lastPacket.replace("à", "a");
    QByteArray payload = lastPacket.toLocal8Bit(); // JSON
    QJsonParseError parseError;
    QJsonDocument metrics = QJsonDocument::fromJson(payload, &parseError);

    QJsonObject json = metrics.object();
    QJsonValue values = json.value("values");

    if (!values[QStringLiteral("Current KPH")].isUndefined()) {
        double kph = values[QStringLiteral("Current KPH")].toString().toDouble();
        Speed = averagespeed(kph);
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    } else if (!values[QStringLiteral("KPH")].isUndefined()) {
        double kph = values[QStringLiteral("KPH")].toString().toDouble();
        Speed = averagespeed(kph);
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    } else if (!values[QStringLiteral("Current MPH")].isUndefined()) {
        double mph = values[QStringLiteral("Current MPH")].toString().toDouble();
        Speed = averagespeed(mph*1.60934);
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
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
    } else if (!values[QStringLiteral("Incline")].isUndefined()) {
        double incline = values[QStringLiteral("Incline")].toString().toDouble();
        Inclination = incline;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(incline));
    }
    if (!values[QStringLiteral("Minimum Incline")].isUndefined()) {
        incline_min  = values[QStringLiteral("Minimum Incline")].toString().toDouble();
        emit debug(QStringLiteral("Minimum Incline: ") + QString::number(incline_min ));
    }
    if (!values[QStringLiteral("Maximum Incline")].isUndefined()) {
        incline_max  = values[QStringLiteral("Maximum Incline")].toString().toDouble();
        emit debug(QStringLiteral("Maximum Incline: ") + QString::number(incline_max ));
    }
    if (!values[QStringLiteral("Minimum KPH")].isUndefined()) {
        speed_km_min  = values[QStringLiteral("Minimum KPH")].toString().toDouble();
        emit debug(QStringLiteral("Minimum KPH: ") + QString::number(speed_km_min ));
    }
    if (!values[QStringLiteral("Maximum KPH")].isUndefined()) {
        speed_km_max  = values[QStringLiteral("Maximum KPH")].toString().toDouble();
        emit debug(QStringLiteral("Maximum KPH: ") + QString::number(speed_km_max ));
    }

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) * settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
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
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen h;
            long appleWatchHeartRate = h.heartRate();
            h.setKcal(KCal.value());
            h.setDistance(Distance.value());
            Heart = appleWatchHeartRate;
            debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
#endif
#endif
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
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

void *proformwifitreadmill::VirtualBike() { return virtualBike; }

void *proformwifitreadmill::VirtualTreadMill() { return virtualTreadMill; }

void *proformwifitreadmill::VirtualDevice() { return VirtualTreadMill(); }

uint16_t proformwifitreadmill::watts() { return m_watts; }
