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

#include "FreebeatUSB.h"

/* ----------------------------------------------------------------------
 * CONSTRUCTOR/DESTRUCTOR
 * ---------------------------------------------------------------------- */
FreebeatUSB::FreebeatUSB(QObject *parent, QString deviceFilename, int baudrate) : QThread(parent) {
    this->deviceFilename = deviceFilename;
    this->baudrate = baudrate;
    targetResistance = 1;
    writeResistance = false;
    doStop = false;
    doReset = false;
    doQuery = true;
    devRpm = 0;
    devResistance = 0;
    devSpeed = 0.0;
    devWatt = 0.0;
    devValid = false;
#ifdef WIN32
    devicePort = INVALID_HANDLE_VALUE;
#else
    devicePort = -1;
#endif
}

FreebeatUSB::~FreebeatUSB() {}

/* ----------------------------------------------------------------------
 * PUBLIC CONTROL METHODS
 * ---------------------------------------------------------------------- */
int FreebeatUSB::start() {
    QThread::start();
    return 0;
}

int FreebeatUSB::stop() {
    pvars.lock();
    doStop = true;
    pvars.unlock();
    return 0;
}

void FreebeatUSB::setResistance(int resistance) {
    if (resistance < 1) resistance = 1;
    if (resistance > 100) resistance = 100;
    pvars.lock();
    targetResistance = resistance;
    writeResistance = true;
    pvars.unlock();
}

void FreebeatUSB::sendQuery() {
    pvars.lock();
    doQuery = true;
    pvars.unlock();
}

void FreebeatUSB::sendStop() {
    pvars.lock();
    doStop = true;
    pvars.unlock();
}

void FreebeatUSB::sendReset() {
    pvars.lock();
    doReset = true;
    pvars.unlock();
}

void FreebeatUSB::getTelemetry(int &rpm, int &resistance, double &speed, double &watt, bool &valid) {
    pvars.lock();
    rpm = devRpm;
    resistance = devResistance;
    speed = devSpeed;
    watt = devWatt;
    valid = devValid;
    pvars.unlock();
}

/* ----------------------------------------------------------------------
 * PROTOCOL HELPERS
 * ---------------------------------------------------------------------- */
// Build 5-byte command: [startCode, action, data, (action+data)&0xFF, 0xA0]
QByteArray FreebeatUSB::buildCmd5(uint8_t startCode, uint8_t action, uint8_t data) {
    QByteArray cmd(5, 0);
    cmd[0] = startCode;
    cmd[1] = action;
    cmd[2] = data;
    cmd[3] = (action + data) & 0xFF;
    cmd[4] = FREEBEAT_END_BYTE;
    return cmd;
}

// Build 4-byte LED heartbeat: [0xB5, 0x08, 0x08, 0xA0]
QByteArray FreebeatUSB::buildLedQuery() {
    QByteArray cmd(4, 0);
    cmd[0] = (char)FREEBEAT_CMD_LED;
    cmd[1] = (char)FREEBEAT_ACT_LED_CHECK;
    cmd[2] = (char)FREEBEAT_ACT_LED_CHECK;
    cmd[3] = (char)FREEBEAT_END_BYTE;
    return cmd;
}

// Parse incoming data packet and update telemetry
// Data packet: byte[0]=0x55, byte[1]=type(0x15/0x03/0x25),
//   bytes[4-5]=speed(16-bit LE), byte[7]=resistance(1-100),
//   bytes[8-9]=RPM(16-bit LE), byte[11]=checksum=sum(bytes[1..10])&0xFF, byte[12]=0x3F
bool FreebeatUSB::parsePacket(const QByteArray &pkt) {
    if (pkt.size() < 13)
        return false;

    uint8_t b0 = (uint8_t)pkt[0];
    if (b0 == FREEBEAT_SYNC_LED) {
        // LED status packet (20 bytes) — ignore for telemetry
        return true;
    }

    if (b0 != FREEBEAT_SYNC_DATA)
        return false;

    // Validate checksum: sum bytes[1..10]
    uint8_t chk = 0;
    for (int i = 1; i <= 10; i++)
        chk += (uint8_t)pkt[i];
    chk &= 0xFF;

    if (chk != (uint8_t)pkt[11])
        return false;

    if ((uint8_t)pkt[12] != FREEBEAT_VERIFY_BYTE)
        return false;

    uint16_t rawSpeed = ((uint8_t)pkt[4]) | (((uint8_t)pkt[5]) << 8);
    uint8_t rawRes    = (uint8_t)pkt[7];
    uint16_t rawRpm   = ((uint8_t)pkt[8]) | (((uint8_t)pkt[9]) << 8);

    double speed = rawSpeed * 0.1; // km/h
    int rpm = rawRpm;
    int res = rawRes;
    // Power estimate using Freebeat's 4.56 coefficient (standard wheel)
    double watt = rpm * 4.56 * res / 100.0;

    pvars.lock();
    devSpeed = speed;
    devRpm = rpm;
    devResistance = res;
    devWatt = watt;
    devValid = true;
    pvars.unlock();

    qDebug() << "Freebeat RX: speed=" << speed << "rpm=" << rpm << "res=" << res << "watt=" << watt;
    return true;
}

/* ----------------------------------------------------------------------
 * RAW I/O
 * Note: Freebeat communicates via internal UART (/dev/ttyS4), NOT USB.
 * We open the port directly with POSIX open() on all platforms (Android
 * included) — NOT through Usbserial.java which is for USB-CDC/FTDI adapters.
 * On Android, chmod 777 is required before open() to get rw access to the
 * UART device node (same approach used by the original Freebeat app's
 * DCUARTDriver class).
 * ---------------------------------------------------------------------- */
