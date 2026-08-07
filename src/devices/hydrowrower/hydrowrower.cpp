#include "hydrowrower.h"

#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>
#include <QStringList>
#include <cstring>

#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

#ifndef WIN32
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {
constexpr int HYDROW_BAUD = 921600;

QString printable(const QByteArray &data) {
    return QString::fromLatin1(data).trimmed();
}
}

HydrowSerialThread::HydrowSerialThread(QObject *parent)
    : QThread(parent) {}

HydrowSerialThread::~HydrowSerialThread() {
    stop();
    wait(3000);
}

void HydrowSerialThread::stop() {
    QMutexLocker locker(&mutex);
    running = false;
}

void HydrowSerialThread::run() {
    {
        QMutexLocker locker(&mutex);
        running = true;
    }

    emit debug(QStringLiteral("Hydrow serial worker starting with automatic port detection at %1 baud").arg(HYDROW_BAUD));
    if (!openPort()) {
        emit debug(QStringLiteral("Hydrow serial worker could not open any candidate port"));
        return;
    }

    // The Hydrow APK sends this command immediately after opening the UART.
    writeCommand(QByteArrayLiteral("Cm 0\r"));

    while (true) {
        {
            QMutexLocker locker(&mutex);
            if (!running)
                break;
        }

#ifdef Q_OS_ANDROID
        QAndroidJniEnvironment env;
        QAndroidJniObject data = QAndroidJniObject::callStaticObjectMethod(
            "org/cagnulen/qdomyoszwift/Usbserial", "read", "()[B");
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            emit debug(QStringLiteral("Hydrow Android serial read raised a JNI exception"));
        } else if (data.isValid()) {
            jint length = QAndroidJniObject::callStaticMethod<jint>(
                "org/cagnulen/qdomyoszwift/Usbserial", "readLen", "()I");
            if (length > 0) {
                jbyteArray array = data.object<jbyteArray>();
                jbyte *bytes = env->GetByteArrayElements(array, nullptr);
                if (bytes) {
                    processBytes(QByteArray(reinterpret_cast<const char *>(bytes), length));
                    env->ReleaseByteArrayElements(array, bytes, JNI_ABORT);
                }
            }
        }
        msleep(5);
#elif defined(WIN32)
        msleep(20);
#else
        char buffer[4096];
        const int length = static_cast<int>(::read(port, buffer, sizeof(buffer)));
        if (length > 0)
            processBytes(QByteArray(buffer, length));
        else
            msleep(5);
#endif
    }

    closePort();
    emit debug(QStringLiteral("Hydrow serial worker stopped"));
}

bool HydrowSerialThread::openPort() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/Usbserial", "openHydrow",
        "(Landroid/content/Context;I)V",
        QtAndroid::androidContext().object(), HYDROW_BAUD);
    emit debug(QStringLiteral("Hydrow Android UART candidate scan requested"));
    return true;
#elif defined(WIN32)
    const QStringList candidates = {QStringLiteral("COM1"), QStringLiteral("COM2"), QStringLiteral("COM3")};
#else
    const QStringList candidates = {QStringLiteral("/dev/ttyMT1"), QStringLiteral("/dev/ttyUSB0"), QStringLiteral("/dev/ttyACM0")};
#endif

#ifdef Q_OS_ANDROID
    activeDevicePath = QStringLiteral("auto");
    return true;
#else
    for (const QString &candidate : candidates) {
        port = ::open(candidate.toLocal8Bit().constData(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (port < 0) {
            emit debug(QStringLiteral("Hydrow open failed: %1 (%2)").arg(candidate, QString::fromLocal8Bit(strerror(errno))));
            continue;
        }

        termios settings{};
        if (tcgetattr(port, &settings) != 0) {
            emit debug(QStringLiteral("Hydrow tcgetattr failed: %1").arg(QString::fromLocal8Bit(strerror(errno))));
            closePort();
            continue;
        }
        cfmakeraw(&settings);
        cfsetspeed(&settings, B921600);
        settings.c_cflag &= static_cast<tcflag_t>(~(CSIZE | CSTOPB | PARENB | CRTSCTS));
        settings.c_cflag |= CS8 | CLOCAL | CREAD;
        settings.c_cc[VMIN] = 0;
        settings.c_cc[VTIME] = 1;
        if (tcsetattr(port, TCSANOW, &settings) != 0) {
            emit debug(QStringLiteral("Hydrow tcsetattr failed: %1").arg(QString::fromLocal8Bit(strerror(errno))));
            closePort();
            continue;
        }
        tcflush(port, TCIOFLUSH);
        activeDevicePath = candidate;
        emit debug(QStringLiteral("Hydrow UART opened: %1 at %2 8N1").arg(candidate).arg(HYDROW_BAUD));
        return true;
    }
    return false;
#endif
}

void HydrowSerialThread::closePort() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "close", "()V");
#elif !defined(WIN32)
    if (port >= 0) {
        tcflush(port, TCIOFLUSH);
        ::close(port);
        port = -1;
    }
#endif
}

void HydrowSerialThread::writeCommand(const QByteArray &command) {
    emit debug(QStringLiteral("Hydrow TX: %1").arg(printable(command)));
#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    jbyteArray array = env->NewByteArray(command.size());
    env->SetByteArrayRegion(array, 0, command.size(), reinterpret_cast<const jbyte *>(command.constData()));
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "write", "([B)V", array);
    env->DeleteLocalRef(array);
