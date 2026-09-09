#include "waterrowerusb.h"
#include <QBluetoothLocalDevice>
#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QProcess>
#include <QSettings>
#include <QThread>
#include <chrono>
#include "qzsettings.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"

#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#endif

using namespace std::chrono_literals;

waterrowerusbThread::waterrowerusbThread(QObject *parent) : QThread(parent) {
    qDebug() << QStringLiteral("waterrowerusbThread::waterrowerusbThread()");
}

void waterrowerusbThread::run() {
    qDebug() << QStringLiteral("waterrowerusbThread::run() start");
    
    mutex.lock();
    running = true;
    mutex.unlock();

    while (running) {
#ifdef Q_OS_ANDROID
        bool isConnected = QAndroidJniObject::callStaticMethod<jboolean>(
            "org/cagnulen/qdomyoszwift/WaterRowerBridge",
            "isConnected",
            "()Z");

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (!isConnected && (lastInitializeAttempt == 0 || now - lastInitializeAttempt > 5000)) {
            lastInitializeAttempt = now;
            initializeWaterRower();
        }
        processWaterRowerData();
#endif
        QThread::msleep(200); // Poll every 200ms
    }

#ifdef Q_OS_ANDROID
    shutdownWaterRower();
#endif

    qDebug() << QStringLiteral("waterrowerusbThread::run() end");
}

void waterrowerusbThread::stop() {
    qDebug() << QStringLiteral("waterrowerusbThread::stop()");
    mutex.lock();
    running = false;
    mutex.unlock();
}

#ifdef Q_OS_ANDROID
void waterrowerusbThread::initializeWaterRower() {
    emit onDebug(QStringLiteral("Initializing WaterRower USB connection..."));
    
    // Call the Java WaterRower initialization
    QAndroidJniEnvironment env;
    
    // Get the device path
    QAndroidJniObject devicePathObject = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/WaterRowerBridge",
        "getDevicePath",
        "(Landroid/content/Context;)Ljava/lang/String;",
        QtAndroid::androidContext().object());

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        emit onError(QStringLiteral("Failed to get WaterRower device path"));
        return;
    }

    QString devicePath = devicePathObject.toString();
    if (devicePath.isEmpty()) {
        emit onDebug(QStringLiteral("WaterRower device not found."));
        return;
    }

    emit onDebug(QStringLiteral("WaterRower device found at: ") + devicePath);

    // Create WaterRower instance through JNI
    QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/WaterRowerBridge", 
        "connect", 
        "(Landroid/content/Context;Ljava/lang/String;)Ljava/lang/String;",
        QtAndroid::androidContext().object(), QAndroidJniObject::fromString(devicePath).object<jstring>());
    
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        emit onError(QStringLiteral("Failed to connect to WaterRower"));
        return;
    }
    
    QString initResult = result.toString();
    emit onDebug(QStringLiteral("WaterRower connection result: ") + initResult);
    
    if (initResult == "SUCCESS") {
        emit onDebug(QStringLiteral("WaterRower connection successful - waiting for data..."));
        // Don't emit onConnected() here - wait for actual device connection from processWaterRowerData()
    } else {
        emit onError(QStringLiteral("WaterRower connection failed: ") + initResult);
    }
}

