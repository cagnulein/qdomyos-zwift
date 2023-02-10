#include "fakebike.h"
#include "virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#include "keepawakehelper.h"
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
}

void fakebike::update() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    /*
    static int updcou = 0;
    updcou++;
    double w = 250.0;
    if (updcou > 20000 )
        updcou = 0;
    else if (updcou > 12000)
        w = 300;
    else if (updcou > 6000)
        w = 150;

    Speed = metric::calculateSpeedFromPower(w, Inclination.value(), Speed.value(),fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), speedLimit());*/

    update_metrics(true, watts());

    Distance += ((Speed.value() / (double)3600.0) /
                 ((double)1000.0 / (double)(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    // ******************************************* virtual bike init *************************************
    if (!this->isVirtualDeviceSetUp() && !virtualBike && !noVirtualDevice && !this->isPelotonWorkaroundActive()) {
        bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();

        if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual bike interface..."));
            virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
            connect(virtualBike, &virtualbike::changeInclination, this, &fakebike::changeInclinationRequested);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &fakebike::ftmsCharacteristicChanged);
        }
    }
    if (!this->isVirtualDeviceSetUp()) {
        // signal that it's connected and discovered because it's not a real device for which this happens
        // when a descriptor is written.
        emit connectedAndDiscovered();

        // flag that virtual device setup has been done
        this->setVirtualDeviceSetUp();
    }
    // ********************************************************************************************************

    if (!noVirtualDevice) {
#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
            Heart = (uint8_t)KeepAwakeHelper::heart();
            debug("Current Heart: " + QString::number(Heart.value()));
        }
#endif
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            this->updateLockscreenEnergyDistanceHeartRate();
        }

        this->doPelotonWorkaround();
    }

    if (Heart.value()) {
        KCal = metric::calculateKCalfromHR(Heart.average(), elapsed.value());
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

bool fakebike::connected() { return true; }

void *fakebike::VirtualBike() { return virtualBike; }

void *fakebike::VirtualDevice() { return VirtualBike(); }
