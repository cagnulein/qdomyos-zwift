#include "nordictrackifitadbtreadmill.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualbike.h"
#include "virtualtreadmill.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QProcess>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

nordictrackifitadbtreadmillLogcatAdbThread::nordictrackifitadbtreadmillLogcatAdbThread(QString s) { Q_UNUSED(s) }

void nordictrackifitadbtreadmillLogcatAdbThread::run() {
    QSettings settings;
    QString ip = settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();
    runAdbCommand("connect " + ip);

    while (1) {
        runAdbTailCommand("logcat");
    }
}

QString nordictrackifitadbtreadmillLogcatAdbThread::runAdbCommand(QString command) {
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

void nordictrackifitadbtreadmillLogcatAdbThread::runAdbTailCommand(QString command) {
#ifdef Q_OS_WINDOWS
    auto process = new QProcess;
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process, this]() {
        QString output = process->readAllStandardOutput();
        qDebug() << "adbLogCat STDOUT << " << output;
        QStringList lines = output.split('\n', Qt::SplitBehaviorFlags::SkipEmptyParts);
        bool wattFound = false;
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
            }
        }
        emit onSpeedInclination(speed, inclination);
        if (wattFound)
            emit onWatt(watt);
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

double nordictrackifitadbtreadmill::getDouble(QString v) {
    QChar d = QLocale().decimalPoint();
    if (d == ',') {
        v = v.replace('.', ',');
    }
    return QLocale().toDouble(v);
}

