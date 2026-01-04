#include "proformwifibike.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <QtXml>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

proformwifibike::proformwifibike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                 double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    m_rawWatt.setType(metric::METRIC_WATT);
    target_watts.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &proformwifibike::update);
    refresh->start(50ms);

    bool ok = connect(&websocket, &QWebSocket::binaryMessageReceived, this, &proformwifibike::binaryMessageReceived);
    ok = connect(&websocket, &QWebSocket::textMessageReceived, this, &proformwifibike::characteristicChanged);
    ok = connect(&websocket, &QWebSocket::connected, [&]() { qDebug() << "connected!"; });
    ok = connect(&websocket, &QWebSocket::disconnected, [&]() {
        qDebug() << "disconnected!";
        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
        connectToDevice();
    });

    ergModeSupported = true; // IMPORTANT, only for this bike

    connectToDevice();

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
            // connect(virtualBike,&virtualbike::debug ,this,& proformwifibike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &proformwifibike::changeInclination);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void proformwifibike::connectToDevice() {
    QSettings settings;
    // https://github.com/dawsontoth/zwifit/blob/e846501149a6c8fbb03af8d7b9eab20474624883/src/ifit.js
    websocket.open(QUrl("ws://" +
                        settings.value(QZSettings::proformtdf4ip, QZSettings::default_proformtdf4ip).toString() +
                        "/control"));
}

/*
void proformwifibike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic,
                                                         QByteArray((const char *)data, data_len));

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();
}*/

resistance_t proformwifibike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    QSettings settings;

    double watt_gain = settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
    double watt_offset = settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();

    for (resistance_t i = 1; i < max_resistance; i++) {
        if (((wattsFromResistance(i) * watt_gain) + watt_offset) <= power &&
            ((wattsFromResistance(i + 1) * watt_gain) + watt_offset) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest")
                     << ((wattsFromResistance(i) * watt_gain) + watt_offset)
                     << ((wattsFromResistance(i + 1) * watt_gain) + watt_offset) << power;
            return i;
        }
    }
    if (power < ((wattsFromResistance(1) * watt_gain) + watt_offset))
        return 1;
    else
        return max_resistance;
}

uint16_t proformwifibike::wattsFromResistance(resistance_t resistance) {

    if (currentCadence().value() == 0)
        return 0;

    switch (resistance) {
    case 0:
    case 1:
        // -13.5 + 0.999x + 0.00993x²
        return (-13.5 + (0.999 * currentCadence().value()) + (0.00993 * pow(currentCadence().value(), 2)));
    case 2:
        // -17.7 + 1.2x + 0.0116x²
        return (-17.7 + (1.2 * currentCadence().value()) + (0.0116 * pow(currentCadence().value(), 2)));

    case 3:
        // -17.5 + 1.24x + 0.014x²
        return (-17.5 + (1.24 * currentCadence().value()) + (0.014 * pow(currentCadence().value(), 2)));

    case 4:
        // -20.9 + 1.43x + 0.016x²
        return (-20.9 + (1.43 * currentCadence().value()) + (0.016 * pow(currentCadence().value(), 2)));

    case 5:
        // -27.9 + 1.75x+0.0172x²
        return (-27.9 + (1.75 * currentCadence().value()) + (0.0172 * pow(currentCadence().value(), 2)));

    case 6:
        // -26.7 + 1.9x + 0.0201x²
        return (-26.7 + (1.9 * currentCadence().value()) + (0.0201 * pow(currentCadence().value(), 2)));

    case 7:
        // -33.5 + 2.23x + 0.0225x²
        return (-33.5 + (2.23 * currentCadence().value()) + (0.0225 * pow(currentCadence().value(), 2)));

    case 8:
        // -36.5+2.5x+0.0262x²
        return (-36.5 + (2.5 * currentCadence().value()) + (0.0262 * pow(currentCadence().value(), 2)));

    case 9:
        // -38+2.62x+0.0305x²
        return (-38.0 + (2.62 * currentCadence().value()) + (0.0305 * pow(currentCadence().value(), 2)));

    case 10:
        // -41.2+2.85x+0.0327x²
        return (-41.2 + (2.85 * currentCadence().value()) + (0.0327 * pow(currentCadence().value(), 2)));

    case 11:
        // -43.4+3.01x+0.0359x²
        return (-43.4 + (3.01 * currentCadence().value()) + (0.0359 * pow(currentCadence().value(), 2)));

    case 12:
        // -46.8+3.23x+0.0364x²
        return (-46.8 + (3.23 * currentCadence().value()) + (0.0364 * pow(currentCadence().value(), 2)));

    case 13:
        // -49+3.39x+0.0371x²
        return (-49.0 + (3.39 * currentCadence().value()) + (0.0371 * pow(currentCadence().value(), 2)));

    case 14:
        // -53.4+3.55x+0.0383x²
        return (-53.4 + (3.55 * currentCadence().value()) + (0.0383 * pow(currentCadence().value(), 2)));

    case 15:
        // -49.9+3.37x+0.0429x²
        return (-49.9 + (3.37 * currentCadence().value()) + (0.0429 * pow(currentCadence().value(), 2)));

    case 16:
    default:
        // -47.1+3.25x+0.0464x²
        return (-47.1 + (3.25 * currentCadence().value()) + (0.0464 * pow(currentCadence().value(), 2)));
    }
}

