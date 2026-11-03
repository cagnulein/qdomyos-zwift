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
    m_ergClock.start();
    m_watt.setType(metric::METRIC_WATT, deviceType());
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

bool pelotonbike::forceResistance(double resistance) {
#ifdef Q_OS_ANDROID
    // noWriteResistance is used by QZ when the virtual device must be read-only.
    if (noWriteResistance)
        return true;

    if (!pelotonSensorInitialized || !isPelotonSensorConnected()) {
        emit debug(QStringLiteral("Peloton Bike+ resistance write skipped: sensor service not connected"));
        return false;
    }

    const int target = qBound(0, qRound(resistance), 100);
    const bool result = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/cagnulen/qdomyoszwift/PelotonSensorHelper",
        "setResistance",
        "(I)Z",
        static_cast<jint>(target)
    );

    emit debug(QStringLiteral("Peloton Bike+ set resistance ") + QString::number(target) +
               (result ? QStringLiteral(" OK") : QStringLiteral(" FAILED")));
    return result;
#else
    Q_UNUSED(resistance)
    return false;
#endif
}


void pelotonbike::resetErg() {
    m_ergActive = false;
    m_ergTargetWatts = 0;
    m_ergFeedForwardPending = false;
    m_ergGoalResistance = -1;
    m_ergExpectedResistance = -1;
    m_ergLastObservedResistance = -1;
    m_ergCadenceSince = -1;
    m_ergPedalingSince = -1;
    m_ergUnexpectedResistance = -1;
    m_ergUnexpectedSamples = 0;
    requestResistance = -1;
}

resistance_t pelotonbike::resistanceFromPowerRequest(uint16_t power) {
    const int estimate = m_powerModel.resistanceFor(power, static_cast<float>(Cadence.value()));
    return estimate >= 0 ? estimate : qBound(0, qRound(Resistance.value()), 100);
}

void pelotonbike::changePower(int32_t power) {
    QSettings settings;
    const int offset = settings.value(QZSettings::bike_power_offset, QZSettings::default_bike_power_offset).toInt();
    const qint64 requested = static_cast<qint64>(power) + offset;
    const bool enabled = autoResistanceEnable && !noWriteResistance &&
                         settings.value(QZSettings::virtualbike_forceresistance,
                                        QZSettings::default_virtualbike_forceresistance).toBool() &&
                         requested >= 25;

    // Preserve QZ's RequestedPower display, offsets and existing filtering. The
    // base class also calls changeResistance(), which we intercept below: the
    // motor must be ramped by our controller, never jumped to the estimate.
    m_ergPowerCommand = true;
    bike::changePower(power);
    m_ergPowerCommand = false;

    if (!enabled) {
        resetErg();
        return;
    }

    const int target = static_cast<int>(qBound<qint64>(25, requested, 1000));
    if (m_ergActive && m_ergTargetWatts == target)
        return;

    m_ergActive = true;
    m_ergTargetWatts = target;
    m_ergFeedForwardPending = true;
    m_ergGoalResistance = -1;
    m_ergExpectedResistance = -1;
    m_ergLastObservedResistance = -1;
    m_ergCadenceSince = -1;
    m_ergUnexpectedSamples = 0;
    m_ergSettleUntil = 0;
    emit debug(QStringLiteral("Peloton Bike+ ERG target: %1 W").arg(target));
}

void pelotonbike::changeResistance(resistance_t resistance) {
    if (m_ergPowerCommand) {
        // bike::changePower() calls this for non-native ERG devices. Do not
        // forward its un-ramped resistance to the Bike+ motor.
        bike::changeResistance(resistance);
        requestResistance = -1;
        return;
    }
    resetErg(); // A new manual/FTMS resistance target takes ownership.
    bike::changeResistance(resistance);
}

void pelotonbike::stop(bool pause) {
    resetErg();
    bike::stop(pause);
}

void pelotonbike::setPaused(bool paused) {
    if (paused)
        resetErg();
    bike::setPaused(paused);
}

void pelotonbike::workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state) {
    if (state == bluetoothdevice::STOPPED || state == bluetoothdevice::PAUSED)
        resetErg();
    bike::workoutEventStateChanged(state);
}

