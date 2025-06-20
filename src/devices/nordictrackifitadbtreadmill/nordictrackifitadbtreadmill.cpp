#include "nordictrackifitadbtreadmill.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QProcess>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>
#include <QRegularExpression>

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

using namespace std::chrono_literals;

nordictrackifitadbtreadmillLogcatAdbThread::nordictrackifitadbtreadmillLogcatAdbThread(QString s) { Q_UNUSED(s) }

void nordictrackifitadbtreadmillLogcatAdbThread::run() {
    QSettings settings;
    QString ip = settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();
    runAdbCommand("connect " + ip);

    while (!stop) 
    {
        runAdbTailCommand("logcat");
#ifndef Q_OS_WINDOWS        
        if(adbCommandPending.length() != 0) {
            runAdbCommand(adbCommandPending);
            adbCommandPending = "";
        }
        msleep(100);        
#endif        
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

bool nordictrackifitadbtreadmillLogcatAdbThread::runCommand(QString command) {
    if(adbCommandPending.length() == 0) {
        adbCommandPending = command;
        return true;
    }
    return false;
}

void nordictrackifitadbtreadmillLogcatAdbThread::runAdbTailCommand(QString command) {
#ifdef Q_OS_WINDOWS
    auto process = new QProcess;
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process, this]() {
        if(stop) {
            process->close();
            return;
        }
        QString output = process->readAllStandardOutput();
        qDebug() << "adbLogCat STDOUT << " << output;
        QStringList lines = output.split('\n', Qt::SplitBehaviorFlags::SkipEmptyParts);
        bool wattFound = false;
        bool cadenceFound = false;
        foreach (QString line, lines) {
            if (line.contains("Changed KPH") || line.contains("Changed Actual KPH")) {
                emit debug(line);
                speed = line.split(' ').last().toDouble();
            } else if (line.contains("Changed Grade")) {
                emit debug(line);
                inclination = line.split(' ').last().toDouble();
            } else if (line.contains("Changed RPM")) {
                emit debug(line);
                cadence = line.split(' ').last().toDouble();
                cadenceFound = true;
            } else if (line.contains("Changed Watts")) {
                emit debug(line);
                watt = line.split(' ').last().toDouble();
                wattFound = true;
            }
        }
        emit onSpeedInclination(speed, inclination);
        if (wattFound)
            emit onWatt(watt);
        if (cadenceFound)
            emit onCadence(cadence);
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

nordictrackifitadbtreadmill::DisplayValue nordictrackifitadbtreadmill::extractValue(const QString& ocrText, int imageWidth, bool isLeftSide) {
    QStringList lines = ocrText.split("§§");
    QRegularExpression rectRegex("Rect\\((\\d+), (\\d+) - (\\d+), (\\d+)\\)");
    QRegularExpression numericRegex("^-?\\d+(\\.\\d+)?$");

    DisplayValue result;
    int minX = isLeftSide ? 0 : imageWidth - 200;
    int maxX = isLeftSide ? 200 : imageWidth;
    QStringList targetLabels = isLeftSide ? QStringList{"INCLINE"} : QStringList{"SPEED", "RESISTANCE", "MPH", "KPH"};

    QRect labelRect;
    int closestDistance = INT_MAX;

    // First pass: find the label
    for (const QString& line : lines) {
        QStringList parts = line.split("$$");
        if (parts.size() == 2) {
            QString value = parts[0];
            QRegularExpressionMatch match = rectRegex.match(parts[1]);

            if (match.hasMatch()) {
                int x1 = match.captured(1).toInt();
                int x2 = match.captured(3).toInt();

                if (x1 >= minX && x2 <= maxX) {
                    for (const QString& targetLabel : targetLabels) {
                        if (value.contains(targetLabel, Qt::CaseInsensitive)) {
                            labelRect = QRect(x1, match.captured(2).toInt(),
                                              x2 - x1, match.captured(4).toInt() - match.captured(2).toInt());
                            result.label = value;
                            break;
                        }
                    }
                    if (!result.label.isEmpty()) break;
                }
            }
        }
    }

    // Second pass: find the closest numeric value to the label
    if (!labelRect.isNull()) {
        for (const QString& line : lines) {
            QStringList parts = line.split("$$");
            if (parts.size() == 2) {
                QString value = parts[0];
                QRegularExpressionMatch match = rectRegex.match(parts[1]);

                if (match.hasMatch() && numericRegex.match(value).hasMatch()) {
                    int x1 = match.captured(1).toInt();
                    int y1 = match.captured(2).toInt();
                    int x2 = match.captured(3).toInt();
                    int y2 = match.captured(4).toInt();

                    QRect valueRect(x1, y1, x2 - x1, y2 - y1);

                    if (x1 >= minX && x2 <= maxX) {
                        int distance = qAbs(valueRect.center().y() - labelRect.center().y());
                        if (distance < closestDistance) {
                            closestDistance = distance;
                            result.value = value;
                            result.rect = valueRect;
                        }
                    }
                }
            }
        }
    }

    return result;
}

void nordictrackifitadbtreadmill::processOCROutput(const QString& ocrText, int imageWidth) {
    DisplayValue leftValue = extractValue(ocrText, imageWidth, true);
    DisplayValue rightValue = extractValue(ocrText, imageWidth, false);

    if (!leftValue.value.isEmpty()) {
        qDebug() << "Left value (" << leftValue.label << "):" << leftValue.value;
        Inclination = leftValue.label.toDouble();
    } else {
        qDebug() << "Left value not found";
    }

    if (!rightValue.value.isEmpty()) {
        qDebug() << "Right value (" << rightValue.label << "):" << rightValue.value;
        Speed = rightValue.label.toDouble();
    } else {
        qDebug() << "Right value not found";
    }
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
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &nordictrackifitadbtreadmill::stopLogcatAdbThread);
    QString ip = settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();

    refresh->start(200ms);
    {
        socket = new QUdpSocket(this);
        bool result = socket->bind(QHostAddress::AnyIPv4, 8002);
        qDebug() << result;
        processPendingDatagrams();
        connect(socket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    }
#ifdef Q_OS_WIN32
    if (nordictrack_ifit_adb_remote)
    {
        logcatAdbThread = new nordictrackifitadbtreadmillLogcatAdbThread("logcatAdbThread");
        connect(logcatAdbThread, &nordictrackifitadbtreadmillLogcatAdbThread::onSpeedInclination, this,
                &nordictrackifitadbtreadmill::onSpeedInclination);
        connect(logcatAdbThread, &nordictrackifitadbtreadmillLogcatAdbThread::onWatt, this,
                &nordictrackifitadbtreadmill::onWatt);
        connect(logcatAdbThread, &nordictrackifitadbtreadmillLogcatAdbThread::onCadence, this,
                &nordictrackifitadbtreadmill::onCadence);
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

    // Initialize gRPC service on Android
#ifdef Q_OS_ANDROID
    initializeGrpcService();
    if (grpcInitialized) {
        startGrpcMetricsUpdates();
    }
#endif

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
                    QString numberStr = aValues.last();
                    // Regular expression to match both decimal numbers (X.X or XX.X) and integers
                    QRegularExpression regex(QStringLiteral("\\d+(\\.\\d+)?"));
                    if (regex.match(numberStr).hasMatch()) {
                        speed = getDouble(numberStr);
                        parseSpeed(speed);
                    }
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
                qDebug() << splitted;
                if (splitted.length() > 14) {
                    int heart = splitted[14].toInt();
                    if(heart == 0) {
                        heart = splitted[10].toInt();
                    }
                    if(heart > 0) {
                        Heart = heart;
                        hrmFound = true;
                    }
                }
            }
        }

        double inc = qRound(requestInclination / 0.5) * 0.5;
        if(inc == currentInclination().value()) {
            qDebug() << "ignoring inclination" << requestInclination;
            requestInclination = -100;
        }

        double currentRequestInclination = requestInclination;

        // since the motor of the treadmill is slow, let's filter the inclination changes to more than 1 second
        if (requestInclination != -100 && lastInclinationChanged.secsTo(QDateTime::currentDateTime()) > 1) {
            lastInclinationChanged = QDateTime::currentDateTime();
        } else {
            currentRequestInclination = -100;
        }

        bool nordictrack_ifit_adb_remote =
            settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote)
                .toBool();
        if (nordictrack_ifit_adb_remote) {
            bool nordictrack_x22i =
                settings.value(QZSettings::nordictrack_x22i, QZSettings::default_nordictrack_x22i).toBool();
            bool nordictrack_treadmill_t8_5s = settings.value(QZSettings::nordictrack_treadmill_t8_5s, QZSettings::default_nordictrack_treadmill_t8_5s).toBool();
            bool nordictrack_treadmill_x14i = settings.value(QZSettings::nordictrack_treadmill_x14i, QZSettings::nordictrack_treadmill_x14i).toBool();
            bool proform_treadmill_carbon_t7 = settings.value(QZSettings::proform_treadmill_carbon_t7, QZSettings::default_proform_treadmill_carbon_t7).toBool();
            bool nordictrack_treadmill_1750_adb = settings.value(QZSettings::nordictrack_treadmill_1750_adb, QZSettings::default_nordictrack_treadmill_1750_adb).toBool();

            if (requestSpeed != -1) {
                int x1 = 1845;
                int y1Speed = 807 - (int)((Speed.value() - 1) * 31);
                // set speed slider to target position
                int y2 = y1Speed - (int)((requestSpeed - Speed.value()) * 31);
                if(nordictrack_x22i) {
                    x1 = 1845;
                    y1Speed = (int) (785 - (23.636 * (Speed.value() - 1)));
                    y2 = y1Speed - (int)((requestSpeed - Speed.value()) * 23.636);
                } else if(nordictrack_treadmill_t8_5s) {
                    x1 = 1206;
                    y1Speed = (int) (620 - (35.9 * ((Speed.value() * 0.621371) - 1)));
                    y2 = y1Speed - (int)(((requestSpeed - Speed.value()) * 0.621371) * 35.9);
                } else if(proform_treadmill_carbon_t7) {
                    x1 = 940;
                    // 458 0 183 10 mph
                    y1Speed = (int) (458 - (27.5 * ((Speed.value() * 0.621371) - 1)));
                    y2 = y1Speed - (int)(((requestSpeed - Speed.value()) * 0.621371) * 27.5);
                } else if(nordictrack_treadmill_1750_adb) {
                    x1 = 1206;
                    y1Speed = (int) (603 - (34.0 * ((Speed.value() * 0.621371) - 0.5)));
                    y2 = 603 - (int)(((requestSpeed * 0.621371) - 0.5) * 34.0);
                }

                lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Speed) + " " +
                              QString::number(x1) + " " + QString::number(y2) + " 200";
                qDebug() << " >> " + lastCommand;
#ifdef Q_OS_ANDROID
                QAndroidJniObject command = QAndroidJniObject::fromString(lastCommand).object<jstring>();
                QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "sendCommand",
                                                          "(Ljava/lang/String;)V", command.object<jstring>());
#elif defined(Q_OS_WIN)
                        if (logcatAdbThread)
                            logcatAdbThread->runCommand("shell " + lastCommand);                                                          
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
                h->adb_sendcommand(lastCommand.toStdString().c_str());
#endif
#endif
                requestSpeed = -1;
            } else if (currentRequestInclination != -100) {
                requestInclination = inc;
                int x1 = 75;
                int y1Inclination = 807 - (int)((currentInclination().value() + 3) * 31.1);
                // set speed slider to target position
                int y2 = y1Inclination - (int)((requestInclination - currentInclination().value()) * 31.1);

                if(nordictrack_x22i) {
                    x1 = 75;
                    y1Inclination = (int) (785 - (11.304 * (currentInclination().value() + 6)));
                    y2 = y1Inclination - (int)((requestInclination - currentInclination().value()) * 11.304);
                } else if(nordictrack_treadmill_t8_5s) {
                    x1 = 78;
                    y1Inclination = (int) (620 - (32.916 * (currentInclination().value())));
                    y2 = y1Inclination - (int)((requestInclination - currentInclination().value()) * 32.916);
                } else if (nordictrack_treadmill_x14i) {
                    x1 = 75;
                    y1Inclination = x14i_inclination_lookuptable(currentInclination().value());
                    y2 = x14i_inclination_lookuptable(requestInclination);
                } else if(proform_treadmill_carbon_t7) {
                    x1 = 75;
                    // 458 0 183 10%
                    y1Inclination = (int) (458 - (27.5 * (currentInclination().value())));
                    y2 = y1Inclination - (int)((requestInclination - currentInclination().value()) * 27.5);
                } else if(nordictrack_treadmill_1750_adb) {
                    x1 = 75;
                    y1Inclination = (int) (603 - (21.72222222 * (currentInclination().value() + 3.0)));
                    y2 = 603 - (int)((requestInclination + 3.0) * 21.72222222);
                }

                lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Inclination) + " " +
                            QString::number(x1) + " " + QString::number(y2) + " 200";
                qDebug() << " >> " + lastCommand;
#ifdef Q_OS_ANDROID
                QAndroidJniObject command = QAndroidJniObject::fromString(lastCommand).object<jstring>();
                QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "sendCommand",
                                                        "(Ljava/lang/String;)V", command.object<jstring>());
