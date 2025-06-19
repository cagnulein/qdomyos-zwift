#include "nordictrackusbtreadmill.h"

using namespace std::chrono_literals;

nordictrackusbtreadmill::nordictrackusbtreadmill(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &nordictrackusbtreadmill::update);
    refresh->start(200ms);
    nordictrackusbtreadmillThread *t = new nordictrackusbtreadmillThread();
    t->start();
}

nordictrackusbtreadmillThread::nordictrackusbtreadmillThread() {}

void nordictrackusbtreadmillThread::run() {
    QSettings settings;
    /*devicePort =
        settings.value(QZSettings::computrainer_serialport, QZSettings::default_computrainer_serialport).toString();*/

    openPort();
    int countError = 0;
    while (1) {

        uint8_t command[64];
        memset(command, 0xFF, sizeof(command));
        if(countError == 0)
            rawWrite(command, sizeof(command));
        static uint8_t rx[100];
        memset(rx, 0x00, sizeof(rx));
        rawRead(rx, 100);
        qDebug() << " << " << QByteArray::fromRawData((const char *)rx, 64).toHex(' ');
        QThread::msleep(50);
        if(countError++>10)
            countError = 0;        
    }
    closePort();
}

int nordictrackusbtreadmillThread::closePort() {
#ifdef WIN32
    return (int)!CloseHandle(devicePort);
#else
    tcflush(devicePort, TCIOFLUSH); // clear out the garbage
    return close(devicePort);
#endif
}

int nordictrackusbtreadmillThread::openPort() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/NordictrackUSBHID", "open",
                                              "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
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
    // WINDOWS USES SET/GETCOMMSTATE AND READ/WRITEFILE

    COMMTIMEOUTS timeouts; // timeout settings on serial ports

    // if deviceFilename references a port above COM9
    // then we need to open "\\.\COMX" not "COMX"
    QString portSpec;
    int portnum = deviceFilename.midRef(3).toString().toInt();
    if (portnum < 10)
        portSpec = deviceFilename;
    else
        portSpec = "\\\\.\\" + deviceFilename;
    wchar_t deviceFilenameW[32]; // \\.\COM32 needs 9 characters, 32 should be enough?
    MultiByteToWideChar(CP_ACP, 0, portSpec.toLatin1(), -1, (LPWSTR)deviceFilenameW, sizeof(deviceFilenameW));

    // win32 commport API
    devicePort = CreateFile(deviceFilenameW, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (devicePort == INVALID_HANDLE_VALUE)
        return -1;

    if (GetCommState(devicePort, &deviceSettings) == false)
        return -1;

    // so we've opened the comm port lets set it up for
    deviceSettings.BaudRate = CBR_2400;
    deviceSettings.fParity = NOPARITY;
    deviceSettings.ByteSize = 8;
    deviceSettings.StopBits = ONESTOPBIT;
    deviceSettings.XonChar = 11;
    deviceSettings.XoffChar = 13;
    deviceSettings.EofChar = 0x0;
    deviceSettings.ErrorChar = 0x0;
    deviceSettings.EvtChar = 0x0;
    deviceSettings.fBinary = true;
    deviceSettings.fOutX = 0;
    deviceSettings.fInX = 0;
    deviceSettings.XonLim = 0;
    deviceSettings.XoffLim = 0;
    deviceSettings.fRtsControl = RTS_CONTROL_ENABLE;
    deviceSettings.fDtrControl = DTR_CONTROL_ENABLE;
    deviceSettings.fOutxCtsFlow = FALSE; // TRUE;

    if (SetCommState(devicePort, &deviceSettings) == false) {
        CloseHandle(devicePort);
        return -1;
    }

    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.ReadTotalTimeoutMultiplier = 50;
    timeouts.WriteTotalTimeoutConstant = 2000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(devicePort, &timeouts);

#endif

    // success
    return 0;
}

int nordictrackusbtreadmillThread::rawWrite(uint8_t *bytes, int size) // unix!!
{
    qDebug() << size << QByteArray((const char *)bytes, size).toHex(' ');

    int rc = 0;

#ifdef Q_OS_ANDROID

    QAndroidJniEnvironment env;
    jbyteArray d = env->NewByteArray(size);
    jbyte *b = env->GetByteArrayElements(d, 0);
    for (int i = 0; i < size; i++)
        b[i] = bytes[i];
    env->SetByteArrayRegion(d, 0, size, b);
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/NordictrackUSBHID", "write", "([B)V", d);
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

int nordictrackusbtreadmillThread::rawRead(uint8_t bytes[], int size) {
    int rc = 0;

#ifdef Q_OS_ANDROID
    int64_t start = QDateTime::currentMSecsSinceEpoch();
    jint len = 0;

    do {
        QAndroidJniEnvironment env;
        QAndroidJniObject dd =
            QAndroidJniObject::callStaticObjectMethod("org/cagnulen/qdomyoszwift/NordictrackUSBHID", "read", "()[B");
        len =
            QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/NordictrackUSBHID", "readLen", "()I");
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

void nordictrackusbtreadmill::update() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    // update_metrics(false, watts());

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
            if (virtual_device_enabled) {
            if (!virtual_device_rower) {
                qDebug() << QStringLiteral("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                // connect(virtualBike,&virtualbike::debug ,this,&echelonrower::debug);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
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
}

void nordictrackusbtreadmill::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                        const QByteArray &newValue) {
    QByteArray b = newValue;
    qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');
}

bool nordictrackusbtreadmill::connected() { return true; }

void nordictrackusbtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
}

void nordictrackusbtreadmill::newPacket(QByteArray p) {}
