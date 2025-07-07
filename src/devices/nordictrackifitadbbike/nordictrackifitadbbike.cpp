#include "nordictrackifitadbbike.h"
#include "virtualdevices/virtualbike.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QProcess>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

using namespace std::chrono_literals;

nordictrackifitadbbikeLogcatAdbThread::nordictrackifitadbbikeLogcatAdbThread(QString s) { Q_UNUSED(s) }

void nordictrackifitadbbikeLogcatAdbThread::run() {
    QSettings settings;
    QString ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
    runAdbCommand("connect " + ip);

    while (1) {
        runAdbTailCommand("logcat");
        if(adbCommandPending.length() != 0) {
            runAdbCommand(adbCommandPending);
            adbCommandPending = "";
        }
        msleep(100);
    }
}

QString nordictrackifitadbbikeLogcatAdbThread::runAdbCommand(QString command) {
#ifdef Q_OS_WINDOWS
    QProcess process;
    emit debug("adb >> " + command);
    process.start("adb/adb.exe", QStringList(command.split(' ')));
    process.waitForFinished(-1); // will wait forever until finished

    QString out = process.readAllStandardOutput();
    QString err = process.readAllStandardError();

    emit debug("adb << OUT " + out);
    emit debug("adb << ERR" + err);
#else
    QString out;
#endif
    return out;
}

bool nordictrackifitadbbikeLogcatAdbThread::runCommand(QString command) {
    if(adbCommandPending.length() == 0) {
        adbCommandPending = command;
        return true;
    }
    return false;
}

void nordictrackifitadbbikeLogcatAdbThread::runAdbTailCommand(QString command) {
#ifdef Q_OS_WINDOWS
    auto process = new QProcess;
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process, this]() {
        QString output = process->readAllStandardOutput();
        // qDebug() << "adbLogCat STDOUT << " << output;
        QStringList lines = output.split('\n', Qt::SplitBehaviorFlags::SkipEmptyParts);
        bool wattFound = false;
        bool hrmFound = false;
        foreach (QString line, lines) {
            if (line.contains("Changed KPH")) {
                emit debug(line);
                speed = line.split(' ').last().toDouble();
            } else if (line.contains("Changed Grade")) {
                emit debug(line);
                inclination = line.split(' ').last().toDouble();
            } else if (line.contains("Changed Watts")) {
                emit debug(line);
                watt = line.split(' ').last().toDouble();
                wattFound = true;
            } else if (line.contains("HeartRateDataUpdate")) {
                emit debug(line);
                QStringList splitted = line.split(' ', Qt::SkipEmptyParts);
                if (splitted.length() > 14) {
                    hrm = splitted[14].toInt();
                    hrmFound = true;
                }
            }
        }
        emit onSpeedInclination(speed, inclination);
        if (wattFound)
            emit onWatt(watt);
        if (hrmFound)
            emit onHRM(hrm);
#ifdef Q_OS_WINDOWS        
        if(adbCommandPending.length() != 0) {
            runAdbCommand(adbCommandPending);
            adbCommandPending = "";
        }
#endif                                
    });
    QObject::connect(process, &QProcess::readyReadStandardError, [process, this]() {
        auto output = process->readAllStandardError();
        emit debug("adbLogCat ERROR << " + output);
    });
    emit debug("adbLogCat >> " + command);
    process->start("adb/adb.exe", QStringList(command.split(' ')));
    process->waitForFinished(-1);
#endif
}

