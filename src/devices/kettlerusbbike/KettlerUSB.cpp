/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KettlerUSB.h"

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

/* ----------------------------------------------------------------------
 * CONSTRUCTOR/DESTRUCTOR
 * ---------------------------------------------------------------------- */
KettlerUSB::KettlerUSB(QObject *parent, QString devname, int baudrate) : QThread(parent) {
    devicePower = deviceHeartRate = deviceCadence = deviceSpeed = deviceDistance = 0.00;
    targetPower = DEFAULT_POWER;
    writePower = false;
    setDevice(devname);
    deviceStatus = 0;
    this->parent = parent;
    this->baudrate = baudrate;
}

KettlerUSB::~KettlerUSB() {}

/* ----------------------------------------------------------------------
 * SET
 * ---------------------------------------------------------------------- */
void KettlerUSB::setDevice(QString devname) {
    deviceFilename = devname;
}

void KettlerUSB::setPower(double power) {
    pvars.lock();
    if (power < 0)
        power = 0;
    this->targetPower = power;
    this->writePower = true;
    pvars.unlock();
}

/* ----------------------------------------------------------------------
 * GET
 * ---------------------------------------------------------------------- */
void KettlerUSB::getTelemetry(double &power, double &heartrate, double &cadence,
                              double &speed, double &distance, int &status) {
    pvars.lock();
    power = devicePower;
    heartrate = deviceHeartRate;
    cadence = deviceCadence;
    speed = deviceSpeed;
    distance = deviceDistance;
    status = deviceStatus;
    pvars.unlock();
}

/* ----------------------------------------------------------------------
 * PROTOCOL FUNCTIONS
 * ---------------------------------------------------------------------- */

void KettlerUSB::initSequence() {
    // Kettler initialization sequence from kettlerUSB2BLE
    // Commands: VE, ID, VE, KI, CA, RS, CM, SP1
    QStringList initCommands = {"VE", "ID", "VE", "KI", "CA", "RS", "CM", "SP1"};

    for (const QString &cmd : initCommands) {
        sendCommand(cmd);
        KettlerSleeper::msleep(150); // 150ms between commands
    }
}

void KettlerUSB::parseStatusResponse(const QString &response) {
    // Status response format (tab-separated):
    // HR RPM Speed Distance Power Energy Time CurrentPower
    // Example: 101\t047\t074\t002\t025\t0312\t01:12\t025

    QStringList fields = response.split('\t');
    if (fields.size() >= 8) {
        pvars.lock();
        deviceHeartRate = fields[0].toDouble();
        deviceCadence = fields[1].toDouble();
        deviceSpeed = fields[2].toDouble() * 0.1; // Speed is in 0.1 km/h
        deviceDistance = fields[3].toDouble() * 100; // Distance is in 100m units
        devicePower = fields[7].toDouble(); // CurrentPower (last field)
        pvars.unlock();

        qDebug() << "Kettler Status: HR=" << deviceHeartRate
                 << " RPM=" << deviceCadence
                 << " Speed=" << deviceSpeed
                 << " Dist=" << deviceDistance
                 << " Power=" << devicePower;
    }
}

/* ----------------------------------------------------------------------
 * EXECUTIVE FUNCTIONS
 * ---------------------------------------------------------------------- */
int KettlerUSB::start() {
    QThread::start();
    return 0;
}

int KettlerUSB::restart() {
    int status;
    pvars.lock();
    status = this->deviceStatus;
    pvars.unlock();

    if (status & KETTLER_RUNNING && status & KETTLER_PAUSED) {
        status &= ~KETTLER_PAUSED;
        pvars.lock();
        this->deviceStatus = status;
        pvars.unlock();
        return 0;
    }
    return 2;
}

int KettlerUSB::stop() {
    pvars.lock();
    deviceStatus = 0;
    pvars.unlock();
    return 0;
}

int KettlerUSB::pause() {
    int status;
    pvars.lock();
    status = this->deviceStatus;
    pvars.unlock();

    if (status & KETTLER_PAUSED)
        return 2;
    else if (!(status & KETTLER_RUNNING))
        return 4;
    else {
        status |= KETTLER_PAUSED;
        pvars.lock();
        this->deviceStatus = status;
        pvars.unlock();
        return 0;
    }
}

int KettlerUSB::quit(int code) {
    exit(code);
    return 0;
}

/* ----------------------------------------------------------------------
 * THREADED CODE - MAIN RUN LOOP
 * ---------------------------------------------------------------------- */