void waterrowerusbThread::processWaterRowerData() {
    QAndroidJniEnvironment env;
    
    // Get stroke data from Java
    QAndroidJniObject strokeData = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/WaterRowerBridge",
        "getStrokeData",
        "()Ljava/lang/String;");
    
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return;
    }
    
    QString data = strokeData.toString();
    if (!data.isEmpty() && data != "NO_DATA") {
        // Parse data format: "strokeRate,distance,pace,watts,calories,strokeCount"
        QStringList values = data.split(',');
        if (values.size() >= 5) {
            double strokeRate = values[0].toDouble();
            double distance = values[1].toDouble();
            double pace = values[2].toDouble();
            double watts = values[3].toDouble();
            double calories = values[4].toDouble();
            double strokeCount = values.size() >= 6 ? values[5].toDouble() : 0;
            
            emit onStroke(strokeRate, distance, pace, watts, calories, strokeCount);
        }
    }
    
    // Check connection status
    bool isConnected = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/cagnulen/qdomyoszwift/WaterRowerBridge",
        "isConnected",
        "()Z");
    
    static bool lastConnectedState = false;
    if (isConnected != lastConnectedState) {
        lastConnectedState = isConnected;
        if (isConnected) {
            emit onDebug(QStringLiteral("WaterRower device connected"));
            emit onConnected();
        } else {
            emit onDebug(QStringLiteral("WaterRower device disconnected"));
            emit onDisconnected();
        }
    }
}

void waterrowerusbThread::shutdownWaterRower() {
    emit onDebug(QStringLiteral("Shutting down WaterRower connection..."));
    
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/WaterRowerBridge",
        "shutdown",
        "()V");
#endif
}
#endif

waterrowerusb::waterrowerusb(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    qDebug() << QStringLiteral("waterrowerusb::waterrowerusb()");
    
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &waterrowerusb::update);
    refresh->start(200ms);

#ifdef Q_OS_IOS
    h = new lockscreen();
#endif

    // Create and start worker thread
    workerThread = new waterrowerusbThread(this);
    connect(workerThread, &waterrowerusbThread::onDebug, this, &waterrowerusb::debug);
    connect(workerThread, &waterrowerusbThread::onConnected, this, &waterrowerusb::onWaterRowerConnected);
    connect(workerThread, &waterrowerusbThread::onDisconnected, this, &waterrowerusb::onWaterRowerDisconnected);
    connect(workerThread, &waterrowerusbThread::onError, this, &waterrowerusb::onWaterRowerError);
    connect(workerThread, &waterrowerusbThread::onStroke, this, &waterrowerusb::onWaterRowerStroke);
    
    workerThread->start();
}

waterrowerusb::~waterrowerusb() {
    qDebug() << QStringLiteral("~waterrowerusb()");
    
    if (workerThread) {
        workerThread->stop();
        workerThread->wait(3000);
        delete workerThread;
    }

#ifdef Q_OS_IOS
    if (h)
        delete h;
#endif
}

void waterrowerusb::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    Q_UNUSED(device)
    // WaterRower USB doesn't use Bluetooth discovery
}

bool waterrowerusb::connected() {
    return initDone;
}

uint16_t waterrowerusb::watts() {
    return m_watt.value();
}

void waterrowerusb::update() {
    if (initRequest) {
        initRequest = false;
        // WaterRower USB initialization is handled by the worker thread
        qDebug() << QStringLiteral("WaterRower USB init requested");
    }

    if (initDone) {
        QSettings settings;
        QString heartRateBeltName =
            settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
        bool heart_rate_check = heartRateBeltName.startsWith(QStringLiteral("Disabled"));

        update_metrics(false, watts());

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        // ******************************************* virtual bike/rower init *************************************
        if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            && !h
#endif
#endif
        ) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled)
                    .toBool();
            bool virtual_device_rower =
                settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor)
                               .toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround)
                    .toBool();
            if (ios_peloton_workaround && cadence && !virtual_device_rower) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
            {
                if (!noVirtualDevice && virtual_device_enabled) {
                    if (!virtual_device_rower) {
                        qDebug() << QStringLiteral("creating virtual bike interface...");
                        auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                        this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                    } else {
                        qDebug() << QStringLiteral("creating virtual rower interface...");
                        auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                        this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
                    }
                }
            }
        }
        if (!firstStateChanged)
            emit connectedAndDiscovered();
        firstStateChanged = 1;
        // ********************************************************************************************************

        if (Heart.value() == 0.0 && !heart_rate_check) {
            update_hr_from_external();
        }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor)
                           .toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence && h && firstStateChanged) {
            h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)currentHeart().value());
        }