nordictrackifitadbtreadmill::nordictrackifitadbtreadmill(bool noWriteResistance, bool noHeartService) {
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
    connect(refresh, &QTimer::timeout, this, &nordictrackifitadbtreadmill::update);
    QString ip = settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();

    refresh->start(200ms);
#ifdef Q_OS_WIN32
    if (!nordictrack_ifit_adb_remote)
#endif
    {
        socket = new QUdpSocket(this);
        bool result = socket->bind(QHostAddress::AnyIPv4, 8002);
        qDebug() << result;
        processPendingDatagrams();
        connect(socket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    }
#ifdef Q_OS_WIN32
    else {
        logcatAdbThread = new nordictrackifitadbtreadmillLogcatAdbThread("logcatAdbThread");
        connect(logcatAdbThread, &nordictrackifitadbtreadmillLogcatAdbThread::onSpeedInclination, this,
                &nordictrackifitadbtreadmill::onSpeedInclination);
        connect(logcatAdbThread, &nordictrackifitadbtreadmillLogcatAdbThread::onWatt, this,
                &nordictrackifitadbtreadmill::onWatt);
        connect(logcatAdbThread, &nordictrackifitadbtreadmillLogcatAdbThread::debug, this,
                &nordictrackifitadbtreadmill::debug);
        logcatAdbThread->start();
    }
#endif

    if (nordictrack_ifit_adb_remote) {
#ifdef Q_OS_ANDROID
        QAndroidJniObject IP = QAndroidJniObject::fromString(ip).object<jstring>();
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "createConnection",
                                                  "(Ljava/lang/String;Landroid/content/Context;)V",
                                                  IP.object<jstring>(), QtAndroid::androidContext().object());
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
        h->adb_connect(ip.toStdString().c_str());
#endif
#endif
    }

    initRequest = true;

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()) {
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        bool virtual_device_force_bike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                .toBool();
        if (virtual_device_enabled) {
            if (!virtual_device_force_bike) {
                debug("creating virtual treadmill interface...");
                auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &nordictrackifitadbtreadmill::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &nordictrackifitadbtreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                debug("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &nordictrackifitadbtreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
            firstStateChanged = 1;
        }
    }
    // ********************************************************************************************************
}

void nordictrackifitadbtreadmill::processPendingDatagrams() {
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

        QString ip =
            settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();
        QString heartRateBeltName =
            settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
        double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
        bool disable_hr_frommachinery =
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

        double speed = 0;
        double incline = 0;
        bool hrmFound = false;
        QStringList lines = QString::fromLocal8Bit(datagram.data()).split("\n");
        foreach (QString line, lines) {
            qDebug() << line;
            if (line.contains(QStringLiteral("Changed KPH"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    speed = getDouble(aValues.last());
                    Speed = speed;
                }
            } else if (line.contains(QStringLiteral("Changed Grade"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    incline = getDouble(aValues.last());
                    Inclination = incline;
                }
            } else if (line.contains("HeartRateDataUpdate") && 
#ifdef Q_OS_ANDROID
                        (!settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) &&
#endif
                        heartRateBeltName.startsWith(QStringLiteral("Disabled")) && !disable_hr_frommachinery
            ) {                
                QStringList splitted = line.split(' ', Qt::SkipEmptyParts);
                if (splitted.length() > 14) {
                    Heart = splitted[14].toInt();
                    hrmFound = true;
                }
            }
        }

        bool nordictrack_ifit_adb_remote =
            settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote)
                .toBool();
        if (nordictrack_ifit_adb_remote) {
            bool nordictrack_x22i =
                settings.value(QZSettings::nordictrack_x22i, QZSettings::default_nordictrack_x22i).toBool();
            if (requestSpeed != -1) {
                int x1 = 1845;
                int y1Speed = 807 - (int)((Speed.value() - 1) * 29.78);
                // set speed slider to target position
                int y2 = y1Speed - (int)((requestSpeed - Speed.value()) * 29.78);
                if(nordictrack_x22i) {
                    x1 = 1845;
                    y1Speed = (int) (785 - (23.636 * (Speed.value() - 1)));
                    y2 = y1Speed - (int)((requestSpeed - Speed.value()) * 23.636);
                }

                lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Speed) + " " +
                              QString::number(x1) + " " + QString::number(y2) + " 200";
                qDebug() << " >> " + lastCommand;
#ifdef Q_OS_ANDROID
                QAndroidJniObject command = QAndroidJniObject::fromString(lastCommand).object<jstring>();
                QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "sendCommand",
                                                          "(Ljava/lang/String;)V", command.object<jstring>());
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
                h->adb_sendcommand(lastCommand.toStdString().c_str());
#endif
#endif
                requestSpeed = -1;
            } else if (requestInclination != -100) {
                double inc = qRound(requestInclination / 0.5) * 0.5;
                if(inc != currentInclination().value()) {
                    requestInclination = inc;
                    int x1 = 75;
                    int y1Inclination = 807 - (int)((currentInclination().value() + 3) * 29.9);
                    // set speed slider to target position
                    int y2 = y1Inclination - (int)((requestInclination - currentInclination().value()) * 29.9);

                    if(nordictrack_x22i) {
                        x1 = 75;
                        y1Inclination = (int) (785 - (11.304 * (currentInclination().value() + 6)));
                        y2 = y1Inclination - (int)((requestInclination - currentInclination().value()) * 11.304);
                    }

                    lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Inclination) + " " +
                                QString::number(x1) + " " + QString::number(y2) + " 200";
                    qDebug() << " >> " + lastCommand;
#ifdef Q_OS_ANDROID
                    QAndroidJniObject command = QAndroidJniObject::fromString(lastCommand).object<jstring>();
                    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "sendCommand",
                                                            "(Ljava/lang/String;)V", command.object<jstring>());
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
                    h->adb_sendcommand(lastCommand.toStdString().c_str());
#endif
#endif
                }
                requestInclination = -100;
            }
        }

        QByteArray message = (QString::number(requestSpeed) + ";" + QString::number(requestInclination)).toLocal8Bit();
        // we have to separate the 2 commands
        if (requestSpeed == -1)
            requestInclination = -100;
        requestSpeed = -1;
        int ret = socket->writeDatagram(message, message.size(), sender, 8003);
        qDebug() << QString::number(ret) + " >> " + message;

        if (watts(weight))
            KCal +=
                ((((0.048 * ((double)watts(weight)) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

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

        cadenceFromAppleWatch();

        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
        // debug("Current Distance: " + QString::number(distance));
        emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts(weight)));
    }
}

/*
void nordictrackifitadbtreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool
disable_log, bool wait_for_response) { QEventLoop loop; QTimer timeout; if (wait_for_response) {
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
}
*/
void nordictrackifitadbtreadmill::forceIncline(double incline) {}

void nordictrackifitadbtreadmill::forceSpeed(double speed) {}

void nordictrackifitadbtreadmill::onWatt(double watt) {
    m_watt = watt;
    wattReadFromTM = true;
}

void nordictrackifitadbtreadmill::onSpeedInclination(double speed, double inclination) {

    Speed = speed;
    Inclination = inclination;

    QSettings settings;
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    if (watts(weight))
        KCal += ((((0.048 * ((double)watts(weight)) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

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

    emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    // debug("Current Distance: " + QString::number(distance));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts(weight)));
}

void nordictrackifitadbtreadmill::update() {

    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();

    update_metrics(!wattReadFromTM, watts(weight));

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
        emit tapeStarted();
    }
    if (requestStop != -1) {
        emit debug(QStringLiteral("stopping..."));
        // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
        requestStop = -1;
    }
}

void nordictrackifitadbtreadmill::changeInclinationRequested(double grade, double percentage) {
    // these treadmills support negative inclination
    /*if (percentage < 0)
        percentage = 0;*/
    changeInclination(grade, percentage);
}

bool nordictrackifitadbtreadmill::connected() { return true; }