#elif defined(Q_OS_WIN)
                        if (logcatAdbThread)
                            logcatAdbThread->runCommand("shell " + lastCommand);                                                        
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
                h->adb_sendcommand(lastCommand.toStdString().c_str());
#endif
#endif
                requestInclination = -100;
            }
        }

        // sending only if there is a real command in order to don't send too much commands when on the companion there is the debug log enabled.
        if(!nordictrack_ifit_adb_remote && (requestSpeed != -1 || currentRequestInclination != -100)) {
            QByteArray message = (QString::number(requestSpeed) + ";" + QString::number(currentRequestInclination)).toLocal8Bit();
            // we have to separate the 2 commands
            if (requestSpeed == -1)
                requestInclination = -100;
            requestSpeed = -1;
            int ret = socket->writeDatagram(message, message.size(), sender, 8003);
            qDebug() << QString::number(ret) + " >> " + message;
        }

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
void nordictrackifitadbtreadmill::forceIncline(double incline) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        setGrpcIncline(incline);
    }
#endif
}

void nordictrackifitadbtreadmill::forceSpeed(double speed) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        setGrpcSpeed(speed);
    }
#endif
}

void nordictrackifitadbtreadmill::onWatt(double watt) {
    m_watt = watt;
    wattReadFromTM = true;
}

