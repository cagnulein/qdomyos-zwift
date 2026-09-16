/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include "DaumUSB.h"

#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#endif

#include <errno.h>

DaumUSB::DaumUSB(QObject *parent, QString deviceFilename, int baudrate) : QThread(parent) {
    this->deviceFilename = deviceFilename;
    this->baudrate = baudrate;
}

DaumUSB::~DaumUSB() {
    stop();
    wait(2500);
}

int DaumUSB::start() {
    QThread::start();
    return 0;
}

int DaumUSB::restart() {
    pvars.lock();
    if ((deviceStatus & DAUM_RUNNING) && (deviceStatus & DAUM_PAUSED)) {
        deviceStatus &= ~DAUM_PAUSED;
        pvars.unlock();
        return 0;
    }
    pvars.unlock();
    return 2;
}

int DaumUSB::pause() {
    pvars.lock();
    if (!(deviceStatus & DAUM_RUNNING)) {
        pvars.unlock();
        return 4;
    }
    if (deviceStatus & DAUM_PAUSED) {
        pvars.unlock();
        return 2;
    }
    deviceStatus |= DAUM_PAUSED;
    pvars.unlock();
    return 0;
}

int DaumUSB::stop() {
    pvars.lock();
    deviceStatus = 0;
    writePower = false;
    gearAdjustmentRequested = false;
    pvars.unlock();
    return 0;
}

void DaumUSB::setPower(double power) {
    const int step = powerStep(power);
    pvars.lock();
    // Daum accepts one Set_Watt command per target. Re-sending the same
    // target every 200 ms makes the cockpit overwrite manual jog-dial input.
    if ((writePower && powerStep(targetPower) == step) || lastPowerStep == step) {
        pvars.unlock();
        return;
    }
    targetPower = power;
    writePower = true;
    pvars.unlock();
}

void DaumUSB::requestGearAdjustment() {
    pvars.lock();
    gearAdjustmentRequested = true;
    pvars.unlock();
}

void DaumUSB::getTelemetry(double &power, double &heartRate, double &cadence, double &speed,
                           double &distance, uint8_t &gear, int &status) {
    pvars.lock();
    power = devicePower;
    heartRate = deviceHeartRate;
    cadence = deviceCadence;
    speed = deviceSpeed;
    distance = deviceDistance;
    gear = deviceGear;
    status = deviceStatus;
    pvars.unlock();
}

bool DaumUSB::connected() {
    pvars.lock();
    bool result = (deviceStatus & DAUM_RUNNING) && !(deviceStatus & DAUM_PAUSED);
    pvars.unlock();
    return result;
}

int DaumUSB::powerStep(double power) {
    int step = static_cast<int>((power + 2.5) / 5.0);
    if (step < 5)
        step = 5;
    if (step > 160)
        step = 160;
    return step;
}

QByteArray DaumUSB::readResponse(int expectedLength, unsigned long timeoutMs) {
    QByteArray response;
    unsigned long waited = 0;

    while (waited <= timeoutMs && response.size() < expectedLength) {
        QByteArray chunk = rawReadBytes();
        if (!chunk.isEmpty())
            response.append(chunk);

        if (response.size() >= expectedLength)
            break;

        QThread::msleep(20);
        waited += 20;
    }
    return response;
}

bool DaumUSB::detectCockpit() {
    const char command = static_cast<char>(0x11); // Get_Adress
    rawWrite(&command, 1);

    QByteArray response = readResponse(2, 1200);
    for (int i = 0; i + 1 < response.size(); ++i) {
        if (static_cast<uint8_t>(response.at(i)) == 0x11) {
            cockpitAddress = static_cast<uint8_t>(response.at(i + 1));
            qDebug() << "Daum cockpit detected, address" << cockpitAddress;
            QThread::msleep(60); // protocol requires >= 50 ms between packets
            return true;
        }
    }

    qDebug() << "Daum Get_Adress failed, response" << response.toHex(' ');
    return false;
}