// must be double because it's an inclination
void proformwifibike::forceResistance(double requestResistance) {

    if (tdf2) {
        QString send = "{\"type\":\"set\",\"values\":{\"Master State\":\"4\"}}";
        qDebug() << "forceResistance" << send;
        websocket.sendTextMessage(send);
    }

    double inc = qRound(requestResistance / 0.5) * 0.5;
    QString send;
    if (inclinationAvailableByHardware()) {
        if(max_incline_supported > 0 && inc > max_incline_supported)
            inc = max_incline_supported;
        send = "{\"type\":\"set\",\"values\":{\"Incline\":\"" + QString::number(inc) + "\"}}";
    } else {
        send = "{\"type\":\"set\",\"values\":{\"Resistance\":\"" + QString::number(requestResistance) + "\"}}";
    }

    qDebug() << "forceResistance" << send;
    websocket.sendTextMessage(send);
}

void proformwifibike::setTargetWatts(double watts) {

    QString send = "{\"type\":\"set\",\"values\":{\"Target Watts\":\"" + QString::number(watts) + "\"}}";
    qDebug() << "setTargetWatts" << send;
    websocket.sendTextMessage(send);
}

void proformwifibike::setWorkoutType(QString type) {

    QString send = "{\"type\":\"set\",\"values\":{\"Workout Type\":\"" + type + "\"}}";
    qDebug() << "setWorkoutType" << send;
    websocket.sendTextMessage(send);
}

void proformwifibike::innerWriteResistance() {
    QSettings settings;
    bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
    static QString last_mode = "MANUAL";

    if (requestResistance != -1 || lastGearValue != gears()) {
        resistance_t rR = requestResistance;
        if (rR == -1) {
            rR = Resistance.value();
        }

        if (rR > max_resistance) {
            rR = max_resistance;
        } else if (rR < min_resistance) {
            rR = min_resistance;
        } else if (rR == 0) {
            rR = 1;
        }

        if ((rR + gears() != currentResistance().value() || lastGearValue != gears()) && !inclinationAvailableByHardware() && requestInclination == -100) {
            emit debug(QStringLiteral("writing resistance ") + QString::number(rR));
            auto virtualBike = this->VirtualBike();
            if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike) &&
                (requestPower == 0 || requestPower == -1)) {
                forceResistance(rR + gears());
            }
        }
        requestResistance = -1;
    }

    if (requestPower > 0 && erg_mode) {
        if (last_mode.compare("WATTS_GOAL")) {
            last_mode = "WATTS_GOAL";
            setWorkoutType(last_mode);
        }
        double r = requestPower;
        if (settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble() <= 2.00) {
            if (settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble() != 1.0) {
                qDebug() << QStringLiteral("request watt value was ") << r
                         << QStringLiteral("but it will be transformed to")
                         << r / settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
            }
            r /= settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
        }
        if (settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble() < 0) {
            if (settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble() != 0.0) {
                qDebug() << QStringLiteral("request watt value was ") << r
                         << QStringLiteral("but it will be transformed to")
                         << r - settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
            }
            r -= settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
        }
        setTargetWatts(r);
    }

    if (requestInclination != -100 && !erg_mode && inclinationAvailableByHardware()) {
        if (last_mode.compare("MANUAL")) {
            last_mode = "MANUAL";
            setWorkoutType(last_mode);
        }
        emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));
        forceResistance(requestInclination + gears()); // since this bike doesn't have the concept of resistance,
                                                       // i'm using the gears in the inclination
        requestInclination = -100;
    }

    lastGearValue = gears();
}

