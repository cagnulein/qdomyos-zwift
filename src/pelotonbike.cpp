#include "pelotonbike.h"
#include "homeform.h"
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

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !virtualBike) {
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        if (virtual_device_enabled) {
            debug("creating virtual bike interface...");
            virtualBike = new virtualbike(this);
            connect(virtualBike, &virtualbike::changeInclination, this, &pelotonbike::changeInclinationRequested);
            firstStateChanged = 1;
        }
    }
    // ********************************************************************************************************
}

bool pelotonbike::inclinationAvailableByHardware() { return true; }

void pelotonbike::forceResistance(double resistance) {}

void pelotonbike::update() {

    QSettings settings;
    update_metrics(false, 0);

    if(!initDone) {
        initDone = true;
        emit connectedAndDiscovered();
    }

#ifdef Q_OS_ANDROID
    QAndroidJniObject text = QAndroidJniObject::callStaticObjectMethod<jstring>(
        "org/cagnulen/qdomyoszwift/ScreenCaptureService", "getLastText");
    QString t = text.toString();
    qDebug() << "OCR" << t;
#endif

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
            this->updateLockscreenEnergyDistanceHeartRate();
        }
    }

    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
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

bool pelotonbike::connected() { return true; }

void *pelotonbike::VirtualDevice() { return virtualBike; }