nordictrackifitadbbike::nordictrackifitadbbike(bool noWriteResistance, bool noHeartService,
                                               int8_t bikeResistanceOffset, double bikeResistanceGain) {
    QSettings settings;
    bool nordictrack_ifit_adb_remote =
        settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote)
            .toBool();
    bool nordictrackadbbike_gear_resistance_mode =
        settings.value(QZSettings::nordictrackadbbike_gear_resistance_mode, QZSettings::default_nordictrackadbbike_gear_resistance_mode)
            .toBool();
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &nordictrackifitadbbike::update);
    ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
    refresh->start(200ms);

    socket = new QUdpSocket(this);
    bool result = socket->bind(QHostAddress::AnyIPv4, 8002);
    qDebug() << result;
    processPendingDatagrams();
    connect(socket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    initRequest = true;

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()) {
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
            qDebug() << QStringLiteral("creating virtual bike interface...");
            auto virtualBike =
                new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
            // connect(virtualBike,&virtualbike::debug ,this,&echelonconnectsport::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &nordictrackifitadbbike::changeInclination);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************

    // Initialize gRPC service on Android
#ifdef Q_OS_ANDROID
    initializeGrpcService();
    if (grpcInitialized) {
        startGrpcMetricsUpdates();
    }
#endif

    if (nordictrack_ifit_adb_remote) {
#ifdef Q_OS_ANDROID
        QAndroidJniObject IP = QAndroidJniObject::fromString(ip).object<jstring>();
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "createConnection",
                                                  "(Ljava/lang/String;Landroid/content/Context;)V",
                                                  IP.object<jstring>(), QtAndroid::androidContext().object());
#elif defined Q_OS_WIN
        logcatAdbThread = new nordictrackifitadbbikeLogcatAdbThread("logcatAdbThread");
        /*connect(logcatAdbThread, &nordictrackifitadbbikeLogcatAdbThread::onSpeedInclination, this,
                &nordictrackifitadbbike::onSpeedInclination);
        connect(logcatAdbThread, &nordictrackifitadbbikeLogcatAdbThread::onWatt, this,
                &nordictrackifitadbbike::onWatt);*/
        connect(logcatAdbThread, &nordictrackifitadbbikeLogcatAdbThread::onHRM, this, &nordictrackifitadbbike::onHRM);
        connect(logcatAdbThread, &nordictrackifitadbbikeLogcatAdbThread::debug, this, &nordictrackifitadbbike::debug);
        logcatAdbThread->start();
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
        h->adb_connect(ip.toStdString().c_str());
#endif
#endif
    }
}

bool nordictrackifitadbbike::inclinationAvailableByHardware() { 
    QSettings settings;
    bool proform_studio_NTEX71021 =
    settings.value(QZSettings::proform_studio_NTEX71021, QZSettings::default_proform_studio_NTEX71021)
        .toBool();
    bool nordictrackadbbike_resistance = settings.value(QZSettings::nordictrackadbbike_resistance, QZSettings::default_nordictrackadbbike_resistance).toBool();
    bool nordictrackadbbike_gear_resistance_mode = settings.value(QZSettings::nordictrackadbbike_gear_resistance_mode, QZSettings::default_nordictrackadbbike_gear_resistance_mode).toBool();

    // In gear resistance mode, inclination is available since gears handle resistance separately
    if (nordictrackadbbike_gear_resistance_mode)
        return true;
    
    if(proform_studio_NTEX71021 || nordictrackadbbike_resistance)
        return false;   
    else
        return true; 
}

double nordictrackifitadbbike::getDouble(QString v) {
    QChar d = QLocale().decimalPoint();
    if (d == ',') {
        v = v.replace('.', ',');
    }
    return QLocale().toDouble(v);
}

