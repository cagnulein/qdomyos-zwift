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

#include "KettlerJoystick.h"
#include <QThread>

KettlerJoystick::KettlerJoystick(QObject *parent, QString deviceFilename) {
    this->parent = parent;
    this->deviceFilename = deviceFilename;
#ifdef WIN32
    devicePort = INVALID_HANDLE_VALUE;
#else
    devicePort = -1;
#endif

    // Initialize last button press times to far in the past
    QDateTime farPast = QDateTime::currentDateTime().addSecs(-10);
    for (int i = 0; i < 4; i++) {
        lastButtonPress[i] = farPast;
    }
}

KettlerJoystick::~KettlerJoystick() {
    stop();
    quit(0);
}

void KettlerJoystick::setDevice(QString deviceFilename) {
    this->deviceFilename = deviceFilename;
}

int KettlerJoystick::openPort() {
#ifdef WIN32
    // Windows implementation
    devicePort = CreateFileA(deviceFilename.toStdString().c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

    if (devicePort == INVALID_HANDLE_VALUE) {
        qDebug() << "KettlerJoystick: Failed to open port" << deviceFilename;
        return -1;
    }

    // Configure port settings
    memset(&deviceSettings, 0, sizeof(deviceSettings));
    deviceSettings.DCBlength = sizeof(deviceSettings);

    if (!GetCommState(devicePort, &deviceSettings)) {
        qDebug() << "KettlerJoystick: GetCommState failed";
        CloseHandle(devicePort);
        devicePort = INVALID_HANDLE_VALUE;
        return -1;
    }

    // Configure for joystick (we only read modem control lines)
    deviceSettings.BaudRate = CBR_9600;
    deviceSettings.ByteSize = 8;
    deviceSettings.StopBits = ONESTOPBIT;
    deviceSettings.Parity = NOPARITY;
    deviceSettings.fDtrControl = DTR_CONTROL_DISABLE;
    deviceSettings.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(devicePort, &deviceSettings)) {
        qDebug() << "KettlerJoystick: SetCommState failed";
        CloseHandle(devicePort);
        devicePort = INVALID_HANDLE_VALUE;
        return -1;
    }

    qDebug() << "KettlerJoystick: Port opened successfully:" << deviceFilename;
    return 0;

#else
    // Linux/Android implementation
    devicePort = open(deviceFilename.toStdString().c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (devicePort == -1) {
        qDebug() << "KettlerJoystick: Failed to open port" << deviceFilename << "error:" << strerror(errno);
        return -1;
    }

    // Configure port settings
    memset(&deviceSettings, 0, sizeof(deviceSettings));

    if (tcgetattr(devicePort, &deviceSettings) != 0) {
        qDebug() << "KettlerJoystick: tcgetattr failed";
        close(devicePort);
        devicePort = -1;
        return -1;
    }

    // Configure for joystick (we only read modem control lines)
    cfsetispeed(&deviceSettings, B9600);
    cfsetospeed(&deviceSettings, B9600);

    deviceSettings.c_cflag |= (CLOCAL | CREAD);
    deviceSettings.c_cflag &= ~PARENB;
    deviceSettings.c_cflag &= ~CSTOPB;
    deviceSettings.c_cflag &= ~CSIZE;
    deviceSettings.c_cflag |= CS8;
    deviceSettings.c_cflag &= ~CRTSCTS;

    deviceSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    deviceSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
    deviceSettings.c_oflag &= ~OPOST;

    if (tcsetattr(devicePort, TCSANOW, &deviceSettings) != 0) {
        qDebug() << "KettlerJoystick: tcsetattr failed";
        close(devicePort);
        devicePort = -1;
        return -1;
    }

    qDebug() << "KettlerJoystick: Port opened successfully:" << deviceFilename;
    return 0;
#endif
}

int KettlerJoystick::closePort() {
#ifdef WIN32
    if (devicePort != INVALID_HANDLE_VALUE) {
        CloseHandle(devicePort);
        devicePort = INVALID_HANDLE_VALUE;
    }
#else
    if (devicePort != -1) {
        close(devicePort);
        devicePort = -1;
    }
#endif
    return 0;
}

bool KettlerJoystick::discover(QString deviceFilename) {
    this->deviceFilename = deviceFilename;

    if (openPort() != 0) {
        return false;
    }

    // Try to read modem control lines to verify it's a valid serial port
#ifdef WIN32
    DWORD modemStatus;
    if (!GetCommModemStatus(devicePort, &modemStatus)) {
        qDebug() << "KettlerJoystick: GetCommModemStatus failed";
        closePort();
        return false;
    }
#else
    int status;
    if (ioctl(devicePort, TIOCMGET, &status) == -1) {
        qDebug() << "KettlerJoystick: TIOCMGET failed:" << strerror(errno);
        closePort();
        return false;
    }
#endif

    closePort();
    qDebug() << "KettlerJoystick: Device discovered successfully:" << deviceFilename;
    return true;
}

void KettlerJoystick::checkButtonStates() {
#ifdef WIN32
    DWORD modemStatus;
    if (!GetCommModemStatus(devicePort, &modemStatus)) {
        return;
    }

    // Map Windows modem status bits to button states
    bool currentStates[4];
    currentStates[KETTLER_JOY_LEFT_ARROW] = (modemStatus & MS_RLSD_ON) != 0;  // DCD (RLSD)
    currentStates[KETTLER_JOY_RIGHT_ARROW] = (modemStatus & MS_CTS_ON) != 0;  // CTS
    currentStates[KETTLER_JOY_DOWN_ARROW] = (modemStatus & MS_DSR_ON) != 0;   // DSR
    currentStates[KETTLER_JOY_UP_ARROW] = (modemStatus & MS_RING_ON) != 0;    // RING

#else
    int status;
    if (ioctl(devicePort, TIOCMGET, &status) == -1) {
        return;
    }

    // Map Linux modem control lines to button states
    bool currentStates[4];
    currentStates[KETTLER_JOY_LEFT_ARROW] = (status & TIOCM_CD) != 0;   // DCD
    currentStates[KETTLER_JOY_RIGHT_ARROW] = (status & TIOCM_CTS) != 0; // CTS
    currentStates[KETTLER_JOY_DOWN_ARROW] = (status & TIOCM_DSR) != 0;  // DSR
    currentStates[KETTLER_JOY_UP_ARROW] = (status & TIOCM_RI) != 0;     // RING (RI)
#endif

    QDateTime now = QDateTime::currentDateTime();

    // Check for rising edge (button press) with debouncing
    for (int i = 0; i < 4; i++) {
        // Rising edge detection: button was not pressed, now is pressed
        if (!lastButtonStates[i] && currentStates[i]) {
            // Check debounce time
            qint64 msSinceLastPress = lastButtonPress[i].msecsTo(now);
            if (msSinceLastPress >= KETTLER_JOY_DEBOUNCE_MS) {
                qDebug() << "KettlerJoystick: Button" << i << "pressed";
                emit buttonPressed(i);
                lastButtonPress[i] = now;
            }
        }
        lastButtonStates[i] = currentStates[i];
    }
}

void KettlerJoystick::run() {
    qDebug() << "KettlerJoystick: Thread started";

    if (openPort() != 0) {
        qDebug() << "KettlerJoystick: Failed to open port, exiting thread";
        return;
    }

    shouldStop = false;

    while (!shouldStop) {
        checkButtonStates();
        QThread::msleep(KETTLER_JOY_POLL_INTERVAL);
    }

    closePort();
    qDebug() << "KettlerJoystick: Thread stopped";
}

int KettlerJoystick::start() {
    shouldStop = false;
    QThread::start();
    return 0;
}

int KettlerJoystick::stop() {
    shouldStop = true;

    // Wait for thread to finish (max 2 seconds)
    if (!wait(2000)) {
        qDebug() << "KettlerJoystick: Thread did not stop gracefully, terminating";
        terminate();
        wait();
    }

    return 0;
}

int KettlerJoystick::quit(int error) {
    Q_UNUSED(error);
    closePort();
    return 0;
}
