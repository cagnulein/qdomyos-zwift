#include "csaferower.h"

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

using namespace std::chrono_literals;

csaferower::csaferower(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &csaferower::update);
    refresh->start(200ms);
    csaferowerThread *t = new csaferowerThread();
    connect(t, &csaferowerThread::onPower, this, &csaferower::onPower);
    connect(t, &csaferowerThread::onCadence, this, &csaferower::onCadence);
    connect(t, &csaferowerThread::onHeart, this, &csaferower::onHeart);
    connect(t, &csaferowerThread::onCalories, this, &csaferower::onCalories);
    connect(t, &csaferowerThread::onDistance, this, &csaferower::onDistance);
    connect(t, &csaferowerThread::onPace, this, &csaferower::onPace);
    connect(t, &csaferowerThread::onStatus, this, &csaferower::onStatus);
    t->start();
}

void csaferower::onPace(double pace) {
    qDebug() << "Current Pace received:" << pace;
    if(distanceIsChanging && pace > 0)
        Speed = (60.0 / (double)(pace)) * 60.0;
    else
        Speed = 0;

    qDebug() << "Current Speed calculated:" << Speed.value() << pace;
}


void csaferower::onPower(double power) {
    qDebug() << "Current Power received:" << power;
    if(distanceIsChanging)
        m_watt = power;
}

void csaferower::onCadence(double cadence) {
    qDebug() << "Current Cadence received:" << cadence;
    if(distanceIsChanging)
        Cadence = cadence;
}