void nordictrackifitadbbike::processPendingDatagrams() {
    qDebug() << "in !";
    QHostAddress sender;
    QSettings settings;
    uint16_t port;
    bool freemotion_coachbike_b22_7 = settings.value(QZSettings::freemotion_coachbike_b22_7, QZSettings::default_freemotion_coachbike_b22_7).toBool();
    while (socket->hasPendingDatagrams()) {
        QDateTime now = QDateTime::currentDateTime();
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size(), &sender, &port);
        lastSender = sender;
        qDebug() << "Message From :: " << sender.toString();
        qDebug() << "Port From :: " << port;
        qDebug() << "Message :: " << datagram;

        QString ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
        QString heartRateBeltName =
            settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
        double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
        bool nordictrackadbbike_resistance = settings.value(QZSettings::nordictrackadbbike_resistance, QZSettings::default_nordictrackadbbike_resistance).toBool();

        double speed = 0;
        double cadence = 0;
        double resistance = 0;
        double gear = 0;
        double watt = 0;
        double grade = 0;
        QStringList lines = QString::fromLocal8Bit(datagram.data()).split("\n");
        foreach (QString line, lines) {
            qDebug() << line;

            if (line.contains(QStringLiteral("Changed KPH")) && !settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    speed = getDouble(aValues.last());
                    Speed = speed;
                }
            } else if (line.contains(QStringLiteral("Changed RPM"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    cadence = getDouble(aValues.last());
                    Cadence = cadence;
                }
            } else if (line.contains(QStringLiteral("Changed CurrentGear"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    gear = getDouble(aValues.last());
                    
                    if (nordictrackadbbike_gear_resistance_mode) {
                        // In gear resistance mode: gears control resistance via gRPC, store for later use
                        lastGearValue = gear;
                        // Set gRPC resistance if gRPC is available
                        #ifdef Q_OS_ANDROID
                        if (grpcInitialized) {
                            setGrpcResistance(gear);
                        }
                        #endif
                        qDebug() << "Gear resistance mode: gear" << gear << "set as resistance via gRPC";
                    } else if (!nordictrackadbbike_resistance) {
                        // Standard mode: gears control inclination (existing behavior)
                        Resistance = gear;
                        emit resistanceRead(Resistance.value());
                    }
                    // In resistance mode (nordictrackadbbike_resistance), gears are ignored for resistance
                    
                    gearsAvailable = true;
                }
            } else if (line.contains(QStringLiteral("Changed Resistance"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    resistance = getDouble(aValues.last());
                    if(freemotion_coachbike_b22_7)
                        m_pelotonResistance = (100 / 24) * resistance;
                    else
                        m_pelotonResistance = bikeResistanceToPeloton(resistance);
                    qDebug() << QStringLiteral("Current Peloton Resistance: ") << m_pelotonResistance.value()
                             << resistance;
                    if(!gearsAvailable && !nordictrackadbbike_resistance) {
                        Resistance = resistance;
                        emit resistanceRead(Resistance.value());
                    }
                }
            } else if (line.contains(QStringLiteral("Changed Watts"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    watt = getDouble(aValues.last());
                    m_watt = watt;
                }
            } else if (line.contains(QStringLiteral("Changed Grade"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    grade = getDouble(aValues.last());
                    Inclination = grade;
                }
            }
        }

        if (settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        }

        bool proform_studio_NTEX71021 =
            settings.value(QZSettings::proform_studio_NTEX71021, QZSettings::default_proform_studio_NTEX71021).toBool();
        bool nordictrack_ifit_adb_remote =
            settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote)
                .toBool();
        double inclination_delay_seconds = settings.value(QZSettings::inclination_delay_seconds, QZSettings::default_inclination_delay_seconds).toDouble();
        bool proform_tdf_10_0 = settings.value(QZSettings::proform_tdf_10_0, QZSettings::default_proform_tdf_10_0).toBool();                        

        // only resistance
        if(proform_studio_NTEX71021 || nordictrackadbbike_resistance) {
            if (nordictrack_ifit_adb_remote) {
                if (requestResistance != -1) {
                    if (requestResistance != currentResistance().value()) {
                        int x1 = 950;
                        int y2 = (int)(493 - (13.57 * (requestResistance - 1)));
                        int y1Resistance = (int)(493 - (13.57 * currentResistance().value()));

                        if(proform_tdf_10_0) {
                            x1 = 1175;
                            y2 = (int)(590 - (15.91 * requestResistance));
                            y1Resistance = (int)(590 - (15.91 * currentResistance().value()));
                            Resistance = requestResistance;
                            emit resistanceRead(Resistance.value());
                        }
                        else if(!proform_studio_NTEX71021) { // s22i default
                            x1 = 1920 - 75;
                            y2 = (int)(803 - (23.777 * requestResistance));
                            y1Resistance = (int)(803 - (23.777 * currentResistance().value()));
                            Resistance = requestResistance;
                            emit resistanceRead(Resistance.value());
                        }
                        
                        lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Resistance) + " " +
                                      QString::number(x1) + " " + QString::number(y2) + " 200";
                        qDebug() << " >> " + lastCommand;
#ifdef Q_OS_ANDROID
                        QAndroidJniObject command = QAndroidJniObject::fromString(lastCommand).object<jstring>();
                        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote",
                                                                  "sendCommand", "(Ljava/lang/String;)V",
                                                                  command.object<jstring>());
#elif defined(Q_OS_WIN)
                        if (logcatAdbThread)
                            logcatAdbThread->runCommand("shell " + lastCommand);
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
                        h->adb_sendcommand(lastCommand.toStdString().c_str());
#endif
#endif
                    }
                }

                requestResistance = -1;
            }            
            QByteArray message = (QString::number(requestResistance).toLocal8Bit()) + ";";
            requestResistance = -1;
            int ret = socket->writeDatagram(message, message.size(), sender, 8003);
            qDebug() << QString::number(ret) + " >> " + message;                
        }
        // since the motor of the bike is slow, let's filter the inclination changes to more than 4 seconds
        else if (lastInclinationChanged.secsTo(now) > inclination_delay_seconds) {
            lastInclinationChanged = now;
            if (nordictrack_ifit_adb_remote) {
                bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();
                
                // Check if erg_mode has been disabled and we had an active watts target
                if (!erg_mode && lastErgMode && hasActiveWattsTarget) {
                    qDebug() << "ERG mode disabled, disabling constant watts target";
                    disableGrpcWatts();
                }
                lastErgMode = erg_mode;
                
                if (requestInclination != -100 && erg_mode && requestResistance != -100) {
                    qDebug() << "forcing inclination based on the erg mode resistance request of" << requestResistance;
                    requestInclination = requestResistance;
                    requestResistance = -100;
                }
                if (requestInclination != -100) {
                    double inc = qRound(requestInclination / 0.5) * 0.5;
                    if (inc != currentInclination().value()) {
                        bool proform_studio = settings.value(QZSettings::proform_studio, QZSettings::default_proform_studio).toBool();                        
                        int x1 = 75;
                        
                        // In gear resistance mode, gears control resistance separately via gRPC, so don't add to inclination
                        double inclinationValue = nordictrackadbbike_gear_resistance_mode ? inc : (inc + gears());
                        
                        if (nordictrackadbbike_gear_resistance_mode) {
                            qDebug() << "Gear resistance mode: setting inclination to" << inc << "(not adding gears)";
                        }
                        
                        int y2 = (int)(616.18 - (17.223 * inclinationValue));
                        int y1Resistance = (int)(616.18 - (17.223 * currentInclination().value()));

                        if(proform_studio || freemotion_coachbike_b22_7) {
                            x1 = 1827;
                            y2 = (int)(806 - (21.375 * inclinationValue));
                            y1Resistance = (int)(806 - (21.375 * currentInclination().value()));
                        } else if(proform_tdf_10_0) {
                            x1 = 75;
                            y2 = (int)(477 - (12.5 * inclinationValue));
                            y1Resistance = (int)(477 - (12.5 * currentInclination().value()));
                        }

                        lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Resistance) + " " +
                                      QString::number(x1) + " " + QString::number(y2) + " 200";
                        qDebug() << " >> " + lastCommand;
#ifdef Q_OS_ANDROID
                        QAndroidJniObject command = QAndroidJniObject::fromString(lastCommand).object<jstring>();
                        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote",
                                                                  "sendCommand", "(Ljava/lang/String;)V",
                                                                  command.object<jstring>());
#elif defined(Q_OS_WIN)
                        if (logcatAdbThread)
                            logcatAdbThread->runCommand("shell " + lastCommand);
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
                        h->adb_sendcommand(lastCommand.toStdString().c_str());
#endif
#endif
                        // this bike has both inclination and resistance, let's try to handle both
                        // the Original Poster doesn't want anymore, but maybe it will be useful in the future
                        /*
                        if(freemotion_coachbike_b22_7) {
                            int x1 = 75;
                            int y2 = (int)(616.18 - (17.223 * (inc + gears())));
                            int y1Resistance = (int)(616.18 - (17.223 * currentInclination().value()));

                            lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Resistance) + " " +
                                        QString::number(x1) + " " + QString::number(y2) + " 200";
                            qDebug() << " >> " + lastCommand;
    #ifdef Q_OS_ANDROID
                            QAndroidJniObject command = QAndroidJniObject::fromString(lastCommand).object<jstring>();
                            QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote",
                                                                    "sendCommand", "(Ljava/lang/String;)V",
                                                                    command.object<jstring>());
    #elif defined(Q_OS_WIN)
                            if (logcatAdbThread)
                                logcatAdbThread->runCommand("shell " + lastCommand);
    #elif defined Q_OS_IOS
    #ifndef IO_UNDER_QT
                            h->adb_sendcommand(lastCommand.toStdString().c_str());
    #endif
    #endif
                        }
                        */
                    }
                }

                requestInclination = -100;
            }

            // In gear resistance mode, gears control resistance separately via gRPC, so don't add to message resistance
            double gearValue = nordictrackadbbike_gear_resistance_mode ? 0 : gears();
            double r = currentResistance().value() + difficult() + gearValue; // the inclination here is like the resistance for the other bikes
            QByteArray message = (QString::number(requestInclination).toLocal8Bit()) + ";" + QString::number(r).toLocal8Bit();
            requestInclination = -100;
            int ret = socket->writeDatagram(message, message.size(), sender, 8003);
            qDebug() << QString::number(ret) + " >> " + message;
        }

        if (watts())
            KCal +=
                ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        lastRefreshCharacteristicChanged = now;

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                update_hr_from_external();
            }
        }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadencep =
            settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadencep && h && firstStateChanged) {
            h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
        }