void nordictrackifitadbtreadmill::onCadence(double cadence) {
    Cadence = cadence;
    cadenceReadFromTM = true;
}

void nordictrackifitadbtreadmill::onSpeedInclination(double speed, double inclination) {

    parseSpeed(speed);
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

    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
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

#ifdef Q_OS_ANDROID
    // Use gRPC to fetch current metrics instead of OCR
    if (grpcInitialized) {
        double currentSpeed = getGrpcSpeed();
        double currentIncline = getGrpcIncline();
        double currentWatts = getGrpcWatts();
        double currentCadence = getGrpcCadence();
        
        // Update the metrics if they've changed
        if (currentSpeed != Speed.value()) {
            Speed = currentSpeed;
            emit debug(QString("gRPC Speed: %1").arg(currentSpeed));
        }
        
        if (currentIncline != Inclination.value()) {
            Inclination = currentIncline;
            emit debug(QString("gRPC Incline: %1").arg(currentIncline));
        }
        
        if (currentWatts != m_watt.value() && currentWatts > 0) {
            m_watt = currentWatts;
            wattReadFromTM = true;
            emit debug(QString("gRPC Watts: %1").arg(currentWatts));
        }
        
        if (currentCadence != Cadence.value() && currentCadence > 0) {
            Cadence = currentCadence;
            cadenceReadFromTM = true;
            emit debug(QString("gRPC Cadence: %1").arg(currentCadence));
        }

        if (requestInclination != -100) {
            setGrpcIncline(requestInclination);
            requestInclination = -100;
        }
    } else {
        // Fallback to OCR if gRPC is not available
        QAndroidJniObject text = QAndroidJniObject::callStaticObjectMethod<jstring>(
            "org/cagnulen/qdomyoszwift/ScreenCaptureService", "getLastText");
        QString t = text.toString();
        QAndroidJniObject textExtended = QAndroidJniObject::callStaticObjectMethod<jstring>(
            "org/cagnulen/qdomyoszwift/ScreenCaptureService", "getLastTextExtended");
        QString tt = textExtended.toString();
        jint w = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/ScreenCaptureService",
                                                           "getImageWidth", "()I");
        jint h = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/ScreenCaptureService",
                                                           "getImageHeight", "()I");
        QString tExtended = textExtended.toString();
        QAndroidJniObject packageNameJava = QAndroidJniObject::callStaticObjectMethod<jstring>(
            "org/cagnulen/qdomyoszwift/MediaProjection", "getPackageName");
        QString packageName = packageNameJava.toString();

        qDebug() << QStringLiteral("OCR") << packageName << tt;
        processOCROutput(tt, w);
    }