bool DaumUSB::initializeProgram0() {
    auto sendCommand = [this](uint8_t command, bool hasData, uint8_t data,
                              int expectedLength, const char *name) {
        QByteArray request;
        request.append(static_cast<char>(command));
        request.append(static_cast<char>(cockpitAddress));
        if (hasData)
            request.append(static_cast<char>(data));

        rawWrite(request.constData(), request.size());
        const QByteArray response = readResponse(expectedLength, 1200);
        const bool acknowledged = response.startsWith(request);
        if (!acknowledged)
            qDebug() << "Daum" << name << "acknowledgement missing:" << response.toHex(' ');
        QThread::msleep(60);
        return acknowledged;
    };

    // Match the safe setup sequence used by Ergo48: reset the cockpit, stop
    // any active program, start the PC-controlled program, then select the
    // manual Watt program (Program 0). Do not overwrite the user's cockpit
    // person profile; QZ only needs the program/control state.
    if (!sendCommand(0x12, false, 0, 2, "Reset_Device"))
        return false;
    QThread::msleep(1000);
    if (!sendCommand(0x22, false, 0, 3, "Stop_Prog"))
        return false;
    if (!sendCommand(0x21, false, 0, 3, "Start_Prog"))
        return false;
    if (!sendCommand(0x23, true, 0, 4, "Set_Prog"))
        return false;

    qDebug() << "Daum Program 0 initialized";
    return true;
}

bool DaumUSB::initializeGearAdjustment() {
    // Ergo48 sends Set_Gang 14 once after starting its smart workout. The
    // following Run_Daten responses then contain the physical jog-dial gear
    // changes. This is an initialization command, not a per-change command.
    constexpr uint8_t initialGear = 14;
    const char request[3] = {
        static_cast<char>(0x53),
        static_cast<char>(cockpitAddress),
        static_cast<char>(initialGear)
    };
    rawWrite(request, sizeof(request));

    const QByteArray response = readResponse(3, 1200);
    bool acknowledged = false;
    for (int i = 0; i + 2 < response.size(); ++i) {
        if (static_cast<uint8_t>(response.at(i)) == 0x53 &&
            static_cast<uint8_t>(response.at(i + 1)) == cockpitAddress &&
            static_cast<uint8_t>(response.at(i + 2)) == initialGear) {
            acknowledged = true;
            break;
        }
    }

    if (acknowledged)
        qDebug() << "Daum gear adjustment initialized at gear" << initialGear;
    else
        qDebug() << "Daum Set_Gang acknowledgement missing:" << response.toHex(' ');

    QThread::msleep(60);
    return acknowledged;
}

bool DaumUSB::writePowerTarget(double power) {
    // Set_Watt stores the target in 5 W units. Classic cockpits accept either
    // 25..400 W or 50..800 W depending on model. The 8008 TRS_3 supports the
    // upper range, so keep the protocol byte inside its documented 5..160 range.
    const int step = powerStep(power);

    char request[3] = {
        static_cast<char>(0x51),
        static_cast<char>(cockpitAddress),
        static_cast<char>(step)
    };
    rawWrite(request, sizeof(request));

    QByteArray response = readResponse(3, 1200);
    bool acknowledged = false;
    for (int i = 0; i + 2 < response.size(); ++i) {
        if (static_cast<uint8_t>(response.at(i)) == 0x51 &&
            static_cast<uint8_t>(response.at(i + 1)) == cockpitAddress &&
            static_cast<uint8_t>(response.at(i + 2)) == static_cast<uint8_t>(step)) {
            acknowledged = true;
            break;
        }
    }

    if (!acknowledged)
        qDebug() << "Daum Set_Watt acknowledgement missing:" << response.toHex(' ');
    else {
        pvars.lock();
        lastPowerStep = step;
        pvars.unlock();
    }

    QThread::msleep(60);
    return acknowledged;
}