#endif
#endif

        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        emit debug(QStringLiteral("Current Gear: ") + QString::number(gear));
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
        // debug("Current Distance: " + QString::number(distance));
    }
}

void nordictrackifitadbbike::onHRM(int hrm) {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    if (
#ifdef Q_OS_ANDROID
        (!settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) &&
#endif
        heartRateBeltName.startsWith(QStringLiteral("Disabled")) && !disable_hr_frommachinery) {

        Heart = hrm;
        emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
    }
}

double nordictrackifitadbbike::bikeResistanceToPeloton(resistance_t bikeResistance) {
    for (resistance_t i = 1; i < max_resistance; i++) {
        if (pelotonToBikeResistance(i) <= bikeResistance && pelotonToBikeResistance(i + 1) > bikeResistance) {
            return i;
        }
    }
    if (bikeResistance < pelotonToBikeResistance(1))
        return 1;
    else
        return 100;
}

resistance_t nordictrackifitadbbike::pelotonToBikeResistance(int pelotonResistance) {
    QSettings settings;
    int resistanceLevel;

    if (pelotonResistance <= 5) {
        resistanceLevel = 1;
    }
    else if (pelotonResistance <= 7) {
        resistanceLevel = 2;
    }
    else if (pelotonResistance <= 9) {
        resistanceLevel = 3;
    }
    else if (pelotonResistance <= 10) {
        resistanceLevel = 4;
    }
    else if (pelotonResistance <= 15) {
        resistanceLevel = 5;
    }
    else if (pelotonResistance <= 25) {
        resistanceLevel = 6;
    }
    else if (pelotonResistance <= 30) {
        resistanceLevel = 7;
    }
    else if (pelotonResistance <= 35) {
        resistanceLevel = 8;
    }
    else if (pelotonResistance <= 40) {
        resistanceLevel = 9;
    }
    else if (pelotonResistance <= 45) {
        resistanceLevel = 10;
    }
    else if (pelotonResistance <= 50) {
        resistanceLevel = 11;
    }
    else if (pelotonResistance <= 55) {
        resistanceLevel = 12;
    }
    else if (pelotonResistance <= 60) {
        resistanceLevel = 13;
    }
    else if (pelotonResistance <= 65) {
        resistanceLevel = 14;
    }
    else if (pelotonResistance <= 70) {
        resistanceLevel = 15;
    }
    else if (pelotonResistance <= 75) {
        resistanceLevel = 16;
    }
    else if (pelotonResistance <= 80) {
        resistanceLevel = 17;
    }
    else if (pelotonResistance <= 85) {
        resistanceLevel = 18;
    }
    else if (pelotonResistance <= 95) {
        resistanceLevel = 19;
    }
    else if (pelotonResistance <= 100) {
        resistanceLevel = 20;
    }
    else {
        return Resistance.value();
    }

    return (resistanceLevel * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
           settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
}

void nordictrackifitadbbike::forceResistance(double resistance) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        setGrpcResistance(resistance);
    }