#endif
#endif

        if (sec1Update++ == (1000 / refresh->interval())) {
            sec1Update = 0;
            updateDisplay(elapsed.value());
        }

        if (!noVirtualDevice) {
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
            if (h) {
                h->virtualbike_setHeartRate((uint8_t)currentHeart().value());
            }
#endif
#endif
        }
    }
}

void waterrowerusb::onWaterRowerConnected() {
    qDebug() << QStringLiteral("WaterRower USB connected");
    emit debug(QStringLiteral("WaterRower USB connected"));
    initDone = true;
    // emit connectedChanged(); // This signal doesn't exist in base class
}

void waterrowerusb::onWaterRowerDisconnected() {
    qDebug() << QStringLiteral("WaterRower USB disconnected");
    emit debug(QStringLiteral("WaterRower USB disconnected"));
    initDone = false;
    emit disconnected();
}

void waterrowerusb::onWaterRowerError(QString error) {
    qDebug() << QStringLiteral("WaterRower USB error:") << error;
    emit debug(QStringLiteral("WaterRower USB error: ") + error);
}

void waterrowerusb::onWaterRowerStroke(double strokeRate, double distance, double pace, double watts, double calories, double strokeCount) {
    qDebug() << QStringLiteral("WaterRower stroke data - Rate:") << strokeRate 
             << QStringLiteral("Distance:") << distance 
             << QStringLiteral("Pace:") << pace
             << QStringLiteral("Watts:") << watts
             << QStringLiteral("Calories:") << calories
             << QStringLiteral("Stroke Count:") << strokeCount;

    // WaterRower reports distance in meters; QZ stores distance in kilometers.
    const double distanceKm = distance / 1000.0;
    Cadence = strokeRate;
    Distance = distanceKm;
    Distance1s = distanceKm;
    StrokesCount = strokeCount;
    if (watts > 0) {
        m_watt = watts;
    } else if (strokeRate <= 0 || m_watt.value() <= 0) {
        m_watt = rower::calculateWattsFromPace(pace);
    }
    KCal = calories;

    // The USB bridge pace/velocity is quantized and makes the speed graph jump.
    // Derive speed from distance deltas and smooth it before publishing.
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (distance <= 0 || lastSpeedDistanceMeters < 0 || distance < lastSpeedDistanceMeters) {
        lastSpeedDistanceMeters = distance;
        lastSpeedDistanceTimestamp = now;
        filteredSpeedKph = 0;
        Speed = 0;
    } else if (distance > lastSpeedDistanceMeters) {
        const qint64 deltaMs = now - lastSpeedDistanceTimestamp;
        if (deltaMs >= 2000) {
            const double deltaMeters = distance - lastSpeedDistanceMeters;
            const double rawSpeedKph = (deltaMeters / (static_cast<double>(deltaMs) / 1000.0)) * 3.6;
            if (rawSpeedKph <= 30.0) {
                filteredSpeedKph = filteredSpeedKph > 0 ? (filteredSpeedKph * 0.8) + (rawSpeedKph * 0.2) : rawSpeedKph;
                Speed = filteredSpeedKph;
            }
            lastSpeedDistanceMeters = distance;
            lastSpeedDistanceTimestamp = now;
        }
    } else if (lastSpeedDistanceTimestamp > 0 && now - lastSpeedDistanceTimestamp > 3000) {
        filteredSpeedKph = 0;
        Speed = 0;
    }

    emit debug(QStringLiteral("Updated metrics - Cadence: %1, Distance: %2, Watts: %3, Speed: %4, Stroke Count: %5")
                   .arg(Cadence.value()).arg(Distance.value()).arg(m_watt.value()).arg(Speed.value())
                   .arg(StrokesCount.value()));
}

void waterrowerusb::updateDisplay(uint16_t elapsed) {
    Q_UNUSED(elapsed);
    // WaterRower USB doesn't need display updates
}