#endif
}

void nordictrackifitadbtreadmill::changeInclinationRequested(double grade, double percentage) {
    // these treadmills support negative inclination
    /*if (percentage < 0)
        percentage = 0;*/
    changeInclination(grade, percentage);
}

bool nordictrackifitadbtreadmill::connected() { return true; }

void nordictrackifitadbtreadmill::stopLogcatAdbThread() {
    qDebug() << "stopLogcatAdbThread()";
    
#ifdef Q_OS_ANDROID
    // Stop gRPC metrics updates
    stopGrpcMetricsUpdates();
#endif
    
#ifdef Q_OS_WIN32
    initiateThreadStop();
    logcatAdbThread->quit();
    logcatAdbThread->terminate();
    
    QProcess process;
    QString command = "/c wmic process where name='adb.exe' delete";
    process.start("cmd.exe", QStringList(command.split(' ')));
    process.waitForFinished(-1); // will wait forever until finished
    
    logcatAdbThread->wait();
#endif
}

void nordictrackifitadbtreadmill::initiateThreadStop() {
#ifdef Q_OS_WIN32
    logcatAdbThread->stop = true;
#endif
}

int nordictrackifitadbtreadmill::x14i_inclination_lookuptable(double reqInclination) {
    int y2 = 0;
    if (reqInclination == -6) { y2 = 856; }
    else if (reqInclination == -5.5) { y2 = 850; }
    else if (reqInclination == -5) { y2 = 844; }
    else if (reqInclination == -4.5) { y2 = 838; }
    else if (reqInclination == -4) { y2 = 832; }
    else if (reqInclination == -3.5) { y2 = 826; }
    else if (reqInclination == -3) { y2 = 820; }
    else if (reqInclination == -2.5) { y2 = 814; }
    else if (reqInclination == -2) { y2 = 808; }
    else if (reqInclination == -1.5) { y2 = 802; }
    else if (reqInclination == -1) { y2 = 796; }
    else if (reqInclination == -0.5) { y2 = 785; }
    else if (reqInclination == 0) { y2 = 783; }
    else if (reqInclination == 0.5) { y2 = 778; }
    else if (reqInclination == 1) { y2 = 774; }
    else if (reqInclination == 1.5) { y2 = 768; }
    else if (reqInclination == 2) { y2 = 763; }
    else if (reqInclination == 2.5) { y2 = 757; }
    else if (reqInclination == 3) { y2 = 751; }
    else if (reqInclination == 3.5) { y2 = 745; }
    else if (reqInclination == 4) { y2 = 738; }
    else if (reqInclination == 4.5) { y2 = 731; }
    else if (reqInclination == 5) { y2 = 724; }
    else if (reqInclination == 5.5) { y2 = 717; }
    else if (reqInclination == 6) { y2 = 710; }
    else if (reqInclination == 6.5) { y2 = 703; }
    else if (reqInclination == 7) { y2 = 696; }
    else if (reqInclination == 7.5) { y2 = 691; }
    else if (reqInclination == 8) { y2 = 687; }
    else if (reqInclination == 8.5) { y2 = 683; }
    else if (reqInclination == 9) { y2 = 677; }
    else if (reqInclination == 9.5) { y2 = 671; }
    else if (reqInclination == 10) { y2 = 665; }
    else if (reqInclination == 10.5) { y2 = 658; }
    else if (reqInclination == 11) { y2 = 651; }
    else if (reqInclination == 11.5) { y2 = 645; }
    else if (reqInclination == 12) { y2 = 638; }
    else if (reqInclination == 12.5) { y2 = 631; }
    else if (reqInclination == 13) { y2 = 624; }
    else if (reqInclination == 13.5) { y2 = 617; }
    else if (reqInclination == 14) { y2 = 610; }
    else if (reqInclination == 14.5) { y2 = 605; }
    else if (reqInclination == 15) { y2 = 598; }
    else if (reqInclination == 15.5) { y2 = 593; }
    else if (reqInclination == 16) { y2 = 587; }
    else if (reqInclination == 16.5) { y2 = 581; }
    else if (reqInclination == 17) { y2 = 575; }
    else if (reqInclination == 17.5) { y2 = 569; }
    else if (reqInclination == 18) { y2 = 563; }
    else if (reqInclination == 18.5) { y2 = 557; }
    else if (reqInclination == 19) { y2 = 551; }
    else if (reqInclination == 19.5) { y2 = 545; }
    else if (reqInclination == 20) { y2 = 539; }
    else if (reqInclination == 20.5) { y2 = 533; }
    else if (reqInclination == 21) { y2 = 527; }
    else if (reqInclination == 21.5) { y2 = 521; }
    else if (reqInclination == 22) { y2 = 515; }
    else if (reqInclination == 22.5) { y2 = 509; }
    else if (reqInclination == 23) { y2 = 503; }
    else if (reqInclination == 23.5) { y2 = 497; }
    else if (reqInclination == 24) { y2 = 491; }
    else if (reqInclination == 24.5) { y2 = 485; }
    else if (reqInclination == 25) { y2 = 479; }
    else if (reqInclination == 25.5) { y2 = 473; }
    else if (reqInclination == 26) { y2 = 467; }
    else if (reqInclination == 26.5) { y2 = 461; }
    else if (reqInclination == 27) { y2 = 455; }
    else if (reqInclination == 27.5) { y2 = 449; }
    else if (reqInclination == 28) { y2 = 443; }
    else if (reqInclination == 28.5) { y2 = 437; }
    else if (reqInclination == 29) { y2 = 431; }
    else if (reqInclination == 29.5) { y2 = 425; }
    else if (reqInclination == 30) { y2 = 418; }
    else if (reqInclination == 30.5) { y2 = 412; }
    else if (reqInclination == 31) { y2 = 406; }
    else if (reqInclination == 31.5) { y2 = 400; }
    else if (reqInclination == 32) { y2 = 394; }
    else if (reqInclination == 32.5) { y2 = 388; }
    else if (reqInclination == 33) { y2 = 382; }
    else if (reqInclination == 33.5) { y2 = 375; }
    else if (reqInclination == 34) { y2 = 369; }
    else if (reqInclination == 34.5) { y2 = 363; }
    else if (reqInclination == 35) { y2 = 357; }
    else if (reqInclination == 35.5) { y2 = 351; }
    else if (reqInclination == 36) { y2 = 345; }
    else if (reqInclination == 36.5) { y2 = 338; }
    else if (reqInclination == 37) { y2 = 332; }
    else if (reqInclination == 37.5) { y2 = 326; }
    else if (reqInclination == 38) { y2 = 320; }
    else if (reqInclination == 38.5) { y2 = 314; }
    else if (reqInclination == 39) { y2 = 308; }
    else if (reqInclination == 39.5) { y2 = 302; }
    else if (reqInclination == 40) { y2 = 295; }
    return y2;        
}