void csaferower::onHeart(double hr) {
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

void csaferower::onCalories(double calories) {
    qDebug() << "Current Calories received:" << calories;
    KCal = calories;
}

void csaferower::onDistance(double distance) {
    qDebug() << "Current Distance received:" << distance / 1000.0;

    if(distance != distanceReceived.value()) {
        distanceIsChanging = true;
        distanceReceived = distance;
    } else if(abs(distanceReceived.lastChanged().secsTo(QDateTime::currentDateTime())) > 2) {
        distanceIsChanging = false;
        m_watt = 0;
        Cadence = 0;
        Speed = 0;
    }

}

void csaferower::onStatus(char status) {
    qDebug() << "Current Status received:" << status;
}

csaferowerThread::csaferowerThread() {}

void csaferowerThread::run() {
    QSettings settings;
    deviceFilename = settings.value(QZSettings::csafe_rower, QZSettings::default_csafe_rower).toString();

    openPort();
    csafe *aa = new csafe();
    while (1) {
        QStringList command;
        command << "CSAFE_PM_GET_WORKTIME";
        command << "CSAFE_PM_GET_WORKDISTANCE";
        command << "CSAFE_GETCADENCE_CMD";
        command << "CSAFE_GETPOWER_CMD";
        command << "CSAFE_GETCALORIES_CMD";
        command << "CSAFE_GETHRCUR_CMD";
        command << "CSAFE_GETPACE_CMD";
        command << "CSAFE_GETSTATUS_CMD";
        QByteArray ret = aa->write(command,true);

        qDebug() << " >> " << ret.toHex(' ');
        rawWrite((uint8_t *)ret.data(), ret.length());
        static uint8_t rx[100];
        rawRead(rx, 100);
        qDebug() << " << " << QByteArray::fromRawData((const char *)rx, 64).toHex(' ');

        QVector<quint8> v;
        for (int i = 0; i < 64; i++)
            v.append(rx[i]);
        QVariantMap f = aa->read(v);
        if (f["CSAFE_GETCADENCE_CMD"].isValid()) {
            emit onCadence(f["CSAFE_GETCADENCE_CMD"].value<QVariantList>()[0].toDouble());
        }
        if (f["CSAFE_GETPACE_CMD"].isValid()) {
            emit onPace(f["CSAFE_GETPACE_CMD"].value<QVariantList>()[0].toDouble());
        }
        if (f["CSAFE_GETPOWER_CMD"].isValid()) {
            emit onPower(f["CSAFE_GETPOWER_CMD"].value<QVariantList>()[0].toDouble());
        }
        if (f["CSAFE_GETHRCUR_CMD"].isValid()) {
            emit onHeart(f["CSAFE_GETHRCUR_CMD"].value<QVariantList>()[0].toDouble());
        }
        if (f["CSAFE_GETCALORIES_CMD"].isValid()) {
            emit onCalories(f["CSAFE_GETCALORIES_CMD"].value<QVariantList>()[0].toDouble());
        }
        if (f["CSAFE_PM_GET_WORKDISTANCE"].isValid()) {
            emit onDistance(f["CSAFE_PM_GET_WORKDISTANCE"].value<QVariantList>()[0].toDouble());
        }
        if (f["CSAFE_GETSTATUS_CMD"].isValid()) {
            char statusChar = static_cast<char>(f["CSAFE_GETSTATUS_CMD"].value<QVariantList>()[0].toUInt() & 0x0f);
            emit onStatus(statusChar);
        }

        memset(rx, 0x00, sizeof(rx));
        QThread::msleep(50);
    }
    closePort();
}

int csaferowerThread::closePort() {
#ifdef WIN32
    return (int)!CloseHandle(devicePort);
#else
    tcflush(devicePort, TCIOFLUSH); // clear out the garbage
    return close(devicePort);
#endif
}

int csaferowerThread::openPort() {

qDebug() << "Opening serial port " << deviceFilename.toLatin1();

#ifdef Q_OS_ANDROID
    QJniObject context = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
    QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/CSafeRowerUSBHID", "open",
                                              "(Landroid/content/Context;)V", context.object());
#elif !defined(WIN32)

    // LINUX AND MAC USES TERMIO / IOCTL / STDIO

#if defined(Q_OS_MACX)
    int ldisc = TTYDISC;
#else
    int ldisc = N_TTY;                    // LINUX
#endif

    if ((devicePort = open(deviceFilename.toLatin1(), O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1)
        return errno;

    tcflush(devicePort, TCIOFLUSH); // clear out the garbage

    if (ioctl(devicePort, TIOCSETD, &ldisc) == -1)
        return errno;

    // get current settings for the port
    tcgetattr(devicePort, &deviceSettings);

    // set raw mode i.e. ignbrk, brkint, parmrk, istrip, inlcr, igncr, icrnl, ixon
    //                   noopost, cs8, noecho, noechonl, noicanon, noisig, noiexn
    cfmakeraw(&deviceSettings);
    cfsetspeed(&deviceSettings, B2400);

    // further attributes
    deviceSettings.c_iflag &=
        ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ICANON | ISTRIP | IXON | IXOFF | IXANY);
    deviceSettings.c_iflag |= IGNPAR;
    deviceSettings.c_cflag &= (~CSIZE & ~CSTOPB);
    deviceSettings.c_oflag = 0;

#if defined(Q_OS_MACX)
    deviceSettings.c_cflag &= (~CCTS_OFLOW & ~CRTS_IFLOW); // no hardware flow control
    deviceSettings.c_cflag |= (CS8 | CLOCAL | CREAD | HUPCL);
#else
    deviceSettings.c_cflag &= (~CRTSCTS); // no hardware flow control
    deviceSettings.c_cflag |= (CS8 | CLOCAL | CREAD | HUPCL);
#endif
    deviceSettings.c_lflag = 0;
    deviceSettings.c_cc[VSTART] = 0x11;
    deviceSettings.c_cc[VSTOP] = 0x13;
    deviceSettings.c_cc[VEOF] = 0x20;
    deviceSettings.c_cc[VMIN] = 0;
    deviceSettings.c_cc[VTIME] = 0;

    // set those attributes
    if (tcsetattr(devicePort, TCSANOW, &deviceSettings) == -1)
        return errno;
    tcgetattr(devicePort, &deviceSettings);

    tcflush(devicePort, TCIOFLUSH); // clear out the garbage
#else


#endif

    // success
    return 0;
}

int csaferowerThread::rawWrite(uint8_t *bytes, int size) // unix!!
{
    qDebug() << size << QByteArray((const char *)bytes, size).toHex(' ');

    int rc = 0;

#ifdef Q_OS_ANDROID

    QJniEnvironment env;
    jbyteArray d = env->NewByteArray(size);
    jbyte *b = env->GetByteArrayElements(d, 0);
    for (int i = 0; i < size; i++)
        b[i] = bytes[i];
    env->SetByteArrayRegion(d, 0, size, b);
    QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/CSafeRowerUSBHID", "write", "([B)V", d);
#elif defined(WIN32)
    DWORD cBytes;
    rc = WriteFile(devicePort, bytes, size, &cBytes, NULL);
    if (!rc)
        return -1;
    return rc;

#else
    int ibytes;
    ioctl(devicePort, FIONREAD, &ibytes);

    // timeouts are less critical for writing, since vols are low
    rc = write(devicePort, bytes, size);

    // but it is good to avoid buffer overflow since the
    // computrainer microcontroller has almost no RAM
    if (rc != -1)
        tcdrain(devicePort); // wait till its gone.

    ioctl(devicePort, FIONREAD, &ibytes);
#endif

    return rc;
}

int csaferowerThread::rawRead(uint8_t bytes[], int size) {
    int rc = 0;

#ifdef Q_OS_ANDROID
    int64_t start = QDateTime::currentMSecsSinceEpoch();
    jint len = 0;

    do {
        QJniEnvironment env;
        QJniObject dd =
            QJniObject::callStaticObjectMethod("org/cagnulen/qdomyoszwift/CSafeRowerUSBHID", "read", "()[B");
        len = QJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/CSafeRowerUSBHID", "readLen", "()I");
        if (len > 0) {
            jbyteArray d = dd.object<jbyteArray>();
            jbyte *b = env->GetByteArrayElements(d, 0);
            for (int i = 0; i < len; i++) {
                bytes[i] = b[i];
            }
            qDebug() << len << QByteArray((const char *)b, len).toHex(' ');
        }
    } while (len == 0 && start + 2000 > QDateTime::currentMSecsSinceEpoch());

    return len;
#elif defined(WIN32)
    Q_UNUSED(size);
    // Readfile deals with timeouts and readyread issues
    DWORD cBytes;
    rc = ReadFile(devicePort, bytes, 7, &cBytes, NULL);
    if (rc)
        return (int)cBytes;
    else
        return (-1);

#else

    int timeout = 0, i = 0;
    uint8_t byte;

    // read one byte at a time sleeping when no data ready
    // until we timeout waiting then return error
    for (i = 0; i < size; i++) {
        timeout = 0;
        rc = 0;
        while (rc == 0 && timeout < CT_READTIMEOUT) {
            rc = read(devicePort, &byte, 1);
            if (rc == -1)
                return -1; // error!
            else if (rc == 0) {
                msleep(50); // sleep for 1/20th of a second
                timeout += 50;
            } else {
                bytes[i] = byte;
            }
        }
        if (timeout >= CT_READTIMEOUT)
            return -1; // we timed out!
    }

    qDebug() << i << QString::fromLocal8Bit((const char *)bytes, i);

    return i;

#endif
}

void csaferower::update() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    update_metrics(false, watts());

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    Distance += ((Speed.value() / (double)3600.0) /
                 ((double)1000.0 / (double)(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    // ******************************************* virtual bike/rower init *************************************
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
        bool virtual_device_rower =
            settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
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
        {
            if (virtual_device_enabled) {
                if (!virtual_device_rower) {
                    qDebug() << QStringLiteral("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                    // connect(virtualBike,&virtualbike::debug ,this,&echelonrower::debug);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    qDebug() << QStringLiteral("creating virtual rower interface...");
                    auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                    // connect(virtualRower,&virtualrower::debug ,this,&echelonrower::debug);
                    this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::PRIMARY);
                }
            }
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

    /*
    if (Heart.value()) {
        static double lastKcal = 0;
        if (KCal.value() < 0) // if the user pressed stop, the KCAL resets the accumulator
            lastKcal = abs(KCal.value());
        KCal = metric::calculateKCalfromHR(Heart.average(), elapsed.value()) + lastKcal;
    }*/

    if (requestResistance != -1 && requestResistance != currentResistance().value()) {
        Resistance = requestResistance;
    }
}

void csaferower::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QByteArray b = newValue;
    qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');
}

bool csaferower::connected() { return true; }

void csaferower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

void csaferower::newPacket(QByteArray p) {}

uint16_t csaferower::watts() { return m_watt.value(); }