void proformwifibike::update() {
    qDebug() << "websocket.state()" << websocket.state();

    if (initRequest) {
        initRequest = false;
        btinit();
        emit connectedAndDiscovered();
    } else if (websocket.state() == QAbstractSocket::ConnectedState) {
        update_metrics(false, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()) > 10000) {

            Speed = 0;
            m_watt = 0;
            Cadence = 0;            

            // the bike is not responding
            qDebug() << "bike not responding...Let's close the connection!";
            websocket.close();
        }

        innerWriteResistance();

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

bool proformwifibike::inclinationAvailableByHardware() { return max_incline_supported > 0; }

resistance_t proformwifibike::pelotonToBikeResistance(int pelotonResistance) {
    if (pelotonResistance <= 10) {
        return 1;
    }
    if (pelotonResistance <= 20) {
        return 2;
    }
    if (pelotonResistance <= 25) {
        return 3;
    }
    if (pelotonResistance <= 30) {
        return 4;
    }
    if (pelotonResistance <= 35) {
        return 5;
    }
    if (pelotonResistance <= 40) {
        return 6;
    }
    if (pelotonResistance <= 45) {
        return 7;
    }
    if (pelotonResistance <= 50) {
        return 8;
    }
    if (pelotonResistance <= 55) {
        return 9;
    }
    if (pelotonResistance <= 60) {
        return 10;
    }
    if (pelotonResistance <= 65) {
        return 11;
    }
    if (pelotonResistance <= 70) {
        return 12;
    }
    if (pelotonResistance <= 75) {
        return 13;
    }
    if (pelotonResistance <= 80) {
        return 14;
    }
    if (pelotonResistance <= 85) {
        return 15;
    }
    if (pelotonResistance <= 100) {
        return 16;
    }
    return Resistance.value();
}

void proformwifibike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void proformwifibike::binaryMessageReceived(const QByteArray &message) { characteristicChanged(message); }

void proformwifibike::characteristicChanged(const QString &newValue) {
    QDateTime now = QDateTime::currentDateTime();
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

    if (!values[QStringLiteral("Master State")].isUndefined()) {
        tdf2 = true;
        qDebug() << QStringLiteral("TDF2 mod enabled!");
    }

    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        if (!values[QStringLiteral("Current KPH")].isUndefined()) {
            double kph = values[QStringLiteral("Current KPH")].toString().toDouble();
            Speed = kph;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        } else if (!values[QStringLiteral("KPH")].isUndefined()) {
            double kph = values[QStringLiteral("KPH")].toString().toDouble();
            Speed = kph;
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

    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());

        Distance += ((Speed.value() / 3600000.0) *
                    ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
    }

    if (!values[QStringLiteral("RPM")].isUndefined()) {
        double rpm = values[QStringLiteral("RPM")].toString().toDouble();
        Cadence = rpm;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }
    }

    // some buggy TDF1 bikes send spurious wattage at the end with cadence = 0
    if (Cadence.value() > 0) {
        if (!values[QStringLiteral("Current Watts")].isUndefined()) {
            m_rawWatt = values[QStringLiteral("Current Watts")].toString().toDouble();
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                m_watt = m_rawWatt.value();
            emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
        } else if (!values[QStringLiteral("Watt attuali")].isUndefined()) {
            double watt = values[QStringLiteral("Watt attuali")].toString().toDouble();
            m_watt = watt;
            emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
        }
    } else {
        qDebug() << "watt to 0 due to cadence = 0";
        m_watt = 0;
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
    }

    if (!values[QStringLiteral("Actual Incline")].isUndefined()) {
        bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
        double incline = values[QStringLiteral("Actual Incline")].toString().toDouble();
        // if the bike has the inclination, QZ is using it to change the resistance when it's not in ERG mode.
        // so I would like to keep the real inclination value instead of showing to the user the modified inclination + gears.
        // this is very helpful when you're following a GPX for example
        if(inclinationAvailableByHardware() && !erg_mode)
            incline = incline - gears();
        Inclination = incline;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(incline));
    } else if (!values[QStringLiteral("Incline")].isUndefined()) {
        bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
        double incline = values[QStringLiteral("Incline")].toString().toDouble();
        // if the bike has the inclination, QZ is using it to change the resistance when it's not in ERG mode.
        // so I would like to keep the real inclination value instead of showing to the user the modified inclination + gears.
        // this is very helpful when you're following a GPX for example
        if(inclinationAvailableByHardware() && !erg_mode)
            incline = incline - gears();
        Inclination = incline;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(incline));
    }

    if (!values[QStringLiteral("Target Watts")].isUndefined()) {
        double watt = values[QStringLiteral("Target Watts")].toString().toDouble();
        target_watts = watt;
        emit debug(QStringLiteral("Target Watts: ") + QString::number(watts()));
    }

    if (!values[QStringLiteral("Resistance")].isUndefined()) {
        Resistance = values[QStringLiteral("Resistance")].toString().toDouble();
        emit debug(QStringLiteral("Resistance: ") + QString::number(Resistance.value()));
    }

    if (!values[QStringLiteral("Maximum Incline")].isUndefined()) {
        max_incline_supported = values[QStringLiteral("Maximum Incline")].toString().toDouble();
        emit debug(QStringLiteral("Maximum Incline Supported: ") + QString::number(max_incline_supported));
    }

    if (settings.value(QZSettings::gears_from_bike, QZSettings::default_gears_from_bike).toBool()) {
        if (!values[QStringLiteral("key")].isUndefined()) {
            QJsonObject key = values[QStringLiteral("key")].toObject();
            QJsonValue code = key.value("code");
            QJsonValue name = key.value("name");
            QJsonValue held = key.value("held");
            if(held.toString().contains(QStringLiteral("-1"))) {
                bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
                if(!erg_mode) {
                    double value = 0;
                    if (name.toString().contains(QStringLiteral("LEFT EXTERNAL GEAR DOWN"))) {
                        qDebug() << "LEFT EXTERNAL GEAR DOWN";
                        value = -0.5;
                    } else if (name.toString().contains(QStringLiteral("LEFT EXTERNAL GEAR UP"))) {
                        qDebug() << "LEFT EXTERNAL GEAR UP";
                        value = 0.5;
                    } else if (name.toString().contains(QStringLiteral("RIGHT EXTERNAL GEAR UP"))) {
                        qDebug() << "RIGHT EXTERNAL GEAR UP";
                        value = -5.0;
                    } else if (name.toString().contains(QStringLiteral("RIGHT EXTERNAL GEAR DOWN"))) {
                        qDebug() << "RIGHT EXTERNAL GEAR DOWN";
                        value = 5.0;
                    }
                    if (value != 0.0) {
                        setGears(gears() + value);
                        forceResistance(lastRawRequestedInclinationValue + gears()); // to force an immediate change
                    }
                } else {
                    double value = 0;
                    if (name.toString().contains(QStringLiteral("LEFT EXTERNAL GEAR DOWN"))) {
                        qDebug() << "LEFT EXTERNAL GEAR DOWN";
                        value = -10.0;
                    } else if (name.toString().contains(QStringLiteral("LEFT EXTERNAL GEAR UP"))) {
                        qDebug() << "LEFT EXTERNAL GEAR UP";
                        value = 10.0;
                    } else if (name.toString().contains(QStringLiteral("RIGHT EXTERNAL GEAR UP"))) {
                        qDebug() << "RIGHT EXTERNAL GEAR UP";
                        value = -50.0;
                    } else if (name.toString().contains(QStringLiteral("RIGHT EXTERNAL GEAR DOWN"))) {
                        qDebug() << "RIGHT EXTERNAL GEAR DOWN";
                        value = 50.0;
                    }
                    if (value != 0.0) {
                        changePower(requestPower + value);
                    }
                }
            }
        }
    }

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            now)))); //(( (0.048* Output in watts +1.19) * body weight in kg
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

    lastRefreshCharacteristicChanged = now;

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

void proformwifibike::btinit() { initDone = true; }

void proformwifibike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

bool proformwifibike::connected() { return websocket.state() == QAbstractSocket::ConnectedState; }

uint16_t proformwifibike::watts() { return m_watt.value(); }
