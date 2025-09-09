#include "fakebike.h"
#include "virtualdevices/virtualbike.h"

#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif
#include <chrono>

using namespace std::chrono_literals;

fakebike::fakebike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &fakebike::update);
    refresh->start(200ms);

    // Initialize socket client for receiving metrics from another QZ instance
    socketClient = new QTcpSocket(this);
    connect(socketClient, &QTcpSocket::readyRead, this, &fakebike::socketReadyRead);
    connect(socketClient, &QTcpSocket::connected, this, &fakebike::socketConnected);
    connect(socketClient, &QTcpSocket::disconnected, this, &fakebike::socketDisconnected);

    usingSocketData = false;
    
    // Initialize mDNS discovery to find remote QZ instances
    initMdnsDiscovery();
}

void fakebike::update() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    // Check if we should switch back to simulated data (no socket data for 5 seconds)
    if (usingSocketData && lastSocketData.msecsTo(QDateTime::currentDateTime()) > 5000) {
        usingSocketData = false;
        qDebug() << QStringLiteral("Switching back to simulated data - socket timeout");
    }

    // Only use simulated data if we're not receiving socket data
    if (!usingSocketData) {
        /*
        static int updcou = 0;
        updcou++;
        double w = 60.0;
        if (updcou > 20000 )
            updcou = 0;
        else if (updcou > 12000)
            w = 120;
        else if (updcou > 6000)
            w = 80;
        Speed = metric::calculateSpeedFromPower(w, Inclination.value(),
        Speed.value(),fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), speedLimit());
        */

        if (requestPower != -1) {
            // bepo70: don't know if this conversion is really needed, i would do it anyway.
            m_watt = (double)requestPower * (1.0 + (((double)rand() / RAND_MAX) * 0.4 - 0.2));
            if(requestPower)
                Cadence = 50 + (static_cast<double>(rand()) / RAND_MAX) * 50;
            else
                Cadence = 0;
            qDebug() << QStringLiteral("writing power ") + QString::number(requestPower);
            //requestPower = -1;
            // bepo70: Disregard the current inclination for calculating speed. When the video
            //         has a high inclination you have to give many power to get the desired playback speed,
            //         if inclination is very low little more power gives a quite high speed jump.
            // Speed = metric::calculateSpeedFromPower(m_watt.value(), Inclination.value(),
            // Speed.value(),fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), speedLimit());
            Speed = metric::calculateSpeedFromPower(
                m_watt.value(), 0, Speed.value(), fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0),
                speedLimit());
        }
    }
    
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                              // kg * 3.5) / 200 ) / 60
    
    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    if (requestInclination != -100) {
        Inclination = requestInclination;
        qDebug() << QStringLiteral("writing incline ") + QString::number(requestInclination);
        requestInclination = -100;
    }

    update_metrics(false, watts());

    Distance += ((Speed.value() / (double)3600.0) /
                 ((double)1000.0 / (double)(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();    

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice() && !noVirtualDevice
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {
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
            auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
            connect(virtualBike, &virtualbike::changeInclination, this, &fakebike::changeInclinationRequested);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &fakebike::ftmsCharacteristicChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    if (!firstStateChanged)
        emit connectedAndDiscovered();
    firstStateChanged = 1;
    // ********************************************************************************************************

    if (!noVirtualDevice) {
#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
            Heart = (uint8_t)KeepAwakeHelper::heart();
            debug("Current Heart: " + QString::number(Heart.value()));
        }
#endif
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            update_hr_from_external();
        }
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence =
            settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence && h && firstStateChanged) {
            h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
        }
#endif
#endif
    }

    if (Heart.value()) {
        static double lastKcal = 0;
        if (KCal.value() < 0) // if the user pressed stop, the KCAL resets the accumulator
            lastKcal = abs(KCal.value());
        KCal = metric::calculateKCalfromHR(Heart.average(), elapsed.value()) + lastKcal;
    }

    if (requestResistance != -1 && requestResistance != currentResistance().value()) {
        Resistance = requestResistance;
        m_pelotonResistance = requestResistance;
    }
}

void fakebike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QByteArray b = newValue;
    qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');
}

void fakebike::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

uint16_t fakebike::wattsFromResistance(double resistance) {
    return _ergTable.estimateWattage(Cadence.value(), resistance);
}

resistance_t fakebike::resistanceFromPowerRequest(uint16_t power) {
    return _ergTable.resistanceFromPowerRequest(power, Cadence.value(), maxResistance());
}


uint16_t fakebike::watts() { return m_watt.value(); }
bool fakebike::connected() { return true; }

double fakebike::maxGears() {
    return 24;
}

double fakebike::minGears() {
    return 1;
}

void fakebike::socketReadyRead() {
    QByteArray data = socketClient->readAll();
    socketBuffer += QString::fromUtf8(data);
    
    // Look for complete messages ending with '#'
    int endPos;
    while ((endPos = socketBuffer.indexOf('#')) != -1) {
        QString message = socketBuffer.left(endPos + 1);
        socketBuffer = socketBuffer.mid(endPos + 1);
        
        if (message.contains("SENDER=PAD#")) {
            parseSocketData(message);
            lastSocketData = QDateTime::currentDateTime();
            usingSocketData = true;
            qDebug() << QStringLiteral("Received socket data: ") + message;
        }
    }
}

