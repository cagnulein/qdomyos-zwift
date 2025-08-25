#include "pelotonbike.h"
#include "homeform.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>
#include "localipaddress.h"

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

using namespace std::chrono_literals;

pelotonbike::pelotonbike(bool noWriteResistance, bool noHeartService) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &pelotonbike::update);
    refresh->start(200ms);    

    pelotonOCRsocket = new QUdpSocket(this);
    bool result = pelotonOCRsocket->bind(QHostAddress::AnyIPv4, 8003);
    qDebug() << result;
    pelotonOCRprocessPendingDatagrams();
    connect(pelotonOCRsocket, SIGNAL(readyRead()), this, SLOT(pelotonOCRprocessPendingDatagrams()));
    
    // ******************************************* Peloton sensor init *************************************
#ifdef Q_OS_ANDROID
    initializePelotonSensorService();
#endif
    
    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()) {
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        if (virtual_device_enabled) {
            debug("creating virtual bike interface...");
            auto virtualBike = new virtualbike(this);
            connect(virtualBike, &virtualbike::changeInclination, this, &pelotonbike::changeInclinationRequested);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            firstStateChanged = 1;
        }
    }
    // ********************************************************************************************************
}

bool pelotonbike::inclinationAvailableByHardware() { return true; }

void pelotonbike::forceResistance(double resistance) {}

void pelotonbike::pelotonOCRprocessPendingDatagrams() {
    qDebug() << "in !";
    QHostAddress sender;
    QSettings settings;
    uint16_t port;
    while (pelotonOCRsocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(pelotonOCRsocket->pendingDatagramSize());
        pelotonOCRsocket->readDatagram(datagram.data(), datagram.size(), &sender, &port);
        qDebug() << "PelotonOCR Message From :: " << sender.toString();
        qDebug() << "PelotonOCR Port From :: " << port;
        qDebug() << "PelotonOCR Message :: " << datagram;

        QString s = datagram;
        QStringList metrics = s.split(";");
        if(s.length() >= 5) {
            m_watt = metrics.at(1).toDouble();
            qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());
            Cadence = metrics.at(2).toDouble();
            qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
            Resistance = metrics.at(3).toDouble();
            qDebug() << QStringLiteral("Current Resistance: ") + QString::number(Resistance.value());
            Speed = metrics.at(4).toDouble();
            qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
        }


        QString url = "http://" + localipaddress::getIP(sender).toString() + ":" +
                      QString::number(settings.value("template_inner_QZWS_port", 6666).toInt()) +
                      "/floating/floating.htm";
        int r = pelotonOCRsocket->writeDatagram(QByteArray(url.toLatin1()), sender, 8003);
        qDebug() << "url floating" << url << r;
    }
}

void pelotonbike::update() {

    QSettings settings;
    
#ifdef Q_OS_ANDROID
    // Use Peloton sensor data if available, otherwise fall back to OCR
    if (pelotonSensorInitialized && isPelotonSensorConnected()) {
        // Read sensor values from Java backend
        float power = getPelotonPower();
        float cadence = getPelotonCadence();
        float resistance = getPelotonResistance();
        float speed = getPelotonSpeed();
        
        // Update metrics with sensor data
        m_watt = power;
        Cadence = cadence;
        Resistance = resistance;
        Speed = speed;
    }
#endif
    
    update_metrics(false, 0);

    if(!initDone) {
        initDone = true;
        emit connectedAndDiscovered();
    }

    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();

    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
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
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    // debug("Current Distance: " + QString::number(distance));

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

uint16_t pelotonbike::watts() { return m_watt.value(); }

void pelotonbike::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

bool pelotonbike::connected() { 
#ifdef Q_OS_ANDROID
    return pelotonSensorInitialized && isPelotonSensorConnected();
#else
    return true; 
#endif
}

#ifdef Q_OS_ANDROID
// Peloton sensor integration methods (similar to nordictrackadbbike GRPC approach)

void pelotonbike::initializePelotonSensorService() {
    if (!pelotonSensorInitialized) {
        try {
            // Set context for the Java service
            QAndroidJniObject context = QtAndroid::androidActivity();
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
                "setContext",
                "(Landroid/content/Context;)V",
                context.object<jobject>()
            );
            
            // Initialize the service
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
                "initialize",
                "()V"
            );
            
            pelotonSensorInitialized = true;
            startPelotonSensorUpdates();
            emit debug("Peloton sensor service initialized successfully");
            
        } catch (...) {
            emit debug("Failed to initialize Peloton sensor service");
            pelotonSensorInitialized = false;
        }
    }
}

void pelotonbike::startPelotonSensorUpdates() {
    if (pelotonSensorInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
            "startSensorUpdates",
            "()V"
        );
        emit debug("Started Peloton sensor updates");
    }
}

void pelotonbike::stopPelotonSensorUpdates() {
    if (pelotonSensorInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
            "stopSensorUpdates",
            "()V"
        );
        emit debug("Stopped Peloton sensor updates");
    }
}

float pelotonbike::getPelotonPower() {
    if (pelotonSensorInitialized) {
        return QAndroidJniObject::callStaticMethod<jfloat>(
            "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
            "getCurrentPower",
            "()F"
        );
    }
    return 0.0f;
}

float pelotonbike::getPelotonCadence() {
    if (pelotonSensorInitialized) {
        return QAndroidJniObject::callStaticMethod<jfloat>(
            "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
            "getCurrentCadence",
            "()F"
        );
    }
    return 0.0f;
}

float pelotonbike::getPelotonResistance() {
    if (pelotonSensorInitialized) {
        return QAndroidJniObject::callStaticMethod<jfloat>(
            "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
            "getCurrentResistance",
            "()F"
        );
    }
    return 0.0f;
}

float pelotonbike::getPelotonSpeed() {
    if (pelotonSensorInitialized) {
        return QAndroidJniObject::callStaticMethod<jfloat>(
            "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
            "getCurrentSpeed",
            "()F"
        );
    }
    return 0.0f;
}

bool pelotonbike::isPelotonSensorConnected() {
    if (pelotonSensorInitialized) {
        return QAndroidJniObject::callStaticMethod<jboolean>(
            "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
            "isConnected",
            "()Z"
        );
    }
    return false;
}
#endif


