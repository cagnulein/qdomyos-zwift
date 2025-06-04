#include "proformtelnetbike.h"
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

proformtelnetbike::proformtelnetbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                 double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT);
    m_rawWatt.setType(metric::METRIC_WATT);
    target_watts.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &proformtelnetbike::update);
    refresh->start(200ms);

    bool ok = connect(&telnet, &QTelnet::newData, this, &proformtelnetbike::characteristicChanged);

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
                // connect(virtualBike,&virtualbike::debug ,this,& proformtelnetbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &proformtelnetbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void proformtelnetbike::connectToDevice() {
    QSettings settings;
    // https://github.com/dawsontoth/zwifit/blob/e846501149a6c8fbb03af8d7b9eab20474624883/src/ifit.js
    telnet.connectToHost(settings.value(QZSettings::proformtdf1ip, QZSettings::default_proformtdf1ip).toString(), 23);
    telnet.waitForConnected();
    telnet.sendData("./utconfig\n");
    QThread::sleep(1);
    telnet.sendData("2\n"); // modify variables

}

/*
void proformtelnetbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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

void proformtelnetbike::sendFrame(QByteArray frame) {
    telnet.sendData(frame);
    qDebug() << " >> " << frame;
}

void proformtelnetbike::update() {
    qDebug() << "websocket.state()" << telnet.isConnected();

    if (initRequest) {
        initRequest = false;
        btinit();
        emit connectedAndDiscovered();
    } else if (telnet.isConnected()) {
        update_metrics(false, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

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

bool proformtelnetbike::inclinationAvailableByHardware() { return true; }

resistance_t proformtelnetbike::pelotonToBikeResistance(int pelotonResistance) {
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

void proformtelnetbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void proformtelnetbike::characteristicChanged(const char *buff, int len) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();

    QByteArray newValue = QByteArray::fromRawData(buff, len);
    emit debug(QStringLiteral(" << ") + newValue);

    if(newValue.contains("Shared Memory Management Utility")) {
        emit debug(QStringLiteral("Ready to start the poll"));
        sendFrame("2\n"); // current watt
    } else if(newValue.contains("Enter New Value")) {
        if(poolIndex >= 4) {
            if(!erg_mode) {
                sendFrame((QString::number(requestInclination * 10.0) + "\n").toLocal8Bit()); // target incline
                qDebug() << "forceInclination" << requestInclination;
                requestInclination = -100;
            } else {
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
                sendFrame((QString::number(r) + "\n").toLocal8Bit()); // target watt
                qDebug() << "forceWatt" << r;
                requestPower = -1;
            }
            poolIndex = 0;
        } else {
            sendFrame("q\n"); // quit
        }
    } else if(newValue.contains("Enter Variable Offset")) {
        qDebug() << "poolIndex" << poolIndex;
        bool done = false;
        do {
            switch (poolIndex)
            {          
                case 0:
                    sendFrame("124\n"); // current watt
                    done = true;
                break;
                case 1:
                    sendFrame("40\n"); // current rpm
                    done = true;
                break;
                case 2:
                    sendFrame("34\n"); // current speed
                    done = true;
                break;
                case 3:
                    if(!erg_mode) {
                        if(requestInclination != -100) {
                            sendFrame("45\n"); // target incline
                            done = true;
                        }
                        else
                            poolIndex = 99;
                    } else {
                        if(requestPower != -1) {
                            sendFrame("125\n"); // target watt
                            done = true;
                        }
                        else
                            poolIndex = 99;
                    }
                break;
                default:
                break;
            }
            poolIndex++;
            if(poolIndex > 4)
                poolIndex = 0;
        } while(!done);            
    }

    QStringList packet = QString::fromLocal8Bit(newValue).split(" ");
    qDebug() << packet;    
    if (newValue.contains("Current Watts")) {
        m_rawWatt = packet[3].toDouble();
        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            m_watt = m_rawWatt.value();
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
    } else if (newValue.contains("Cur RPM")) {
        double RPM = packet[3].toDouble();
        Cadence = RPM;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }
    } else if (newValue.contains("Cur KPH")) {
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            double kph = packet[3].toDouble() / 10.0;
            Speed = kph;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        } else {
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        }
    }

    if (watts()) {
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
        Distance += ((Speed.value() / (double)3600.0) /
                     ((double)1000.0 / (double)(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }
    /*
        Resistance = resistance;
        m_pelotonResistance = (100 / 32) * Resistance.value();
        emit resistanceRead(Resistance.value());    */

    /*
    if (!disable_hr_frommachinery && !values[QStringLiteral("Chest Pulse")].isUndefined()) {
        Heart = values[QStringLiteral("Chest Pulse")].toString().toDouble();
        // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
        emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
    }*/

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

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

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

void proformtelnetbike::btinit() { initDone = true; }

void proformtelnetbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

bool proformtelnetbike::connected() { return telnet.isConnected(); }

uint16_t proformtelnetbike::watts() { return m_watt.value(); }


uint16_t proformtelnetbike::wattsFromResistance(resistance_t resistance) {
    return _ergTable.estimateWattage(Cadence.value(), resistance);
}

resistance_t proformtelnetbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    if (Cadence.value() == 0)
        return 1;

    for (resistance_t i = 1; i < maxResistance(); i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i)
                    << wattsFromResistance(i + 1) << power;
            return i;
        }
    }
    if (power < wattsFromResistance(1))
        return 1;
    else
        return maxResistance();
}
