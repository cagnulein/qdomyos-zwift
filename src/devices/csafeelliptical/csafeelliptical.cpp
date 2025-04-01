#include "csafeelliptical.h"

using namespace std::chrono_literals;

csafeelliptical::csafeelliptical(bool noWriteResistance, bool noHeartService, bool noVirtualDevice,
                                 int8_t bikeResistanceOffset, double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    connect(refresh, &QTimer::timeout, this, &csafeelliptical::update);
    refresh->start(200ms);
    QString deviceFilename =
        settings.value(QZSettings::csafe_elliptical_port, QZSettings::default_csafe_elliptical_port).toString();
    CsafeRunnerThread *csafeRunner = new CsafeRunnerThread(deviceFilename);
    setupCommands(csafeRunner);
    connect(csafeRunner, &CsafeRunnerThread::portAvailable, this, &csafeelliptical::portAvailable);
    connect(csafeRunner, &CsafeRunnerThread::onCsafeFrame, this, &csafeelliptical::onCsafeFrame);
    connect(this, &csafeelliptical::sendCsafeCommand, csafeRunner, &CsafeRunnerThread::sendCommand,
            Qt::QueuedConnection);
    csafeRunner->start();
    distanceReceived = 0;
    kalman = new KalmanFilter(1, .01, .75, 0); // measure error, estimate error, process noise q, initial value
}

void csafeelliptical::setupCommands(CsafeRunnerThread *runner) {
    QStringList command;
    command << "CSAFE_GETPOWER_CMD";
    command << "CSAFE_GETSPEED_CMD";
    command << "CSAFE_GETCALORIES_CMD";
    command << "CSAFE_GETHRCUR_CMD";
    command << "CSAFE_GETHORIZONTAL_CMD";
    runner->addRefreshCommand(command);
    runner->addRefreshCommand(QStringList() << "CSAFE_LF_GET_DETAIL");
    runner->addRefreshCommand(QStringList() << "CSAFE_GETPROGRAM_CMD");
}

void csafeelliptical::setupWorkout() {
    emit sendCsafeCommand(QStringList() << "CSAFE_GETUSERINFO_CMD");
    QStringList command = {"CSAFE_SETUSERINFO_CMD",
                           QString::number(settings.value(QZSettings::weight, QZSettings::default_weight).toInt()),
                           "39", QString::number(settings.value(QZSettings::age, QZSettings::default_age).toInt()),
                           "1"}; // weight,weight unit,age,gender
    emit sendCsafeCommand(command);
    emit sendCsafeCommand(QStringList() << "CSAFE_SETPROGRAM_CMD" << "4" << "5");
    emit sendCsafeCommand(QStringList() << "CSAFE_GOINUSE_CMD");
}

void csafeelliptical::onCsafeFrame(const QVariantMap &csafeFrame) {
    // qDebug() << "Current CSAFE frame received:" << csafeFrame;

    if (csafeFrame["CSAFE_GETCADENCE_CMD"].isValid()) {
        onCadence(csafeFrame["CSAFE_GETCADENCE_CMD"].value<QVariantList>()[0].toDouble());
    }
    if (csafeFrame["CSAFE_GETSPEED_CMD"].isValid()) {
        double speed = csafeFrame["CSAFE_GETSPEED_CMD"].value<QVariantList>()[0].toDouble();
        int unit = csafeFrame["CSAFE_GETSPEED_CMD"].value<QVariantList>()[1].toInt();
        qDebug() << "Speed value:" << speed << "unit:" << CSafeUtility::getUnitName(unit) << "(" << unit << ")";

        if (unit == 82) { // revs/minute
            onCadence(speed);
        } else {
            onSpeed(CSafeUtility::convertToStandard(unit, speed));
        }
    }
    if (csafeFrame["CSAFE_GETPOWER_CMD"].isValid()) {
        onPower(csafeFrame["CSAFE_GETPOWER_CMD"].value<QVariantList>()[0].toDouble());
    }
    if (csafeFrame["CSAFE_GETHRCUR_CMD"].isValid()) {
        onHeart(csafeFrame["CSAFE_GETHRCUR_CMD"].value<QVariantList>()[0].toDouble());
    }
    if (csafeFrame["CSAFE_GETCALORIES_CMD"].isValid()) {
        onCalories(csafeFrame["CSAFE_GETCALORIES_CMD"].value<QVariantList>()[0].toDouble());
    }
    if (csafeFrame["CSAFE_GETHORIZONTAL_CMD"].isValid()) {
        double distance = csafeFrame["CSAFE_GETHORIZONTAL_CMD"].value<QVariantList>()[0].toDouble();
        int unit = csafeFrame["CSAFE_GETHORIZONTAL_CMD"].value<QVariantList>()[1].toInt();
        qDebug() << "Distance value:" << distance << "unit:" << CSafeUtility::getUnitName(unit) << "(" << unit << ")"
                 << CSafeUtility::convertToStandard(unit, distance);
        onDistance(CSafeUtility::convertToStandard(unit, distance));
    }
    if (csafeFrame["CSAFE_GETPROGRAM_CMD"].isValid()) {
        int resistance = csafeFrame["CSAFE_GETPROGRAM_CMD"].value<QVariantList>()[1].toUInt();
        Resistance = resistance;
        qDebug() << "Program:" << csafeFrame["CSAFE_GETPROGRAM_CMD"].value<QVariantList>()[0].toUInt()
                 << "Current level received:" << resistance;
    }
    if (csafeFrame["CSAFE_GETSTATUS_CMD"].isValid()) {
        uint16_t statusvalue = csafeFrame["CSAFE_GETSTATUS_CMD"].value<QVariantList>()[0].toUInt();
        qDebug() << "Status value:" << statusvalue << " lastStatus:" << lastStatus
                 << " Machine state from status:" << (statusvalue & 0x0f);
        if (statusvalue != lastStatus) {
            lastStatus = statusvalue;
            char statusChar = static_cast<char>(statusvalue & 0x0f);
            onStatus(statusChar);
        }
    }
}

