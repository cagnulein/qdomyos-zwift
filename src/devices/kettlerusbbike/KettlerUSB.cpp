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

#include <QRegularExpression>

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

bool KettlerUSB::initSequence() {
    // Preserve the known working E7 sequence, including its timing and flush.
    // The AX1 is detected from the status layout, not from the configured baud rate.
    statusCommand = QStringLiteral("ST");
    activeProtocol = Protocol::Unknown;
    extendedHandshakeAttempted = false;

    const QStringList commands = {"VE", "ID", "VE", "KI", "CA", "RS", "CM", "SP1"};
    for (const QString &command : commands) {
        if (sendCommand(command) < 0)
            return false;
        KettlerSleeper::msleep(150);
    }
    KettlerSleeper::msleep(500);
    flushSerialBuffer();
    return true;
}

bool KettlerUSB::tryExtendedHandshake() {
    // Only used when the legacy startup does not yield any valid status frame.
    // Do not disturb a working E7 or another legacy console.
    if (extendedHandshakeAttempted || activeProtocol != Protocol::Unknown)
        return false;
    extendedHandshakeAttempted = true;
    flushSerialBuffer();

    if (sendCommand(QStringLiteral("ID")) < 0)
        return false;
    const QString identifier = readResponse();
    qDebug() << "Kettler fallback ID:" << identifier;
    if (identifier.isEmpty())
        return false;

    for (const QString &command : {QStringLiteral("ST"), QStringLiteral("DB")}) {
        if (sendCommand(command) < 0)
            return false;
        for (int i = 0; i < 8; ++i) {
            const QString response = readResponse();
            if (response.isEmpty() || response == QStringLiteral("ERROR"))
                break;
            if (parseStatusResponse(response)) {
                statusCommand = command;
                return true;
            }
        }
    }
    statusCommand = QStringLiteral("ST");
    return false;
}

bool KettlerUSB::decodeStatusFrame(const QString &response, StatusFrame &frame) {
    if (response == QStringLiteral("ACK") || response == QStringLiteral("ERROR"))
        return false;

    const QStringList fields = response.split('\t', Qt::KeepEmptyParts);
    if (fields.size() < 8)
        return false;

    // The legacy protocol puts time at index 6 (and can append vendor
    // fields). AX1 moves time to index 7 and current power to index 8.
    // Detect by the time column, not just the number of columns.
    static const QRegularExpression timeExpression(QStringLiteral("^[0-9]{1,3}:[0-9]{2}$"));
    const bool legacy = timeExpression.match(fields[6]).hasMatch();
    const bool extended = fields.size() >= 9 && timeExpression.match(fields[7]).hasMatch();
    if (legacy == extended)
        return false;

    const int powerIndex = legacy ? 7 : 8;
    // Match the pre-AX1 legacy parser's tolerances: E7 consoles may emit
    // non-numeric distance (3) or energy (5) while the other fields are valid.
    // For AX1, validate only the fields required by its existing parser.
    const int legacyRequired[] = {0, 1, 2, 4, 7};
    const int extendedRequired[] = {1, 2, 3, 4, 6, 8};
    const int *required = legacy ? legacyRequired : extendedRequired;
    const int count = legacy ? 5 : 6;
    for (int i = 0; i < count; ++i) {
        bool ok = false;
        fields[required[i]].toDouble(&ok);
        if (!ok)
            return false;
    }

    frame.protocol = legacy ? Protocol::Legacy8 : Protocol::Extended9;
    frame.heartRate = fields[legacy ? 0 : 1].toDouble();
    frame.cadence = fields[legacy ? 1 : 2].toDouble();
    frame.speed = fields[legacy ? 2 : 3].toDouble() * 0.1;
    frame.distance = fields[legacy ? 3 : 4].toDouble() * 100.0;
    frame.power = fields[powerIndex].toDouble();
    return true;
}

bool KettlerUSB::isStatusResponse(const QString &response) {
    StatusFrame frame;
    return decodeStatusFrame(response, frame);
}

bool KettlerUSB::takeResponse(QByteArray &buffer, QString &response) {
    const int lineEnd = buffer.indexOf("\r\n");
    if (lineEnd < 0)
        return false;

    response = QString::fromLatin1(buffer.left(lineEnd)).trimmed();
    buffer.remove(0, lineEnd + 2);
    return true;
}

