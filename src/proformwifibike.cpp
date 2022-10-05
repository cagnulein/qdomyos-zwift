#include "proformwifibike.h"
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

proformwifibike::proformwifibike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
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
    connect(refresh, &QTimer::timeout, this, &proformwifibike::update);
    refresh->start(200ms);

    bool ok = connect(&websocket, &QWebSocket::binaryMessageReceived, this, &proformwifibike::binaryMessageReceived);
    ok = connect(&websocket, &QWebSocket::textMessageReceived, this, &proformwifibike::characteristicChanged);
    ok = connect(&websocket, &QWebSocket::connected, [&]() { qDebug() << "connected!"; });
    ok = connect(&websocket, &QWebSocket::disconnected, [&]() {
        qDebug() << "disconnected!";
        connectToDevice();
    });

    connectToDevice();

    initRequest = true;    

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !virtualBike
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {
        QSettings settings;
        bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence) {
            qDebug() << "ios_peloton_workaround activated!";
            h = new lockscreen();
            h->virtualbike_ios();
        } else
#endif
#endif
            if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual bike interface..."));
            virtualBike =
                new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
            // connect(virtualBike,&virtualbike::debug ,this,& proformwifibike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &proformwifibike::changeInclination);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void proformwifibike::connectToDevice() {
    QSettings settings;
    // https://github.com/dawsontoth/zwifit/blob/e846501149a6c8fbb03af8d7b9eab20474624883/src/ifit.js
    websocket.open(QUrl("ws://" + settings.value(QZSettings::proformtdf4ip, QZSettings::default_proformtdf4ip).toString() + "/control"));
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
        emit debug(QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
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

void proformwifibike::forceResistance(resistance_t requestResistance) {

    QString send = "{\"type\":\"set\",\"values\":{\"Incline\":\"" + QString::number(requestResistance) + "\"}}";
    qDebug() << "forceResistance" << send;
    websocket.sendTextMessage(send);
}

void proformwifibike::innerWriteResistance() {
    if (requestResistance != -1) {
        if (requestResistance > max_resistance) {
            requestResistance = max_resistance;
        } else if (requestResistance < min_resistance) {
            requestResistance = min_resistance;
        } else if (requestResistance == 0) {
            requestResistance = 1;
        }

        if (requestResistance != currentResistance().value()) {
            emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
            if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike) &&
                (requestPower == 0 || requestPower == -1)) {
                forceResistance(requestResistance);
            }
        }
        requestResistance = -1;

        if (requestInclination != -100) {
            emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));
            forceResistance(requestInclination + gears()); // since this bike doesn't have the concept of resistance,
                                                           // i'm using the gears in the inclination
            requestInclination = -100;
        }
    }
}

void proformwifibike::update() {
    qDebug() << "websocket.state()" << websocket.state();

    if (initRequest) {
        initRequest = false;
        btinit();
        emit connectedAndDiscovered();
    } else if (websocket.state() == QAbstractSocket::ConnectedState) {
        update_metrics(true, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
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

bool proformwifibike::inclinationAvailableByHardware() {
    QSettings settings;
    bool proform_studio = settings.value(QZSettings::proform_studio, QZSettings::default_proform_studio).toBool();
    bool proform_tdf_10 = settings.value(QZSettings::proform_tdf_10, QZSettings::default_proform_tdf_10).toBool();

    if (proform_studio || proform_tdf_10)
        return true;
    else
        return false;
}

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
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery = settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

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

    if (!values[QStringLiteral("RPM")].isUndefined()) {
        double rpm = values[QStringLiteral("RPM")].toString().toDouble();
        Cadence = rpm;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
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

void proformwifibike::btinit() { initDone = true; }

void proformwifibike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

bool proformwifibike::connected() { return websocket.state() == QAbstractSocket::ConnectedState; }

void *proformwifibike::VirtualBike() { return virtualBike; }

void *proformwifibike::VirtualDevice() { return VirtualBike(); }

uint16_t proformwifibike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watts;
}