void fakebike::socketConnected() {
    qDebug() << QStringLiteral("Socket client connected to remote QZ instance");
}

void fakebike::socketDisconnected() {
    qDebug() << QStringLiteral("Socket client disconnected from remote QZ instance");
    usingSocketData = false;
    // Try to rediscover services after 5 seconds
    QTimer::singleShot(5000, [this]() {
        if (socketClient->state() == QAbstractSocket::UnconnectedState) {
            initMdnsDiscovery();
        }
    });
}

void fakebike::parseSocketData(const QString &data) {
    // Parse message format: SENDER=PAD#HR=X#KCAL=X#BCAD=X#SPD=X#PWR=X#CAD=X#ODO=X#
    QStringList parts = data.split('#');
    
    for (const QString &part : parts) {
        if (part.startsWith("SPD=")) {
            double speed = part.mid(4).toDouble();
            Speed = speed;
        } else if (part.startsWith("PWR=")) {
            double power = part.mid(4).toDouble();
            m_watt = power;
        } else if (part.startsWith("CAD=")) {
            double cadence = part.mid(4).toDouble();
            Cadence = cadence;
        } else if (part.startsWith("HR=")) {
            double heartRate = part.mid(3).toDouble();
            Heart = heartRate;
        }
    }
}

void fakebike::initMdnsDiscovery() {
#ifndef Q_OS_IOS
    qDebug() << QStringLiteral("Starting mDNS discovery for QZ PAD service");
    
    // Clean up existing browser if any
    if (mdnsBrowser) {
        delete mdnsBrowser;
        mdnsBrowser = nullptr;
    }
    if (mdnsResolver) {
        delete mdnsResolver;
        mdnsResolver = nullptr;
    }
    
    // Create mDNS browser to look for "_qz_iphone._tcp.local." services (PAD servers)
    mdnsBrowser = new QMdnsEngine::Browser(&mdnsServer, "_qz_iphone._tcp.local.", &mdnsCache);
    
    // Handle service discovery
    QObject::connect(mdnsBrowser, &QMdnsEngine::Browser::serviceAdded, [this](const QMdnsEngine::Service &service) {
        mdnsService = service;
        qDebug() << QStringLiteral("QZ PAD service discovered: ") + service.name() + " at " + service.hostname() + ":" + QString::number(service.port());
        
        // Clean up existing resolver
        if (mdnsResolver) {
            delete mdnsResolver;
        }
        
        // Resolve the hostname to IP address
        mdnsResolver = new QMdnsEngine::Resolver(&mdnsServer, service.hostname(), &mdnsCache);
        QObject::connect(mdnsResolver, &QMdnsEngine::Resolver::resolved, [this](const QHostAddress &address) {
            qDebug() << QStringLiteral("QZ PAD service resolved to: ") + address.toString();
            
            if (address.protocol() == QAbstractSocket::IPv4Protocol && 
                (socketClient->state() == QAbstractSocket::UnconnectedState || !remoteAddress.isEqual(address))) {
                
                // Disconnect existing connection if any
                if (socketClient->state() != QAbstractSocket::UnconnectedState) {
                    socketClient->disconnectFromHost();
                }
                
                remoteAddress = address;
                qDebug() << QStringLiteral("Connecting to QZ PAD service at ") + address.toString() + ":" + QString::number(mdnsService.port());
                socketClient->connectToHost(address, mdnsService.port());
            }
        });
    });
    
    // Handle service updates
    QObject::connect(mdnsBrowser, &QMdnsEngine::Browser::serviceUpdated, [this](const QMdnsEngine::Service &service) {
        mdnsService = service;
        qDebug() << QStringLiteral("QZ PAD service updated: ") + service.name() + " at " + service.hostname() + ":" + QString::number(service.port());
        
        // Clean up existing resolver
        if (mdnsResolver) {
            delete mdnsResolver;
        }
        
        // Resolve the updated service
        mdnsResolver = new QMdnsEngine::Resolver(&mdnsServer, service.hostname(), &mdnsCache);
        QObject::connect(mdnsResolver, &QMdnsEngine::Resolver::resolved, [this](const QHostAddress &address) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && 
                (socketClient->state() == QAbstractSocket::UnconnectedState || !remoteAddress.isEqual(address))) {
                
                // Disconnect existing connection if any
                if (socketClient->state() != QAbstractSocket::UnconnectedState) {
                    socketClient->disconnectFromHost();
                }
                
                remoteAddress = address;
                qDebug() << QStringLiteral("Reconnecting to updated QZ PAD service at ") + address.toString() + ":" + QString::number(mdnsService.port());
                socketClient->connectToHost(address, mdnsService.port());
            }
        });
    });
#endif
}