bool KettlerUSB::parseStatusResponse(const QString &response) {
    StatusFrame frame;
    if (!decodeStatusFrame(response, frame)) {
        qDebug() << "  Skipping non-status response:" << response;
        return false;
    }

    if (activeProtocol != frame.protocol) {
        activeProtocol = frame.protocol;
        qDebug() << "Kettler detected status protocol:"
                 << (activeProtocol == Protocol::Legacy8 ? "legacy 8-field (E7)" : "extended 9-field (AX1)");
    }

    pvars.lock();
    deviceHeartRate = frame.heartRate;
    deviceCadence = frame.cadence;
    deviceSpeed = frame.speed;
    deviceDistance = frame.distance;
    devicePower = frame.power;
    pvars.unlock();

    qDebug() << "Kettler Status: HR=" << frame.heartRate
             << " RPM=" << frame.cadence << " Speed=" << frame.speed
             << " Dist=" << frame.distance << " Power=" << frame.power;
    return true;
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
        pvars.lock();
        deviceStatus = 0;
        pvars.unlock();
        quit(2);
        return;
    } else {
        isDeviceOpen = true;
    }

    // Send initialization sequence
    if (!initSequence()) {
        pvars.lock();
        deviceStatus = 0;
        pvars.unlock();
        closePort();
        quit(2);
        return;
    }

    // Maintain the legacy console settle time before the first poll.
    KettlerSleeper::msleep(3000);
    int missedStatusPolls = 0;

    while (1) {
        if (isDeviceOpen) {
            pvars.lock();
            curWritePower = this->writePower;
            curPower = this->targetPower;
            pvars.unlock();

            if (curWritePower) {
                const QString powerCmd = QString("PW%1").arg(static_cast<int>(curPower));
                if (sendCommand(powerCmd) < 0) {
                    qDebug() << "Kettler power write failed";
                    pvars.lock();
                    deviceStatus = 0;
                    pvars.unlock();
                    closePort();
                    quit(2);
                    return;
                }
                pvars.lock();
                this->writePower = false;
                pvars.unlock();

                KettlerSleeper::msleep(150);
                const QString powerReply = readResponse();
                if (isStatusResponse(powerReply))
                    parseStatusResponse(powerReply);
                else
                    qDebug() << "Kettler PW response:" << powerReply;
            }

            if (sendCommand(statusCommand) < 0) {
                qDebug() << "Kettler status write failed";
                pvars.lock();
                deviceStatus = 0;
                pvars.unlock();
                closePort();
                quit(2);
                return;
            }

            bool receivedStatus = false;
            // E7 may have queued VE/ID/KI/CA/RS/CM/SP1 replies before ST.
            // Drain those lines rather than treating them as telemetry.
            for (int i = 0; i < 12; ++i) {
                const QString response = readResponse();
                if (response.isEmpty() || response == QStringLiteral("ERROR"))
                    break;
                if (parseStatusResponse(response)) {
                    receivedStatus = true;
                    break;
                }
            }

            if (receivedStatus) {
                missedStatusPolls = 0;
            } else if (activeProtocol == Protocol::Unknown && ++missedStatusPolls >= 3) {
                if (tryExtendedHandshake())
                    missedStatusPolls = 0;
            }
            // A single missing response is not a disconnect on legacy USB.
            KettlerSleeper::msleep(500);
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
                pvars.lock();
                deviceStatus = 0;
                pvars.unlock();
                quit(2);
                return;
            }
            isDeviceOpen = true;
            missedStatusPolls = 0;
            if (!initSequence()) {
                pvars.lock();
                deviceStatus = 0;
                pvars.unlock();
                closePort();
                quit(2);
                return;
            }
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
    QElapsedTimer timeout;
    timeout.start();

    while (timeout.elapsed() < KETTLER_READTIMEOUT) {
        QString response;
        if (takeResponse(rxBuffer, response))
            return response;

        const QString chunk = rawRead();
        if (!chunk.isEmpty()) {
            rxBuffer.append(chunk.toLatin1());
            // Android USB reads can block up to the configured timeout.
            // A complete line arriving on that read is still a valid reply.
            if (takeResponse(rxBuffer, response))
                return response;
        } else {
            KettlerSleeper::msleep(10);
        }
    }

    // Keep a partial CRLF frame for the next poll; bound stale input.
    if (rxBuffer.size() > 1024)
        rxBuffer.clear();
    return QString();
}