#endif
}

void nordictrackifitadbbike::update() {

    QSettings settings;
    update_metrics(false, 0);

    if (initRequest) {
        initRequest = false;
        emit connectedAndDiscovered();
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

#ifdef Q_OS_ANDROID
    // Use gRPC to fetch current metrics
    if (grpcInitialized) {
        double currentSpeed = getGrpcSpeed();
        double currentIncline = getGrpcIncline();
        double currentWatts = getGrpcWatts();
        double currentCadence = getGrpcCadence();
        double currentResistance = getGrpcResistance();
        
        // Update the metrics if they've changed
        if (currentSpeed != Speed.value()) {
            Speed = currentSpeed;
            emit debug(QString("gRPC Speed: %1").arg(currentSpeed));
        }
        
        if (currentIncline != Inclination.value()) {
            Inclination = currentIncline;
            emit debug(QString("gRPC Inclination: %1").arg(currentIncline));
        }
        
        if (currentWatts != m_watt.value()) {
            m_watt = currentWatts;
            emit debug(QString("gRPC Watts: %1").arg(currentWatts));
        }
        
        if (currentCadence != Cadence.value()) {
            Cadence = currentCadence;
            emit debug(QString("gRPC Cadence: %1").arg(currentCadence));
        }
        
        if (currentResistance != Resistance.value()) {
            Resistance = currentResistance;
            emit debug(QString("gRPC Resistance: %1").arg(currentResistance));
        }
        
        int currentFanSpeed = getGrpcFanSpeed();
        if (currentFanSpeed != FanSpeed) {
            FanSpeed = currentFanSpeed;
            emit debug(QString("gRPC Fan Speed: %1").arg(currentFanSpeed));
        }

        bool nordictrackadbbike_resistance = settings.value(QZSettings::nordictrackadbbike_resistance, QZSettings::default_nordictrackadbbike_resistance).toBool();
        auto virtualBike = this->VirtualBike();

        if (requestInclination != -100 && !nordictrackadbbike_resistance && !settings.value(QZSettings::force_resistance_instead_inclination, QZSettings::default_force_resistance_instead_inclination).toBool()) {
            QDateTime now = QDateTime::currentDateTime();
            double inclination_delay_seconds = settings.value(QZSettings::inclination_delay_seconds, QZSettings::default_inclination_delay_seconds).toDouble();
            
            // Apply delay logic similar to ADB version
            if (lastGrpcInclinationChanged.secsTo(now) > inclination_delay_seconds) {
                double inc = qRound(requestInclination / 0.5) * 0.5;
                double currentInc = qRound(currentInclination().value() / 0.5) * 0.5;
                if (inc != currentInc) {
                    emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));
                    setGrpcIncline(lastRawRequestedInclinationValue + gears());
                    lastGrpcInclinationChanged = now;
                }
            }
            requestInclination = -100;
            requestResistance = -1;
        } else if((virtualBike && virtualBike->ftmsDeviceConnected()) && lastGearValue != gears() && lastRawRequestedInclinationValue != -100 && !nordictrackadbbike_resistance) {
            // in order to send the new gear value ASAP (similar to tacxneo2)
            setGrpcIncline(lastRawRequestedInclinationValue + gears());
            requestResistance = -1;
        }
        
        if(requestResistance != - 1) {
            setGrpcResistance(requestResistance);
            requestResistance = -1;
        } else if (lastGearValue != gears()) {
            if (nordictrackadbbike_gear_resistance_mode) {
                // In gear resistance mode: gears directly set resistance value (magnet stays enabled)
                qDebug() << QStringLiteral("Gear resistance mode: Setting gRPC resistance to gear value: ") << gears();
                setGrpcResistance(gears());
            } else if (!autoResistanceEnable && nordictrackadbbike_resistance) {
                // Old resistance mode: gears change resistance incrementally (only when magnet is disabled)
                qDebug() << QStringLiteral("Setting gRPC resistance based on gear change: ") << gears() << " lastGearValue: " << lastGearValue << " Resistance: " << Resistance.value();
                setGrpcResistance(Resistance.value() + (gears() - lastGearValue));
            }
        }

        // Update lastGearValue for gear change detection
        lastGearValue = gears();
    }