bool DaumUSB::pollTelemetry() {
    const char request[2] = {static_cast<char>(0x40), static_cast<char>(cockpitAddress)};
    rawWrite(request, sizeof(request));

    QByteArray response = readResponse(19, 1200);
    int start = -1;
    for (int i = 0; i + 18 < response.size(); ++i) {
        uint8_t command = static_cast<uint8_t>(response.at(i));
        if ((command == 0x40 || command == 0x41) &&
            static_cast<uint8_t>(response.at(i + 1)) == cockpitAddress) {
            start = i;
            break;
        }
    }

    if (start < 0) {
        if (!response.isEmpty())
            qDebug() << "Invalid Daum Run_Daten response:" << response.toHex(' ');
        return false;
    }

    const uint8_t powerInFiveWatts = static_cast<uint8_t>(response.at(start + 5));
    const uint8_t cadence = static_cast<uint8_t>(response.at(start + 6));
    const uint8_t speed = static_cast<uint8_t>(response.at(start + 7));
    const uint16_t distanceInHundredMeters =
        static_cast<uint8_t>(response.at(start + 8)) |
        (static_cast<uint16_t>(static_cast<uint8_t>(response.at(start + 9))) << 8);
    const uint8_t heartRate = static_cast<uint8_t>(response.at(start + 14));
    const uint8_t gear = static_cast<uint8_t>(response.at(start + 16));

    pvars.lock();
    // The cockpit always reports its minimum 25 W step while stopped. Do not
    // expose that idle value as actual effort to FTMS applications.
    devicePower = cadence == 0 ? 0.0 : static_cast<double>(powerInFiveWatts) * 5.0;
    deviceHeartRate = heartRate;
    deviceCadence = cadence;
    deviceSpeed = speed;
    deviceDistance = static_cast<double>(distanceInHundredMeters) * 100.0;
    deviceGear = gear;
    pvars.unlock();

    qDebug() << "Daum status HR" << deviceHeartRate
             << "RPM" << deviceCadence
             << "speed" << deviceSpeed
             << "distance" << deviceDistance
             << "power" << devicePower
             << "gear" << gear;
    return true;
}

void DaumUSB::run() {
    pvars.lock();
    deviceStatus = DAUM_RUNNING;
    devicePower = deviceHeartRate = deviceCadence = deviceSpeed = deviceDistance = 0.0;
    deviceGear = 1;
    writePower = false;
    lastPowerStep = -1;
    pvars.unlock();

    if (openPort() != 0) {
        qDebug() << "Unable to open Daum serial port" << deviceFilename;
        stop();
        return;
    }

    QThread::msleep(150);
    if (!detectCockpit()) {
        closePort();
        stop();
        return;
    }

    if (!initializeProgram0()) {
        closePort();
        stop();
        return;
    }

    while (true) {
        pvars.lock();
        int status = deviceStatus;
        bool shouldWritePower = writePower;
        bool shouldInitializeGears = gearAdjustmentRequested;
        double power = targetPower;
        if (shouldWritePower)
            writePower = false;
        if (shouldInitializeGears)
            gearAdjustmentRequested = false;
        pvars.unlock();

        if (!(status & DAUM_RUNNING))
            break;

        if (status & DAUM_PAUSED) {
            QThread::msleep(100);
            continue;
        }

        // Ergo48 sends Set_Gang once after the Program 0 setup. The physical
        // gear changes themselves arrive in Run_Daten and are not written
        // back by QZ.
        if (shouldInitializeGears)
            initializeGearAdjustment();

        if (shouldWritePower)
            writePowerTarget(power);

        pollTelemetry();

        // The official protocol specifies Run_Daten at a one-second cycle.
        QThread::msleep(1000);
    }

    closePort();
}

int DaumUSB::openPort() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject path = QAndroidJniObject::fromString(deviceFilename);
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/Usbserial", "open",
        "(Landroid/content/Context;ILjava/lang/String;)V",
        QtAndroid::androidContext().object(), baudrate, path.object<jstring>());
    return 0;