#elif !defined(WIN32)
    if (port >= 0) {
        ::write(port, command.constData(), command.size());
        tcdrain(port);
    }
#endif
}

void HydrowSerialThread::processBytes(const QByteArray &bytes) {
    receiveBuffer.append(bytes);
    while (true) {
        const int newline = receiveBuffer.indexOf('\n');
        if (newline < 0)
            break;
        const QByteArray line = receiveBuffer.left(newline).trimmed();
        receiveBuffer.remove(0, newline + 1);
        if (!line.isEmpty())
            parseLine(line);
    }
    if (receiveBuffer.size() > 16384) {
        emit debug(QStringLiteral("Hydrow RX buffer reset after oversized partial record"));
        receiveBuffer.clear();
    }
}

void HydrowSerialThread::parseLine(const QByteArray &line) {
    const QString text = printable(line);
    emit debug(QStringLiteral("Hydrow RX: %1").arg(text));

    QStringList fields = text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (fields.isEmpty())
        return;

    const QString type = fields.takeFirst();
    bool ok = false;
    double cadence = lastCadence;
    double distanceMeters = lastDistanceMeters;
    double watts = lastWatts;
    double speedKph = lastSpeedKph;

    if (type == QStringLiteral("Di") && fields.size() >= 5) {
        const double watts10 = fields.value(1).toDouble(&ok);
        const double rpm10 = fields.value(3).toDouble();
        if (ok) {
            watts = watts10 / 10.0;
            cadence = rpm10 / 10.0;
        }
    } else if (type == QStringLiteral("Di2") && fields.size() >= 5) {
        // Di2 is logged by the APK as distance, handle force, handle position, power, sequence.
        const double distance = fields.value(0).toDouble(&ok);
        const double power = fields.value(3).toDouble();
        if (ok && distance >= lastDistanceMeters)
            distanceMeters = distance;
        if (fields.value(3).toDouble(&ok) && ok)
            watts = power;
    } else if (type == QStringLiteral("Ds2") && fields.size() >= 7) {
        const double distance = fields.value(2).toDouble(&ok);
        const double averagePower = fields.value(3).toDouble();
        if (ok && distance >= lastDistanceMeters)
            distanceMeters = distance;
        if (fields.value(3).toDouble(&ok) && ok)
            watts = averagePower;
    } else if (type == QStringLiteral("Rl") && fields.size() >= 1) {
        emit debug(QStringLiteral("Hydrow resistance: %1").arg(fields.first()));
        return;
    } else {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (distanceMeters > lastDistanceMeters && lastDistanceTimestamp > 0) {
        const double seconds = (now - lastDistanceTimestamp) / 1000.0;
        if (seconds > 0.0)
            speedKph = ((distanceMeters - lastDistanceMeters) / seconds) * 3.6;
    }
    if (distanceMeters != lastDistanceMeters)
        lastDistanceTimestamp = now;
    lastDistanceMeters = distanceMeters;
    lastCadence = cadence;
    lastWatts = watts;
    lastSpeedKph = speedKph;
    if (cadence > 0.0)
        strokeCount += cadence / 60.0;

    emit metrics(cadence, distanceMeters, watts, speedKph, strokeCount);
}

hydrowrower::hydrowrower(bool noWriteResistance, bool noHeartService, bool noVirtualDevice)
    : noWriteResistance(noWriteResistance), noHeartService(noHeartService), noVirtualDevice(noVirtualDevice) {
    emit debug(QStringLiteral("Hydrow rower created; serial setting is used as an enable trigger; port is automatic"));
    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &hydrowrower::update);
    refresh->start(200);

    serial = new HydrowSerialThread(this);
    connect(serial, &HydrowSerialThread::metrics, this, &hydrowrower::onMetrics);
    connect(serial, &HydrowSerialThread::debug, this, &hydrowrower::debug);
    serial->start();
    emit debug(QStringLiteral("Hydrow serial worker started; setting is only an enable trigger"));
}

hydrowrower::~hydrowrower() {
    if (serial) {
        serial->stop();
        serial->wait(3000);
    }
}

bool hydrowrower::connected() {
    return connectedState;
}

uint16_t hydrowrower::watts() {
    return static_cast<uint16_t>(qBound(0.0, currentWatts, 65535.0));
}

void hydrowrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    Q_UNUSED(device)
}

void hydrowrower::onMetrics(double cadence, double distanceMeters, double wattsValue, double speedKph, double strokes) {
    connectedState = true;
    Cadence = cadence;
    currentWatts = wattsValue;
    Speed = speedKph;
    Distance = distanceMeters / 1000.0;
    StrokesCount = strokes;
    if (cadence > 0.0) {
        CrankRevs = strokes;
        LastCrankEventTime = static_cast<uint16_t>(QDateTime::currentMSecsSinceEpoch() & 0xffff);
    }
    lastDistanceMeters = distanceMeters;
    lastSpeedKph = speedKph;
    lastMetricsTimestamp = QDateTime::currentMSecsSinceEpoch();
    emit debug(QStringLiteral("Hydrow metrics cadence=%1 distance_m=%2 speed=%3 watts=%4 strokes=%5")
                   .arg(cadence).arg(distanceMeters).arg(speedKph).arg(wattsValue).arg(strokes));
}

void hydrowrower::update() {
    if (!connectedState)
        return;
    update_metrics(false, watts());
    if (lastMetricsTimestamp > 0 && QDateTime::currentMSecsSinceEpoch() - lastMetricsTimestamp > 3000)
        connectedState = false;
}
