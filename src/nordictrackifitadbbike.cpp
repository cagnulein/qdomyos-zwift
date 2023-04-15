#include "nordictrackifitadbbike.h"
#include "homeform.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "virtualtreadmill.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

nordictrackifitadbbike::nordictrackifitadbbike(bool noWriteResistance, bool noHeartService) {
    QSettings settings;
    bool nordictrack_ifit_adb_remote = settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote).toBool();
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
    if (!firstStateChanged && !virtualBike) {
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        if (virtual_device_enabled) {
            debug("creating virtual bike interface...");
            virtualBike = new virtualbike(this);
            connect(virtualBike, &virtualbike::changeInclination, this,
                    &nordictrackifitadbbike::changeInclinationRequested);
            firstStateChanged = 1;
        }
    }
    // ********************************************************************************************************

#ifdef Q_OS_ANDROID
    if(nordictrack_ifit_adb_remote) {
        QAndroidJniObject IP = QAndroidJniObject::fromString(ip).object<jstring>();
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "createConnection",
                                              "(Ljava/lang/String;Landroid/content/Context;)V", IP.object<jstring>(), QtAndroid::androidContext().object());
    }
#endif
}

bool nordictrackifitadbbike::inclinationAvailableByHardware() { return true; }

double nordictrackifitadbbike::getDouble(QString v) {
    QChar d = QLocale().decimalPoint();
    if(d == ',') {
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
            if (line.contains(QStringLiteral("Changed KPH"))) {
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
                }
            } else if (line.contains(QStringLiteral("Changed Resistance"))) {
                QStringList aValues = line.split(" ");
                if (aValues.length()) {
                    resistance = getDouble(aValues.last());
                    m_pelotonResistance = (100 / 32) * resistance;
                    qDebug() << QStringLiteral("Current Peloton Resistance: ") << m_pelotonResistance.value() << resistance;
                    //Resistance = resistance;
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

        // since the motor of the bike is slow, let's filter the inclination changes to more than 4 seconds
        if(lastInclinationChanged.secsTo(QDateTime::currentDateTime()) > 4) {
            lastInclinationChanged = QDateTime::currentDateTime();
    #ifdef Q_OS_ANDROID
            bool nordictrack_ifit_adb_remote = settings.value(QZSettings::nordictrack_ifit_adb_remote, QZSettings::default_nordictrack_ifit_adb_remote).toBool();
            if(nordictrack_ifit_adb_remote) {
                if(requestInclination != -100) {
                    double inc = qRound(requestInclination / 0.5) * 0.5;
                    if(inc != currentInclination().value()) {
                        int x1 = 75;
                        int y2 = (int) (616.18 - (17.223 * (inc + gears())));
                        int y1Resistance = (int) (616.18 - (17.223 * currentInclination().value()));

                        lastCommand = "input swipe " + QString::number(x1) + " " + QString::number(y1Resistance) + " " + QString::number(x1) + " " + QString::number(y2) + " 200";
                        qDebug() << " >> " + lastCommand;
                        QAndroidJniObject command = QAndroidJniObject::fromString(lastCommand).object<jstring>();
                        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/QZAdbRemote", "sendCommand",
                                                              "(Ljava/lang/String;)V", command.object<jstring>());
                    }
                }
                requestInclination = -100;
            }
    #endif

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

void *nordictrackifitadbbike::VirtualDevice() { return virtualBike; }