void KettlerUSB::run() {
    int curstatus;
    double curPower = targetPower;
    bool curWritePower = false;
    bool isDeviceOpen = false;

    // Initialize
    pvars.lock();
    this->deviceStatus = KETTLER_RUNNING;
    this->devicePower = 0;
    this->deviceHeartRate = 0;
    this->deviceCadence = 0;
    this->deviceSpeed = 0;
    this->deviceDistance = 0;
    pvars.unlock();

    // Open the device
    if (openPort()) {
        quit(2);
        return;
    } else {
        isDeviceOpen = true;
    }

    // Send initialization sequence
    initSequence();

    // Wait 3 seconds before first poll (as per kettlerUSB2BLE)
    KettlerSleeper::msleep(3000);

    while (1) {
        if (isDeviceOpen) {
            // Check if we need to write power
            pvars.lock();
            curWritePower = this->writePower;
            curPower = this->targetPower;
            pvars.unlock();

            if (curWritePower) {
                // Send power command instead of status request
                QString powerCmd = QString("PW%1").arg((int)curPower);
                sendCommand(powerCmd);

                pvars.lock();
                this->writePower = false;
                pvars.unlock();
            } else {
                // Normal status poll
                sendCommand("ST");
                QString response = readResponse();
                if (!response.isEmpty()) {
                    parseStatusResponse(response);
                }
            }

            // Wait 2 seconds between polls (as per kettlerUSB2BLE)
            KettlerSleeper::msleep(2000);
        }

        // Check status
        pvars.lock();
        curstatus = this->deviceStatus;
        pvars.unlock();

        if (!(curstatus & KETTLER_RUNNING)) {
            closePort();
            quit(0);
            return;
        }

        if ((curstatus & KETTLER_PAUSED) && isDeviceOpen) {
            closePort();
            isDeviceOpen = false;
        } else if (!(curstatus & KETTLER_PAUSED) && (curstatus & KETTLER_RUNNING) && !isDeviceOpen) {
            if (openPort()) {
                quit(2);
                return;
            }
            isDeviceOpen = true;
            initSequence();
            KettlerSleeper::msleep(3000);
        }
    }
}

void KettlerSleeper::msleep(unsigned long msecs) {
    QThread::msleep(msecs);
}

/* ----------------------------------------------------------------------
 * LOW LEVEL DEVICE IO ROUTINES
 * ---------------------------------------------------------------------- */

int KettlerUSB::sendCommand(const QString &command) {
    QString fullCommand = command + "\r\n";
    QByteArray data = fullCommand.toLatin1();
    return rawWrite(data.constData(), data.size());
}

QString KettlerUSB::readResponse() {
    QString response = rawRead();
    return response.trimmed();
}

int KettlerUSB::closePort() {
#ifdef WIN32
    return (int)!CloseHandle(devicePort);
#else
    tcflush(devicePort, TCIOFLUSH);
    return close(devicePort);
#endif
}

int KettlerUSB::openPort() {
#ifdef Q_OS_ANDROID
    // Call Java Usbserial with configured baud rate for Kettler
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "open",
                                              "(Landroid/content/Context;I)V",
                                              QtAndroid::androidContext().object(),
                                              baudrate);
#elif !defined(WIN32)
    // LINUX AND MAC USES TERMIO / IOCTL / STDIO

#if defined(Q_OS_MACX)
    int ldisc = TTYDISC;
#else
    int ldisc = N_TTY;