#elif defined(WIN32)
    COMMTIMEOUTS timeouts;
    QString portSpec = deviceFilename;
    if (deviceFilename.startsWith(QStringLiteral("COM"), Qt::CaseInsensitive)) {
        int portnum = deviceFilename.midRef(3).toString().toInt();
        if (portnum >= 10)
            portSpec = "\\\\.\\" + deviceFilename;
    }

    wchar_t deviceFilenameW[64];
    MultiByteToWideChar(CP_ACP, 0, portSpec.toLatin1(), -1, deviceFilenameW, 64);
    devicePort = CreateFile(deviceFilenameW, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, 0, NULL);
    if (devicePort == INVALID_HANDLE_VALUE)
        return -1;

    if (!GetCommState(devicePort, &deviceSettings))
        return -1;

    deviceSettings.BaudRate = baudrate;
    deviceSettings.fParity = NOPARITY;
    deviceSettings.ByteSize = 8;
    deviceSettings.StopBits = ONESTOPBIT;
    deviceSettings.fBinary = true;
    deviceSettings.fOutX = 0;
    deviceSettings.fInX = 0;
    deviceSettings.fRtsControl = RTS_CONTROL_ENABLE;
    deviceSettings.fDtrControl = DTR_CONTROL_ENABLE;
    deviceSettings.fOutxCtsFlow = FALSE;

    if (!SetCommState(devicePort, &deviceSettings)) {
        CloseHandle(devicePort);
        devicePort = INVALID_HANDLE_VALUE;
        return -1;
    }

    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 1000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(devicePort, &timeouts);
    PurgeComm(devicePort, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return 0;
#else
#if defined(Q_OS_MACX)
    int ldisc = TTYDISC;
#else
    int ldisc = N_TTY;
#endif

    devicePort = open(deviceFilename.toLatin1(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (devicePort == -1)
        return errno;

    tcflush(devicePort, TCIOFLUSH);
    if (ioctl(devicePort, TIOCSETD, &ldisc) == -1)
        return errno;
    if (tcgetattr(devicePort, &deviceSettings) == -1)
        return errno;

    cfmakeraw(&deviceSettings);
    speed_t speed = B9600;
    switch (baudrate) {
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
        case 9600:
        default: speed = B9600; break;
    }
    cfsetispeed(&deviceSettings, speed);
    cfsetospeed(&deviceSettings, speed);
    deviceSettings.c_cflag &= ~(CSIZE | CSTOPB | PARENB);
    deviceSettings.c_cflag |= CS8 | CLOCAL | CREAD;
    deviceSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
#ifdef CRTSCTS
    deviceSettings.c_cflag &= ~CRTSCTS;
#endif
    deviceSettings.c_cc[VMIN] = 0;
    deviceSettings.c_cc[VTIME] = 1;

    if (tcsetattr(devicePort, TCSANOW, &deviceSettings) == -1)
        return errno;
    tcflush(devicePort, TCIOFLUSH);
    return 0;
#endif
}

int DaumUSB::closePort() {
#ifdef Q_OS_ANDROID
    // Usbserial.java owns the Android USB handle for the lifetime of QZ.
    return 0;
#elif defined(WIN32)
    if (devicePort == INVALID_HANDLE_VALUE)
        return 0;
    bool ok = CloseHandle(devicePort);
    devicePort = INVALID_HANDLE_VALUE;
    return ok ? 0 : -1;
#else
    if (devicePort < 0)
        return 0;
    tcflush(devicePort, TCIOFLUSH);
    int result = close(devicePort);
    devicePort = -1;
    return result;
#endif
}

int DaumUSB::rawWrite(const char *bytes, int size) {
    qDebug() << "Daum TX:" << QByteArray(bytes, size).toHex(' ');
#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    jbyteArray data = env->NewByteArray(size);
    env->SetByteArrayRegion(data, 0, size, reinterpret_cast<const jbyte *>(bytes));
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "write", "([B)V", data);
    env->DeleteLocalRef(data);
    return size;
#elif defined(WIN32)
    DWORD written = 0;
    if (!WriteFile(devicePort, bytes, size, &written, NULL))
        return -1;
    return static_cast<int>(written);
#else
    int result = write(devicePort, bytes, size);
    if (result >= 0)
        tcdrain(devicePort);
    return result;
#endif
}

QByteArray DaumUSB::rawReadBytes() {
    QByteArray result;
#ifdef Q_OS_ANDROID
    QAndroidJniObject data = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/Usbserial", "read", "()[B");
    jint len = QAndroidJniObject::callStaticMethod<jint>(
        "org/cagnulen/qdomyoszwift/Usbserial", "readLen", "()I");
    if (len > 0 && data.isValid()) {
        result.resize(len);
        QAndroidJniEnvironment env;
        env->GetByteArrayRegion(data.object<jbyteArray>(), 0, len,
                                reinterpret_cast<jbyte *>(result.data()));
    }
#elif defined(WIN32)
    char buffer[256];
    DWORD readCount = 0;
    if (ReadFile(devicePort, buffer, sizeof(buffer), &readCount, NULL) && readCount > 0)
        result.append(buffer, static_cast<int>(readCount));
#else
    char buffer[256];
    int readCount = read(devicePort, buffer, sizeof(buffer));
    if (readCount > 0)
        result.append(buffer, readCount);
#endif
    if (!result.isEmpty())
        qDebug() << "Daum RX:" << result.toHex(' ');
    return result;
}