#endif
}

uint16_t nordictrackifitadbbike::watts() { return m_watt.value(); }

void nordictrackifitadbbike::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

bool nordictrackifitadbbike::connected() { return true; }

resistance_t nordictrackifitadbbike::resistanceFromPowerRequest(uint16_t power) {
    // actually it's using inclination for the s22i
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    if (Cadence.value() == 0)
        return 0;

    for (resistance_t i = 0; i < max_resistance; i++) {
        if (wattsFromResistance(i, Cadence.value()) <= power && wattsFromResistance(i + 1, Cadence.value()) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i, Cadence.value())
                     << wattsFromResistance(i + 1, Cadence.value()) << power;
            return i;
        }
    }
    if (power < wattsFromResistance(0, Cadence.value()))
        return 0;
    else
        return max_resistance;
}

uint16_t nordictrackifitadbbike::wattsFromResistance(double inclination, double cadence) {
    // this is for the s22i
    double power = 0.0;

    if (inclination == 0.0) {
        power = 0.01 * cadence * cadence + -0.25 * cadence + 8.00;
    }
    else if (inclination == 0.5) {
        power = 0.01 * cadence * cadence + -0.15 * cadence + 4.00;
    }
    else if (inclination == 1.0) {
        power = 0.01 * cadence * cadence + -0.50 * cadence + 11.00;
    }
    else if (inclination == 1.5) {
        power = 0.01 * cadence * cadence + -0.50 * cadence + 11.00;
    }
    else if (inclination == 2.0) {
        power = 0.01 * cadence * cadence + -0.40 * cadence + 8.00;
    }
    else if (inclination == 2.5) {
        power = 0.01 * cadence * cadence + 0.15 * cadence + -6.00;
    }
    else if (inclination == 3.0) {
        power = 0.02 * cadence * cadence + -1.10 * cadence + 21.00;
    }
    else if (inclination == 3.5) {
        power = 0.03 * cadence * cadence + -2.10 * cadence + 47.00;
    }
    else if (inclination == 4.0) {
        power = 0.04 * cadence * cadence + -2.45 * cadence + 56.00;
    }
    else if (inclination == 4.5) {
        power = 0.03 * cadence * cadence + -1.90 * cadence + 42.00;
    }
    else if (inclination == 5.0) {
        power = 0.05 * cadence * cadence + -3.70 * cadence + 86.00;
    }
    else if (inclination == 5.5) {
        power = 0.05 * cadence * cadence + -3.80 * cadence + 92.00;
    }
    else if (inclination == 6.0) {
        power = 0.07 * cadence * cadence + -5.10 * cadence + 112.00;
    }
    else if (inclination == 6.5) {
        power = 0.06 * cadence * cadence + -4.20 * cadence + 94.00;
    }
    else if (inclination == 7.0) {
        power = 0.03 * cadence * cadence + -0.40 * cadence + -10.00;
    }
    else if (inclination == 7.5) {
        power = 0.07 * cadence * cadence + -4.60 * cadence + 100.00;
    }
    else if (inclination == 8.0) {
        power = 0.11 * cadence * cadence + -7.75 * cadence + 180.00;
    }
    else if (inclination == 8.5) {
        power = 0.09 * cadence * cadence + -5.75 * cadence + 132.00;
    }
    else if (inclination == 9.0) {
        power = 0.08 * cadence * cadence + -4.40 * cadence + 90.00;
    }
    else if (inclination == 9.5) {
        power = 0.08 * cadence * cadence + -4.60 * cadence + 102.00;
    }
    else if (inclination == 10.0) {
        power = 0.11 * cadence * cadence + -7.30 * cadence + 180.00;
    }
    else if (inclination == 10.5) {
        power = 0.08 * cadence * cadence + -4.00 * cadence + 90.00;
    }
    else if (inclination == 11.0) {
        power = 0.12 * cadence * cadence + -7.40 * cadence + 174.00;
    }
    else if (inclination == 11.5) {
        power = 0.12 * cadence * cadence + -7.40 * cadence + 174.00;
    }
    else if (inclination == 12.0) {
        power = 0.20 * cadence * cadence + -14.70 * cadence + 351.00;
    }
    else if (inclination == 12.5) {
        power = 0.20 * cadence * cadence + -14.75 * cadence + 372.00;
    }
    else if (inclination == 13.0) {
        power = 0.12 * cadence * cadence + -6.30 * cadence + 159.00;
    }
    else if (inclination == 13.5) {
        power = 0.15 * cadence * cadence + -9.00 * cadence + 219.00;
    }
    else if (inclination == 14.0) {
        power = 0.37 * cadence * cadence + -30.60 * cadence + 753.00;
    }
    else if (inclination == 14.5) {
        power = 0.14 * cadence * cadence + -7.30 * cadence + 183.00;
    }
    else if (inclination == 15.0) {
        power = 0.17 * cadence * cadence + -8.85 * cadence + 222.00;
    }
    else if (inclination == 15.5) {
        power = 0.17 * cadence * cadence + -8.85 * cadence + 222.00;
    }
    else if (inclination == 16.0) {
        power = 0.19 * cadence * cadence + -9.75 * cadence + 245.00;
    }
    else if (inclination == 16.5) {
        power = 0.26 * cadence * cadence + -17.45 * cadence + 455.00;
    }
    else if (inclination == 17.0) {
        power = 0.27 * cadence * cadence + -17.90 * cadence + 470.00;
    }
    else if (inclination == 17.5) {
        power = 0.27 * cadence * cadence + -17.90 * cadence + 470.00;
    }
    else {
        qDebug() << "Inclination level not supported";
    }

    return power;
}