int FreebeatUSB::rawWrite(const char *bytes, int size) {
    qDebug() << "Freebeat TX:" << QByteArray(bytes, size).toHex();
#ifdef WIN32
    DWORD cBytes;
    if (!WriteFile(devicePort, bytes, size, &cBytes, NULL))
        return -1;
    return (int)cBytes;
#else
    int rc = write(devicePort, bytes, size);
    if (rc != -1)
        tcdrain(devicePort);
    return rc;
#endif
}

// Read up to maxLen bytes with a timeout in ms; returns bytes read
int FreebeatUSB::rawRead(char *buf, int maxLen, int timeoutMs) {
#ifdef WIN32
    DWORD cBytes = 0;
    COMMTIMEOUTS ct;
    GetCommTimeouts(devicePort, &ct);
    ct.ReadTotalTimeoutConstant = timeoutMs;
    ct.ReadIntervalTimeout = 0;
    SetCommTimeouts(devicePort, &ct);
    ReadFile(devicePort, buf, maxLen, &cBytes, NULL);
    return (int)cBytes;
#else
    // Use select() for non-blocking read with timeout
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(devicePort, &fds);
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    int rc = select(devicePort + 1, &fds, NULL, NULL, &tv);
    if (rc > 0)
        return read(devicePort, buf, maxLen);
    return 0;
#endif
}

/* ----------------------------------------------------------------------
 * PORT OPEN / CLOSE
 * ---------------------------------------------------------------------- */
int FreebeatUSB::openPort() {
#ifdef WIN32
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
                            FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, 0, NULL);
    if (devicePort == INVALID_HANDLE_VALUE)
        return -1;

    if (!GetCommState(devicePort, &deviceSettings))
        return -1;

    deviceSettings.BaudRate = CBR_9600;
    deviceSettings.fParity = NOPARITY;
    deviceSettings.ByteSize = 8;
    deviceSettings.StopBits = ONESTOPBIT;
    deviceSettings.fBinary = TRUE;
    deviceSettings.fOutX = 0;
    deviceSettings.fInX = 0;
    deviceSettings.fRtsControl = RTS_CONTROL_ENABLE;
    deviceSettings.fDtrControl = DTR_CONTROL_ENABLE;
    deviceSettings.fOutxCtsFlow = FALSE;

    if (!SetCommState(devicePort, &deviceSettings)) {
        CloseHandle(devicePort);
        return -1;
    }

    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 2000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(devicePort, &timeouts);
    return 0;
#else
    // POSIX path — used on Linux, macOS, and Android.
    // On Android the UART (/dev/ttyS4) is owned by root; chmod 777 grants access.
#ifdef Q_OS_ANDROID
    {
        QString cmd = "chmod 777 " + deviceFilename;
        int rv = system(cmd.toLatin1().constData());
        if (rv != 0)
            qDebug() << "Freebeat: chmod failed for" << deviceFilename;
    }
#endif

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
    cfsetspeed(&deviceSettings, B9600);

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
    deviceSettings.c_cc[VTIME] = 5; // 0.5 second timeout

    if (tcsetattr(devicePort, TCSANOW, &deviceSettings) == -1)
        return errno;

    tcflush(devicePort, TCIOFLUSH);
    return 0;
#endif
}

int FreebeatUSB::closePort() {
#ifdef WIN32
    return (int)!CloseHandle(devicePort);
#else
    tcflush(devicePort, TCIOFLUSH);
    return close(devicePort);
#endif
}

/* ----------------------------------------------------------------------
 * MAIN RUN LOOP
 * ---------------------------------------------------------------------- */
void FreebeatUSB::run() {
    if (openPort()) {
        qDebug() << "FreebeatUSB: failed to open port" << deviceFilename;
        return;
    }

    // Send initial machine query
    {
        QByteArray q = buildCmd5(FREEBEAT_CMD_START, FREEBEAT_ACT_QUERY, 0);
        rawWrite(q.constData(), q.size());
    }

    QThread::msleep(500);

    while (true) {
        pvars.lock();
        bool stop    = doStop;
        bool reset   = doReset;
        bool query   = doQuery;
        bool writeR  = writeResistance;
        int  res     = targetResistance;
        doQuery      = false;
        writeResistance = false;
        pvars.unlock();

        if (stop) {
            QByteArray cmd = buildCmd5(FREEBEAT_CMD_START, FREEBEAT_ACT_STOP, 0);
            rawWrite(cmd.constData(), cmd.size());
            break;
        }

        if (reset) {
            QByteArray cmd = buildCmd5(FREEBEAT_CMD_START, FREEBEAT_ACT_RESET, 0);
            rawWrite(cmd.constData(), cmd.size());
            pvars.lock();
            doReset = false;
            pvars.unlock();
        }

        if (writeR) {
            QByteArray cmd = buildCmd5(FREEBEAT_CMD_START, FREEBEAT_ACT_RESISTANCE, (uint8_t)res);
            rawWrite(cmd.constData(), cmd.size());
        }

        if (query) {
            QByteArray cmd = buildCmd5(FREEBEAT_CMD_START, FREEBEAT_ACT_QUERY, 0);
            rawWrite(cmd.constData(), cmd.size());
        }

        // Send LED heartbeat every cycle to keep bike awake
        {
            QByteArray led = buildLedQuery();
            rawWrite(led.constData(), led.size());
        }

        // Read response — data packets are 13 bytes, LED packets 20 bytes
        char buf[64];
        int n = rawRead(buf, sizeof(buf), 200);
        if (n > 0) {
            QByteArray pkt(buf, n);
            parsePacket(pkt);
        }

        QThread::msleep(200);
    }

    closePort();
}
