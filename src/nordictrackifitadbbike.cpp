#include "nordictrackifitadbbike.h"

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

using namespace std::chrono_literals;

nordictrackifitadbbikeLogcatAdbThread::nordictrackifitadbbikeLogcatAdbThread(QString s) { Q_UNUSED(s) }

void nordictrackifitadbbikeLogcatAdbThread::run() {
    QSettings settings;
    QString ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
    
#ifdef Q_OS_ANDROID
    QAndroidJniObject IP = QAndroidJniObject::fromString(ip).object<jstring>();
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "createConnection",
                                                "(Ljava/lang/String;Landroid/content/Context;)V",
                                                IP.object<jstring>(), QtAndroid::androidContext().object());
#elif defined Q_OS_WIN
    runAdbCommand("connect " + ip);
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
    h = new lockscreen();
    h->adb_connect(ip.toStdString().c_str());
#endif
#endif    

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

    return out;
#elif defined Q_OS_ANDROID
    qDebug() << "adbLogCat >> " << command;
    QAndroidJniObject c = QAndroidJniObject::fromString(command).object<jstring>();
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote",
                                            "sendCommand", "(Ljava/lang/String;)V",
                                            c.object<jstring>());
    QThread:msleep(100);
    QAndroidJniObject recv = QAndroidJniObject::callStaticObjectMethod<jstring>(
        "org/cagnulen/qdomyoszwift/QZAdbRemote", "getReceiveData");
    QString r = recv.toString();
    return r;
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
    qDebug() << "adb >> " << command;
    unsigned char* tailMemoryBuffer = nullptr;
    int size = h->adb_sendcommand(command.toStdString().c_str(), &tailMemoryBuffer);
    if(tailMemoryBuffer) {
        QString output = QString::fromUtf8((const char*)tailMemoryBuffer);
        delete tailMemoryBuffer;
        return output;
    }
    return "";
#endif
#endif    
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
    });
    QObject::connect(process, &QProcess::readyReadStandardError, [process, this]() {
        auto output = process->readAllStandardError();
        emit debug("adbLogCat ERROR << " + output);
    });
    emit debug("adbLogCat >> " + command);
    process->start("adb/adb.exe", QStringList(command.split(' ')));
    process->waitForFinished(-1);
#elif defined Q_OS_ANDROID
    qDebug() << "adbLogCat >> " << command;
    QAndroidJniObject c = QAndroidJniObject::fromString(command).object<jstring>();
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote",
                                            "sendCommand", "(Ljava/lang/String;)V",
                                            c.object<jstring>());
    QString r = "";
    do {
        QAndroidJniObject recv = QAndroidJniObject::callStaticObjectMethod<jstring>(
        "org/cagnulen/qdomyoszwift/QZAdbRemote", "getReceiveData");
        r = recv.toString();
        QString output = r;
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
    } while (r.length());
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
    qDebug() << "adbLogCat >> " << command;
    unsigned char* tailMemoryBuffer = nullptr;
    int size = h->adb_sendcommand("logcat", &tailMemoryBuffer);

    if(tailMemoryBuffer) {
        QString output = QString::fromUtf8((const char*)tailMemoryBuffer);
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
        delete tailMemoryBuffer;
    }    
#endif
#endif
}

nordictrackifitadbbike::nordictrackifitadbbike(bool noWriteResistance, bool noHeartService,
                                               uint8_t bikeResistanceOffset, double bikeResistanceGain) {
    QSettings settings;
    bool nordictrack_ifit_adb_remote =
        settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote)
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

    if (nordictrack_ifit_adb_remote) {
        logcatAdbThread = new nordictrackifitadbbikeLogcatAdbThread("logcatAdbThread");
        /*connect(logcatAdbThread, &nordictrackifitadbbikeLogcatAdbThread::onSpeedInclination, this,
                &nordictrackifitadbbike::onSpeedInclination);
        connect(logcatAdbThread, &nordictrackifitadbbikeLogcatAdbThread::onWatt, this,
                &nordictrackifitadbbike::onWatt);*/
        connect(logcatAdbThread, &nordictrackifitadbbikeLogcatAdbThread::onHRM, this, &nordictrackifitadbbike::onHRM);
        connect(logcatAdbThread, &nordictrackifitadbbikeLogcatAdbThread::debug, this, &nordictrackifitadbbike::debug);
        logcatAdbThread->start();
    }
}