bool nordictrackifitadbbike::ifitCompatible() {return true;}

// gRPC integration methods implementation
void nordictrackifitadbbike::initializeGrpcService() {
#ifdef Q_OS_ANDROID
    if (!grpcInitialized) {
        try {
            QSettings settings;
            QString ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
            
            // Set Android context first
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                "setContext",
                "(Landroid/content/Context;)V",
                QtAndroid::androidContext().object()
            );
            
            // Now initialize the service with the host IP
            QAndroidJniObject hostObj = QAndroidJniObject::fromString(ip);
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                "initialize",
                "(Ljava/lang/String;)V",
                hostObj.object<jstring>()
            );
            grpcInitialized = true;
            emit debug("gRPC service initialized successfully with host: " + ip);
        } catch (...) {
            emit debug("Failed to initialize gRPC service");
            grpcInitialized = false;
        }
    }
#endif
}

void nordictrackifitadbbike::startGrpcMetricsUpdates() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "startMetricsUpdates",
            "()V"
        );
        emit debug("Started gRPC metrics updates");
    }
#endif
}

void nordictrackifitadbbike::stopGrpcMetricsUpdates() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "stopMetricsUpdates",
            "()V"
        );
        emit debug("Stopped gRPC metrics updates");
    }
#endif
}

double nordictrackifitadbbike::getGrpcSpeed() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentSpeed",
            "()D"
        );
    }