void pelotonbike::updateErg(float power, float cadence, float resistance) {
    const qint64 now = m_ergClock.elapsed();
    if (!std::isfinite(power) || !std::isfinite(cadence) || !std::isfinite(resistance)) {
        resetErg();
        return;
    }
    const int current = qBound(0, qRound(resistance), 100);

    // Learn the per-bike power curve only from stable motor/load-cell readings.
    if (m_ergStableResistance != current) {
        m_ergStableResistance = current;
        m_ergStableSince = now;
    } else if (now - m_ergStableSince >= 2500 && m_ergExpectedResistance == -1 &&
               m_ergGoalResistance == -1) {
        m_powerModel.learn(current, power, cadence);
    }

    if (!m_ergActive)
        return;
    QSettings settings;
    if (!autoResistanceEnable || noWriteResistance ||
        !settings.value(QZSettings::virtualbike_forceresistance,
                        QZSettings::default_virtualbike_forceresistance).toBool()) {
        resetErg();
        return;
    }

    // Do not command the brake without steady pedaling. Recompute the initial
    // target after a stop rather than resuming an obsolete motor ramp.
    if (cadence < 25.0f || power <= 0.0f) {
        m_ergPedalingSince = -1;
        m_ergCadenceSince = -1;
        m_ergFeedForwardPending = true;
        m_ergGoalResistance = -1;
        m_ergExpectedResistance = -1;
        m_ergLastObservedResistance = current;
        return;
    }
    if (cadence < 50.0f) {
        m_ergPedalingSince = -1;
        m_ergCadenceSince = -1;
        m_ergFeedForwardPending = true;
        m_ergGoalResistance = -1;
        m_ergLastObservedResistance = current;
        return;
    }
    if (m_ergPedalingSince < 0)
        m_ergPedalingSince = now;
    if (m_ergExpectedResistance != -1) {
        if (current == m_ergExpectedResistance) {
            m_ergLastObservedResistance = current;
            m_ergExpectedResistance = -1;
            m_ergUnexpectedSamples = 0;
        } else if (now - m_ergWriteSince > 2000) {
            emit debug(QStringLiteral("Peloton Bike+ ERG stopped: resistance command not acknowledged"));
            resetErg();
            return;
        } else {
            return; // Do not queue a second motor write while the first is pending.
        }
    }

    if (m_ergFeedForwardPending) {
        if (cadence < 50.0f) {
            m_ergCadenceSince = -1;
            m_ergLastObservedResistance = current;
            return;
        }
        if (m_ergCadenceSince < 0)
            m_ergCadenceSince = now;
        // Grupetto defers large increases during the first 15 seconds of
        // pedaling so the rider's rising cadence does not cause an overshoot.
        if (now - m_ergCadenceSince < 1500 ||
            (m_ergTargetWatts - power >= 15 &&
             now - m_ergPedalingSince < 15000)) {
            m_ergLastObservedResistance = current;
            return;
        }

        m_ergGoalResistance = m_powerModel.resistanceFor(m_ergTargetWatts, cadence);
        if (m_ergGoalResistance < 0)
            return;
        m_ergFeedForwardPending = false;
        m_ergFeedForwardRamp = true;
        m_ergLastObservedResistance = current;
        m_ergCorrectedResistance = static_cast<float>(current);
        emit debug(QStringLiteral("Peloton Bike+ ERG model: %1 W at %2 rpm -> resistance %3")
                   .arg(m_ergTargetWatts).arg(cadence).arg(m_ergGoalResistance));
    } else {
        // The resistance sensor is filtered in Java. Require two identical
        // unexpected samples before treating a change as a manual knob turn.
        if (m_ergLastObservedResistance != -1 && current != m_ergLastObservedResistance) {
            if (m_ergUnexpectedResistance == current)
                ++m_ergUnexpectedSamples;
            else {
                m_ergUnexpectedResistance = current;
                m_ergUnexpectedSamples = 1;
            }
            if (m_ergUnexpectedSamples >= 2) {
                emit debug(QStringLiteral("Peloton Bike+ ERG stopped: manual resistance override"));
                resetErg();
            }
            return;
        }
        m_ergUnexpectedSamples = 0;
        m_ergUnexpectedResistance = -1;
    }

    if (cadence < 50.0f)
        return;

    if (m_ergGoalResistance >= 0) {
        if (current == m_ergGoalResistance) {
            m_ergGoalResistance = -1;
            m_ergSettleUntil = now + (m_ergFeedForwardRamp ? 4500 : 1800);
            m_ergFilteredPower = power;
            m_ergLastPowerSample = now;
            m_ergCorrectedResistance = static_cast<float>(current);
        } else if (now - m_ergLastWrite >= 334) {
            const int next = current + (m_ergGoalResistance > current ? 1 : -1);
            if (!forceResistance(next)) {
                emit debug(QStringLiteral("Peloton Bike+ ERG stopped: motor write failed"));
                resetErg();
                return;
            }
            m_ergExpectedResistance = next;
            m_ergWriteSince = now;
            m_ergLastWrite = now;
        }
        return;
    }

    const float dt = static_cast<float>(qBound<qint64>(0, now - m_ergLastPowerSample, 400)) / 1000.0f;
    m_ergLastPowerSample = now;
    if (now < m_ergSettleUntil || dt <= 0.0f)
        return;
    // Grupetto-style 2 s low-pass feedback, with a half-level deadband.
    m_ergFilteredPower += dt / (2.0f + dt) * (power - m_ergFilteredPower);
    const float error = static_cast<float>(m_ergTargetWatts) - m_ergFilteredPower;
    const float stepWatts = std::fabs(m_powerModel.predictedWatts(qMin(current + 1, 100), cadence) -
                                      m_powerModel.predictedWatts(current, cadence));
    const float deadband = qMax(3.0f, 0.5f * stepWatts);
    if (std::fabs(error) <= deadband) {
        m_ergCorrectedResistance = static_cast<float>(current);
        return;
    }

    // Proportional gain 0.007 per 100 ms from Grupetto. Limit the next
    // correction to one motor level, with an acknowledgement before continuing.
    m_ergCorrectedResistance = qBound(static_cast<float>(current - 1),
                                      m_ergCorrectedResistance + 0.007f * error * dt / 0.1f,
                                      static_cast<float>(current + 1));
    const int next = qBound(0, qRound(m_ergCorrectedResistance), 100);
    if (next != current) {
        m_ergGoalResistance = next;
        m_ergFeedForwardRamp = false;
    }
}

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

        if (requestResistance != -1) {
            // Explicit resistance control keeps its existing direct-write path.
            const int targetResistance = qBound(0, static_cast<int>(requestResistance), 100);
            if (targetResistance == qRound(Resistance.value()) ||
                forceResistance(targetResistance))
                requestResistance = -1;
        } else {
            updateErg(power, cadence, resistance);
        }
    } else if (m_ergActive) {
        emit debug(QStringLiteral("Peloton Bike+ ERG stopped: sensor disconnected"));
        resetErg();
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


