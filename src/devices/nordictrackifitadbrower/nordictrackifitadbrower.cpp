#include "nordictrackifitadbrower.h"
#include "qzsettings.h"
#include "virtualdevices/virtualrower.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QJniObject>
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

nordictrackifitadbrowerLogcatAdbThread::nordictrackifitadbrowerLogcatAdbThread(QString s) { Q_UNUSED(s) }

void nordictrackifitadbrowerLogcatAdbThread::run() {
    QSettings settings;
    QString ip = settings.value(QZSettings::proform_rower_ip, QZSettings::default_proform_rower_ip).toString();
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

QString nordictrackifitadbrowerLogcatAdbThread::runAdbCommand(QString command) {
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

bool nordictrackifitadbrowerLogcatAdbThread::runCommand(QString command) {
    if(adbCommandPending.length() == 0) {
        adbCommandPending = command;
        return true;
    }
    return false;
}

void nordictrackifitadbrowerLogcatAdbThread::runAdbTailCommand(QString command) {
#ifdef Q_OS_WINDOWS
    auto process = new QProcess;
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process, this]() {
        QString output = process->readAllStandardOutput();
        // qDebug() << "adbLogCat STDOUT << " << output;
        QStringList lines = output.split('\n', Qt::SplitBehaviorFlags::SkipEmptyParts);
        bool wattFound = false;
        bool hrmFound = false;
        bool cadenceFound = false;
        bool resistanceFound = false;
        foreach (QString line, lines) {
            if (line.contains("Changed KPH") || line.contains("Changed Actual KPH")) {
                emit debug(line);                
                speed = line.split(' ').last().toDouble();
            } else if (line.contains("Changed Resistance")) {
                emit debug(line);
                resistance = line.split(' ').last().toDouble();
                resistanceFound = true;
            } else if (line.contains("Changed RPM")) {
                emit debug(line);
                cadence = line.split(' ').last().toDouble();
                cadenceFound = true;
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
        emit onSpeedResistance(speed, resistance);
        if (cadenceFound)
            emit onCadence(cadence);
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

nordictrackifitadbrower::nordictrackifitadbrower(bool noWriteResistance, bool noHeartService,
                                               int8_t bikeResistanceOffset, double bikeResistanceGain) {
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
    connect(refresh, &QTimer::timeout, this, &nordictrackifitadbrower::update);
    ip = settings.value(QZSettings::proform_rower_ip, QZSettings::default_proform_rower_ip).toString();
    refresh->start(200ms);

    socket = new QUdpSocket(this);
    bool result = socket->bind(QHostAddress::AnyIPv4, 8002);
    qDebug() << result;
    processPendingDatagrams();
    connect(socket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    initRequest = true;

    // ******************************************* virtual device init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()) {
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        bool virtual_device_rower =
            settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
        bool virtual_device_force_bike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence =
            settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence && !virtual_device_rower) {
            qDebug() << "ios_peloton_workaround activated!";
            h = new lockscreen();
            h->virtualbike_ios();
        } else
#endif
#endif
            if (virtual_device_enabled) {
            if (virtual_device_rower) {
                qDebug() << QStringLiteral("creating virtual rower interface...");
                auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                // connect(virtualRower,&virtualrower::debug ,this,&nordictrackifitadbrower::debug);
                this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else if (virtual_device_force_bike) {
                qDebug() << QStringLiteral("creating virtual bike interface...");
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&nordictrackifitadbrower::debug);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                qDebug() << QStringLiteral("creating virtual rower interface...");
                auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                // connect(virtualRower,&virtualrower::debug ,this,&nordictrackifitadbrower::debug);
                this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************

    if (nordictrack_ifit_adb_remote) {
#ifdef Q_OS_ANDROID
        QJniObject IP = QJniObject::fromString(ip).object<jstring>();
        QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "createConnection",
                                                  "(Ljava/lang/String;Landroid/content/Context;)V",
                                                  IP.object<jstring>(), QtAndroid::androidContext().object());
#elif defined Q_OS_WIN
        logcatAdbThread = new nordictrackifitadbrowerLogcatAdbThread("logcatAdbThread");
        connect(logcatAdbThread, &nordictrackifitadbrowerLogcatAdbThread::onCadence, this,
                &nordictrackifitadbrower::onCadence);
        connect(logcatAdbThread, &nordictrackifitadbrowerLogcatAdbThread::onSpeedResistance, this,
                &nordictrackifitadbrower::onSpeedResistance);
        connect(logcatAdbThread, &nordictrackifitadbrowerLogcatAdbThread::onWatt, this,
                &nordictrackifitadbrower::onWatt);
        connect(logcatAdbThread, &nordictrackifitadbrowerLogcatAdbThread::onHRM, this, &nordictrackifitadbrower::onHRM);
        connect(logcatAdbThread, &nordictrackifitadbrowerLogcatAdbThread::debug, this, &nordictrackifitadbrower::debug);
        logcatAdbThread->start();
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
        h->adb_connect(ip.toStdString().c_str());
#endif
#endif
    }
}

void nordictrackifitadbrower::onSpeedResistance(double speed, double resistance) {
    if(speed > 0)
        speedReadFromTM = true;
    Speed = speed;
    Resistance = resistance;
    resistanceReadFromTM = true;
}

void nordictrackifitadbrower::onWatt(double watt) {
    m_watt = watt;
    wattReadFromTM = true;
}

void nordictrackifitadbrower::onCadence(double cadence) {
    Cadence = cadence;
    cadenceReadFromTM = true;
}

double nordictrackifitadbrower::getDouble(QString v) {
    QChar d = QLocale().decimalPoint();
    if (d == ',') {
        v = v.replace('.', ',');
    }
    return QLocale().toDouble(v);
}

void nordictrackifitadbrower::processPendingDatagrams() {
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

        QString ip = settings.value(QZSettings::proform_rower_ip, QZSettings::default_proform_rower_ip).toString();
        QString heartRateBeltName =
            settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
        double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();

        double speed = 0;
        double cadence = 0;
        double resistance = 0;
        double gear = 0;
        double watt = 0;
        QStringList lines = QString::fromLocal8Bit(datagram.data()).split("\n");
        foreach (QString line, lines) {
            qDebug() << line;

            if (line.contains(QStringLiteral("Changed KPH")) && !settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    speedReadFromTM = true;
                    speed = getDouble(aValues.last());
                    Speed = speed;
                }
            } else if (line.contains(QStringLiteral("Changed RPM"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    cadence = getDouble(aValues.last());
                    Cadence = cadence;
                    cadenceReadFromTM = true;
                    if(!speedReadFromTM) {
                        Speed = Cadence.value() *
                                settings.value(QZSettings::cadence_sensor_speed_ratio, QZSettings::default_cadence_sensor_speed_ratio)
                                    .toDouble();
                    }
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
                    m_pelotonResistance = (100 / 32) * resistance; // adjusted for rower resistance range
                    qDebug() << QStringLiteral("Current Peloton Resistance: ") << m_pelotonResistance.value()
                             << resistance;
                    if(!gearsAvailable) {
                        Resistance = resistance;
                        resistanceReadFromTM = true;
                    }
                }
            } else if (line.contains(QStringLiteral("Changed Watts"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    watt = getDouble(aValues.last());
                    m_watt = watt;
                    wattReadFromTM = true;
                }
            }
        }

        if (settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = metric::calculateSpeedFromPower(
                watts(), 0, Speed.value(), // no inclination for rower
                fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), 20);
        }

        bool nordictrack_ifit_adb_remote =
            settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote)
                .toBool();

        // resistance
        if (nordictrack_ifit_adb_remote) {
            if (requestResistance != -1) {
                if (requestResistance != currentResistance().value()) {
                    int x1 = 1205; // Estimated x-coordinate of the resistance slider (right side)
                    int y2 = (int)(590 - (15.65 * requestResistance));
                    int y1Resistance = (int)(590 - (15.65 * currentResistance().value()));

                    lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Resistance) + " " +
                                  QString::number(x1) + " " + QString::number(y2) + " 200";
                    qDebug() << " >> " + lastCommand;
#ifdef Q_OS_ANDROID
                    QJniObject command = QJniObject::fromString(lastCommand).object<jstring>();
                    QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote",
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
        } else {
            QByteArray message = (QString::number(requestResistance).toLocal8Bit()) + ";";
            requestResistance = -1;
            int ret = socket->writeDatagram(message, message.size(), sender, 8003);
            qDebug() << QString::number(ret) + " >> " + message;
        }

        if (watts())
            KCal +=
                ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
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

        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        emit debug(QStringLiteral("Current Gear: ") + QString::number(gear));
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    }
}

void nordictrackifitadbrower::onHRM(int hrm) {
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

void nordictrackifitadbrower::forceResistance(double resistance) {}

void nordictrackifitadbrower::update() {

    QSettings settings;
    update_metrics(false, 0);

    if (initRequest) {
        initRequest = false;
        emit connectedAndDiscovered();
    }    

    // updating the rower console every second
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

uint16_t nordictrackifitadbrower::watts() { 
    // If we have watts from the machine, use them
    if (wattReadFromTM && m_watt.value() > 0) {
        return m_watt.value();
    }
    
    // Otherwise calculate watts from resistance and cadence
    return wattsFromResistance(currentResistance().value(), currentCadence().value());
}

void nordictrackifitadbrower::changeResistanceRequested(double resistance) {
    if (resistance < 0)
        resistance = 0;
    changeResistance(resistance);
}

bool nordictrackifitadbrower::connected() { return true; }

uint16_t nordictrackifitadbrower::wattsFromResistance(double resistance, double cadence) {
    // Rower power estimation based on resistance and cadence
    // This formula is based on general rowing power curves
    // Power increases with both resistance and cadence (stroke rate)
    
    if (cadence <= 0) {
        return 0;
    }
    
    // Basic power formula for rowing: power increases exponentially with stroke rate
    // and linearly with resistance level
    double basePower = resistance * 8; // Base power per resistance level
    double cadenceFactor = 1.0 + (cadence - 20.0) * 0.05; // Cadence multiplier
    
    // Additional exponential component for higher stroke rates (similar to real rowing)
    double exponentialFactor = exp(cadence * 0.015);
    
    double power = basePower * cadenceFactor * (exponentialFactor / 10.0);
    
    // Ensure minimum and maximum bounds
    if (power < 10) power = 10;
    if (power > 500) power = 500; // Reasonable max for most users
    
    return (uint16_t)power;
}