bool nordictrackifitadbbike::inclinationAvailableByHardware() { 
    QSettings settings;
    bool proform_studio_NTEX71021 =
    settings.value(QZSettings::proform_studio_NTEX71021, QZSettings::default_proform_studio_NTEX71021)
        .toBool();
    if(proform_studio_NTEX71021)
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
    while (socket->hasPendingDatagrams()) {
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
                    Resistance = gear;
                    gearsAvailable = true;
                }
            } else if (line.contains(QStringLiteral("Changed Resistance"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    resistance = getDouble(aValues.last());
                    m_pelotonResistance = (100 / 32) * resistance;
                    qDebug() << QStringLiteral("Current Peloton Resistance: ") << m_pelotonResistance.value()
                             << resistance;
                    if(!gearsAvailable)
                        Resistance = resistance;
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
                fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        }

        bool proform_studio_NTEX71021 =
            settings.value(QZSettings::proform_studio_NTEX71021, QZSettings::default_proform_studio_NTEX71021).toBool();
        bool nordictrack_ifit_adb_remote =
            settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote)
                .toBool();

        // only resistance
        if(proform_studio_NTEX71021) {
            if (nordictrack_ifit_adb_remote) {
                if (requestResistance != -100) {
                    if (requestResistance != currentResistance().value()) {
                        int x1 = 950;
                        int y2 = (int)(493 - (13.57 * (requestResistance - 1)));
                        int y1Resistance = (int)(493 - (13.57 * currentResistance().value()));

                        lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Resistance) + " " +
                                      QString::number(x1) + " " + QString::number(y2) + " 200";
                        qDebug() << " >> " + lastCommand;

                        if (logcatAdbThread)
                            logcatAdbThread->runCommand("shell " + lastCommand);
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
        else if (lastInclinationChanged.secsTo(QDateTime::currentDateTime()) > 4) {
            lastInclinationChanged = QDateTime::currentDateTime();
            if (nordictrack_ifit_adb_remote) {
                if (requestInclination != -100) {
                    double inc = qRound(requestInclination / 0.5) * 0.5;
                    if (inc != currentInclination().value()) {
                        bool proform_studio = settings.value(QZSettings::proform_studio, QZSettings::default_proform_studio).toBool();
                        int x1 = 75;
                        int y2 = (int)(616.18 - (17.223 * (inc + gears())));
                        int y1Resistance = (int)(616.18 - (17.223 * currentInclination().value()));

                        if(proform_studio) {
                            int x1 = 1827;
                            int y2 = (int)(806 - (21.375 * (inc + gears())));
                            int y1Resistance = (int)(806 - (21.375 * currentInclination().value()));
                        }

                        lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Resistance) + " " +
                                      QString::number(x1) + " " + QString::number(y2) + " 200";
                        qDebug() << " >> " + lastCommand;

                        if (logcatAdbThread)
                            logcatAdbThread->runCommand("shell " + lastCommand);
                    }
                }

                requestInclination = -100;
            }

            QByteArray message = (QString::number(requestInclination).toLocal8Bit()) + ";";
            requestInclination = -100;
            int ret = socket->writeDatagram(message, message.size(), sender, 8003);
            qDebug() << QString::number(ret) + " >> " + message;
        }

        if (watts())
            KCal +=
                ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

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

resistance_t nordictrackifitadbbike::pelotonToBikeResistance(int pelotonResistance) {
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

void nordictrackifitadbbike::forceResistance(double resistance) {}

void nordictrackifitadbbike::update() {

    QSettings settings;
    update_metrics(false, 0);

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

uint16_t nordictrackifitadbbike::watts() { return m_watt.value(); }

void nordictrackifitadbbike::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

bool nordictrackifitadbbike::connected() { return true; }