void csafeelliptical::onSpeed(double speed) {
    qDebug() << "Current Speed received:" << speed << " updated:" << distanceIsChanging;
    if (distanceIsChanging)
        Speed = speed;
}

void csafeelliptical::onPower(double power) {
    qDebug() << "Current Power received:" << power << " updated:" << distanceIsChanging;
    if (distanceIsChanging)
        m_watt = power;
}

void csafeelliptical::onCadence(double cadence) {
    qDebug() << "Current Cadence received:" << cadence << " updated:" << distanceIsChanging;
    if (distanceIsChanging)
        Cadence = cadence;
}

void csafeelliptical::onHeart(double hr) {
    qDebug() << "Current Heart received:" << hr;
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            uint8_t heart = ((uint8_t)hr);
            if (heart == 0 || disable_hr_frommachinery) {
                update_hr_from_external();
            } else
                Heart = heart;
        }
    }
}

void csafeelliptical::onCalories(double calories) {
    qDebug() << "Current Calories received:" << calories;
    KCal = calories;
}

void csafeelliptical::onDistance(double distance) {
    qDebug() << "Current Distance received:" << distance << " value tracker:" << distanceReceived.value();
    Distance = distance / 1000.0;

    if (distance != distanceReceived.value()) {

        double calculated_speed = 3600 * (distance - distanceReceived.value()) /
                                  abs(distanceReceived.lastChanged().msecsTo(QDateTime::currentDateTime()));
        if (distanceIsChanging) { // skip the first distance or after pause otherwise you get enormous speed values
            // Speed is calculated from the distance and is bit erratic. The kalman filter is used to smooth it out
            Speed = kalman->updateEstimate(calculated_speed);
        }

        qDebug() << "Distance received:" << distance << " Previous:" << distanceReceived.value()
                 << " time(ms):" << abs(distanceReceived.lastChanged().msecsTo(QDateTime::currentDateTime()))
                 << " Calculated Speed:" << calculated_speed << " New speed:" << Speed.value()
                 << " updated:" << distanceIsChanging;
        distanceReceived = distance;
        distanceIsChanging = true;
    } else if (abs(distanceReceived.lastChanged().secsTo(QDateTime::currentDateTime())) > 15) {
        distanceIsChanging = false;
        m_watt = 0.0;
        Cadence = 0.0;
        Speed = 0.0;
    }
}

void csafeelliptical::onStatus(char status) {
    QString statusString = CSafeUtility::statusByteToText(status);
    qDebug() << "Current Status code:" << status << " status: " << statusString;
    if (status == 0x06) {
        // pause
        paused = true;
        m_watt = 0.0;
        Cadence = 0.0;
        Speed = 0.0;
        distanceIsChanging = false;
    } else {
        paused = false;
    }
}

void csafeelliptical::portAvailable(bool available) {
    if (available) {
        qDebug() << "CSAFE port available";
        _connected = true;
        setupWorkout();
    } else {
        qDebug() << "CSAFE port not available";
        _connected = false;
    }
}

void csafeelliptical::update() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    update_metrics(true, watts());
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {
        QSettings settings;
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();

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
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
        if (virtual_device_enabled) {
            if (!virtual_device_force_bike) {
                debug("creating virtual treadmill interface...");
                auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &csafeelliptical::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &csafeelliptical::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                debug("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this);
                //        auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &csafeelliptical::changeInclinationRequested);
                connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this,
                        &csafeelliptical::ftmsCharacteristicChanged);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
        } else {
            debug("not creating virtual interface... not enabled");
        }
        // ********************************************************************************************************
    }

    if (!firstStateChanged) {
        emit connectedAndDiscovered();
    }

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

    if (requestResistance != -1 && requestResistance != currentResistance().value()) {
        if (!noWriteResistance) {
            changeResistance(requestResistance);
        }
    }
}

void csafeelliptical::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    QByteArray b = newValue;
    qDebug() << "Routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');
}

void csafeelliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
    qDebug() << "Requested grade:" << grade << " percentage:" << percentage;
}

void csafeelliptical::changeResistance(resistance_t res) {
    if (res < 0)
        res = 0;
    if (res > 25)
        res = 25;
    QStringList resistanceCommand = {"CSAFE_SETLEVEL", QString::number(res)};
    emit sendCsafeCommand(resistanceCommand);
    qDebug() << "Send resistance update to device. Requested resitance: " << res;
    emit sendCsafeCommand(QStringList() << "CSAFE_GETPROGRAM_CMD");
}

bool csafeelliptical::connected() { return _connected; }

void csafeelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

uint16_t csafeelliptical::watts() { return m_watt.value(); }