// gRPC integration methods implementation
void nordictrackifitadbtreadmill::initializeGrpcService() {
#ifdef Q_OS_ANDROID
    if (!grpcInitialized) {
        try {
            // Set Android context first
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                "setContext",
                "(Landroid/content/Context;)V",
                QtAndroid::androidContext().object()
            );
            
            // Now initialize the service
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                "initialize",
                "()V"
            );
            grpcInitialized = true;
            emit debug("gRPC service initialized successfully");
        } catch (...) {
            emit debug("Failed to initialize gRPC service");
            grpcInitialized = false;
        }
    }
#endif
}

void nordictrackifitadbtreadmill::startGrpcMetricsUpdates() {
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

void nordictrackifitadbtreadmill::stopGrpcMetricsUpdates() {
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

double nordictrackifitadbtreadmill::getGrpcSpeed() {
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

double nordictrackifitadbtreadmill::getGrpcIncline() {
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

double nordictrackifitadbtreadmill::getGrpcWatts() {
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

double nordictrackifitadbtreadmill::getGrpcCadence() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentCadence",
            "()D"
        );
    }
#endif
    return 0.0;
}

void nordictrackifitadbtreadmill::setGrpcSpeed(double speed) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "adjustSpeed",
            "(D)V",
            speed - Speed.value()
        );
        emit debug(QString("Set gRPC speed to: %1").arg(speed));
    }
#endif
}

void nordictrackifitadbtreadmill::setGrpcIncline(double incline) {
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