int KettlerUSB::closePort() {
#ifdef Q_OS_ANDROID
    rxBuffer.clear();
    return QAndroidJniObject::callStaticMethod<jboolean>("org/cagnulen/qdomyoszwift/Usbserial", "close", "()Z") ? 0 : -1;
#elif defined(WIN32)
    if (devicePort == INVALID_HANDLE_VALUE)
        return 0;
    const int result = (int)!CloseHandle(devicePort);
    devicePort = INVALID_HANDLE_VALUE;
    rxBuffer.clear();
    return result;
#else
    if (devicePort < 0)
        return 0;
    tcflush(devicePort, TCIOFLUSH);
    const int result = close(devicePort);
    devicePort = -1;
    rxBuffer.clear();
    return result;
#endif
}

int KettlerUSB::openPort() {
#ifdef Q_OS_ANDROID
    // Call Java Usbserial with configured baud rate for Kettler
    rxBuffer.clear();
    const jboolean opened = QAndroidJniObject::callStaticMethod<jboolean>("org/cagnulen/qdomyoszwift/Usbserial", "openKettler",
                                                                           "(Landroid/content/Context;I)Z",
                                                                           QtAndroid::androidContext().object(),
                                                                           baudrate);
    if (!opened)
        return -1;
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

    if (ioctl(devicePort, TIOCSETD, &ldisc) == -1) {
        const int error = errno;
        close(devicePort);
        devicePort = -1;
        return error;
    }

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

    if (tcsetattr(devicePort, TCSANOW, &deviceSettings) == -1) {
        const int error = errno;
        close(devicePort);
        devicePort = -1;
        return error;
    }
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

    if (GetCommState(devicePort, &deviceSettings) == false) {
        CloseHandle(devicePort);
        devicePort = INVALID_HANDLE_VALUE;
        return -1;
    }

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
        devicePort = INVALID_HANDLE_VALUE;
        return -1;
    }

    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 2000;
    timeouts.ReadTotalTimeoutMultiplier = 50;
    timeouts.WriteTotalTimeoutConstant = 2000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(devicePort, &timeouts);
#endif

    rxBuffer.clear();
    return 0;
}

int KettlerUSB::rawWrite(const char *bytes, int size) {
    qDebug() << "Kettler TX:" << QByteArray(bytes, size);

    int rc = 0;

#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    jbyteArray d = env->NewByteArray(size);
    if (!d)
        return -1;
    env->SetByteArrayRegion(d, 0, size, reinterpret_cast<const jbyte *>(bytes));
    rc = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Usbserial", "writeKettler", "([B)I", d);
    env->DeleteLocalRef(d);
#elif defined(WIN32)
    DWORD cBytes = 0;
    if (!WriteFile(devicePort, bytes, size, &cBytes, NULL) || cBytes != static_cast<DWORD>(size))
        return -1;
    return static_cast<int>(cBytes);
#else
    rc = write(devicePort, bytes, size);
    if (rc != size)
        return -1;
    if (tcdrain(devicePort) != 0)
        return -1;
    return rc;
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

        env->ReleaseByteArrayElements(d, b, JNI_ABORT);
        result = QString::fromLatin1(data);
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
        qDebug() << "Kettler RX bytes:" << result.toLatin1().toHex(' ');
    }

    return result;
}

void KettlerUSB::flushSerialBuffer() {
    qDebug() << "Flushing serial buffer...";
    // Clear accumulated fragments on every platform, before discarding input.
    rxBuffer.clear();
#ifdef Q_OS_ANDROID
    // Clear Android buffer
    // Read and discard any pending data
    QAndroidJniObject::callStaticObjectMethod("org/cagnulen/qdomyoszwift/Usbserial", "read", "()[B");
#elif defined(WIN32)
    PurgeComm(devicePort, PURGE_RXCLEAR | PURGE_TXCLEAR);
#else
    tcflush(devicePort, TCIOFLUSH);
#endif
}

bool KettlerUSB::discover(QString filename) {
    if (filename.isEmpty())
        return false;

    setDevice(filename);

    if (openPort())
        return false;

    // Try sending VE command and see if we get a response
    if (sendCommand("VE") < 0) {
        closePort();
        return false;
    }
    KettlerSleeper::msleep(500);
    QString response = readResponse();

    closePort();

    return !response.isEmpty();
}