#endif

    if ((devicePort = open(deviceFilename.toLatin1(), O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1)
        return errno;

    tcflush(devicePort, TCIOFLUSH);

    if (ioctl(devicePort, TIOCSETD, &ldisc) == -1)
        return errno;

    tcgetattr(devicePort, &deviceSettings);

    cfmakeraw(&deviceSettings);
    // Convert int baudrate to speed_t constant
    speed_t speed;
    switch (baudrate) {
        case 57600: speed = B57600; break;
        case 9600:
        default: speed = B9600; break;
    }
    cfsetspeed(&deviceSettings, speed);

    deviceSettings.c_iflag &=
        ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ICANON | ISTRIP | IXON | IXOFF | IXANY);
    deviceSettings.c_iflag |= IGNPAR;
    deviceSettings.c_cflag &= (~CSIZE & ~CSTOPB);
    deviceSettings.c_oflag = 0;

#if defined(Q_OS_MACX)
    deviceSettings.c_cflag &= (~CCTS_OFLOW & ~CRTS_IFLOW);
    deviceSettings.c_cflag |= (CS8 | CLOCAL | CREAD | HUPCL);
#else
    deviceSettings.c_cflag &= (~CRTSCTS);
    deviceSettings.c_cflag |= (CS8 | CLOCAL | CREAD | HUPCL);
#endif
    deviceSettings.c_lflag = 0;
    deviceSettings.c_cc[VMIN] = 0;
    deviceSettings.c_cc[VTIME] = 10; // 1 second timeout

    if (tcsetattr(devicePort, TCSANOW, &deviceSettings) == -1)
        return errno;
    tcgetattr(devicePort, &deviceSettings);

    tcflush(devicePort, TCIOFLUSH);
#else
    // WINDOWS IMPLEMENTATION
    COMMTIMEOUTS timeouts;
    QString portSpec;
    int portnum = deviceFilename.midRef(3).toString().toInt();
    if (portnum < 10)
        portSpec = deviceFilename;
    else
        portSpec = "\\\\.\\" + deviceFilename;

    wchar_t deviceFilenameW[32];
    MultiByteToWideChar(CP_ACP, 0, portSpec.toLatin1(), -1, (LPWSTR)deviceFilenameW, sizeof(deviceFilenameW));

    devicePort = CreateFile(deviceFilenameW, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (devicePort == INVALID_HANDLE_VALUE)
        return -1;

    if (GetCommState(devicePort, &deviceSettings) == false)
        return -1;

    // Convert int baudrate to Windows CBR constant
    switch (baudrate) {
        case 57600: deviceSettings.BaudRate = CBR_57600; break;
        case 9600:
        default: deviceSettings.BaudRate = CBR_9600; break;
    }
    deviceSettings.fParity = NOPARITY;
    deviceSettings.ByteSize = 8;
    deviceSettings.StopBits = ONESTOPBIT;
    deviceSettings.fBinary = true;
    deviceSettings.fOutX = 0;
    deviceSettings.fInX = 0;
    deviceSettings.fRtsControl = RTS_CONTROL_ENABLE;
    deviceSettings.fDtrControl = DTR_CONTROL_ENABLE;
    deviceSettings.fOutxCtsFlow = FALSE;

    if (SetCommState(devicePort, &deviceSettings) == false) {
        CloseHandle(devicePort);
        return -1;
    }

    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 2000;
    timeouts.ReadTotalTimeoutMultiplier = 50;
    timeouts.WriteTotalTimeoutConstant = 2000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(devicePort, &timeouts);
#endif

    return 0;
}

int KettlerUSB::rawWrite(const char *bytes, int size) {
    qDebug() << "Kettler TX:" << QByteArray(bytes, size);

    int rc = 0;

#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    jbyteArray d = env->NewByteArray(size);
    jbyte *b = env->GetByteArrayElements(d, 0);
    for (int i = 0; i < size; i++)
        b[i] = bytes[i];
    env->SetByteArrayRegion(d, 0, size, b);
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "write", "([B)V", d);
    rc = size;
#elif defined(WIN32)
    DWORD cBytes;
    rc = WriteFile(devicePort, bytes, size, &cBytes, NULL);
    if (!rc)
        return -1;
    return rc;
#else
    rc = write(devicePort, bytes, size);
    if (rc != -1)
        tcdrain(devicePort);
#endif

    return rc;
}

QString KettlerUSB::rawRead() {
    QString result;

#ifdef Q_OS_ANDROID
    QAndroidJniObject dd =
        QAndroidJniObject::callStaticObjectMethod("org/cagnulen/qdomyoszwift/Usbserial", "read", "()[B");
    jint len = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Usbserial", "readLen", "()I");

    if (len > 0) {
        QAndroidJniEnvironment env;
        jbyteArray d = dd.object<jbyteArray>();
        jbyte *b = env->GetByteArrayElements(d, 0);

        QByteArray data;
        for (int i = 0; i < len; i++) {
            data.append(b[i]);
        }

        rxBuffer.append(data);

        // Look for complete line (ends with \r\n)
        int lineEnd = rxBuffer.indexOf("\r\n");
        if (lineEnd >= 0) {
            result = QString::fromLatin1(rxBuffer.left(lineEnd));
            rxBuffer.remove(0, lineEnd + 2);
        }
    }
#elif defined(WIN32)
    char buffer[256];
    DWORD cBytes;
    if (ReadFile(devicePort, buffer, sizeof(buffer) - 1, &cBytes, NULL) && cBytes > 0) {
        buffer[cBytes] = '\0';
        result = QString::fromLatin1(buffer);
    }
#else
    char buffer[256];
    int rc = read(devicePort, buffer, sizeof(buffer) - 1);
    if (rc > 0) {
        buffer[rc] = '\0';
        result = QString::fromLatin1(buffer);
    }
#endif

    if (!result.isEmpty()) {
        qDebug() << "Kettler RX:" << result;
    }

    return result;
}

bool KettlerUSB::discover(QString filename) {
    if (filename.isEmpty())
        return false;

    setDevice(filename);

    if (openPort())
        return false;

    // Try sending VE command and see if we get a response
    sendCommand("VE");
    KettlerSleeper::msleep(500);
    QString response = readResponse();

    closePort();

    return !response.isEmpty();
}