#endif
    return 0.0;
}

double nordictrackifitadbbike::getGrpcIncline() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentIncline",
            "()D"
        );
    }
#endif
    return 0.0;
}

double nordictrackifitadbbike::getGrpcWatts() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentWatts",
            "()D"
        );
    }
#endif
    return 0.0;
}

double nordictrackifitadbbike::getGrpcCadence() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentRpm",
            "()D"
        );
    }
#endif
    return 0.0;
}

double nordictrackifitadbbike::getGrpcResistance() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentResistance",
            "()D"
        );
    }
#endif
    return 0.0;
}

void nordictrackifitadbbike::setGrpcResistance(double resistance) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "adjustResistance",
            "(D)V",
            resistance - Resistance.value()
        );
        emit debug(QString("Set gRPC resistance to: %1").arg(resistance));
    }
#endif
}

void nordictrackifitadbbike::setGrpcIncline(double incline) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "adjustIncline",
            "(D)V",
            incline - Inclination.value()
        );
        emit debug(QString("Set gRPC incline to: %1").arg(incline));
    }
#endif
}

void nordictrackifitadbbike::setGrpcWatts(double watts) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "setWatts",
            "(D)V",
            watts
        );
        emit debug(QString("Set gRPC watts to: %1").arg(watts));
        hasActiveWattsTarget = (watts > 0);
    }
#endif
}

void nordictrackifitadbbike::disableGrpcWatts() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "disableConstantWatts",
            "()V"
        );
        emit debug("Disabled gRPC constant watts mode");
        hasActiveWattsTarget = false;
    }
#endif
}

void nordictrackifitadbbike::changePower(int32_t power) {
    // Call base class implementation first to set RequestedPower
    bike::changePower(power);
    
    // If gRPC is available, use it to set the power directly
#ifdef Q_OS_ANDROID
    if (grpcInitialized && power > 0) {
        setGrpcWatts(static_cast<double>(power));
        emit debug(QString("changePower: Set power to %1W via gRPC").arg(power));
    } else {
        emit debug(QString("changePower: Power request %1W (gRPC not available or power <= 0)").arg(power));
    }
#else
    emit debug(QString("changePower: Power request %1W (Android gRPC not available)").arg(power));
#endif
}

bool nordictrackifitadbbike::changeFanSpeed(uint8_t speed) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        setGrpcFanSpeed(static_cast<int>(speed));
        FanSpeed = speed;
        emit debug(QString("changeFanSpeed: Set fan speed to %1 via gRPC").arg(speed));
        return true;
    } else {
        emit debug(QString("changeFanSpeed: Fan speed request %1 (gRPC not available)").arg(speed));
        return false;
    }
#else
    emit debug(QString("changeFanSpeed: Fan speed request %1 (Android gRPC not available)").arg(speed));
    return false;
#endif
}

void nordictrackifitadbbike::setGrpcFanSpeed(int fanSpeed) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "setFanSpeed",
            "(I)V",
            fanSpeed
        );
        emit debug(QString("Set gRPC fan speed to: %1").arg(fanSpeed));
    }
#endif
}

int nordictrackifitadbbike::getGrpcFanSpeed() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jint>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentFanSpeed",
            "()I"
        );
    }
#endif
    return 0;
}


