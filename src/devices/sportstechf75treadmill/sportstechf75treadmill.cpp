#include "sportstechf75treadmill.h"

#include <QDebug>
#include <QSettings>

#include "qzsettings.h"

#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

namespace {
constexpr int kBaudRate = 9600;
constexpr char kDevicePath[] = "/dev/ttyS1";
constexpr int kFrameStart = 0xFB;
constexpr int kFrameEnd = 0xFC;
constexpr int kTelemetryStart = 0xFD;
constexpr int kCommandRun = 0x07;
constexpr int kCommandStart = 0xA2;
constexpr int kCommandStop = 0x00;
}

sportstechf75treadmill::sportstechf75treadmill(bool noHeartService) {
    this->noHeartService = noHeartService;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &sportstechf75treadmill::update);
    openSerial();
    refresh->start(100);
}

sportstechf75treadmill::~sportstechf75treadmill() {
    if (refresh) {
        refresh->stop();
    }
    closeSerial();
}

bool sportstechf75treadmill::connected() {
    return serialOpened;
}

bool sportstechf75treadmill::serialAvailable() const {
    return serialOpened;
}

void sportstechf75treadmill::openSerial() {
#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/Usbserial", "open",
        "(Landroid/content/Context;ILjava/lang/String;)V", QtAndroid::androidContext().object(), kBaudRate,
        QAndroidJniObject::fromString(QString::fromLatin1(kDevicePath)).object<jstring>());
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        qWarning() << "Sportstech F75: failed to open" << kDevicePath;
        return;
    }
    serialOpened = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/cagnulen/qdomyoszwift/Usbserial", "isOpen", "()Z");
    if (!serialOpened) {
        qWarning() << "Sportstech F75: serial device is not open" << kDevicePath;
        return;
    }
    qDebug() << "Sportstech F75 serial opened at" << kDevicePath << kBaudRate;
#else
    qWarning() << "Sportstech F75 serial profile is available on Android only";
#endif
}

void sportstechf75treadmill::closeSerial() {
#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "close", "()V");
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
#endif
    serialOpened = false;
}

void sportstechf75treadmill::readSerial() {
#ifdef Q_OS_ANDROID
    if (!serialAvailable()) {
        return;
    }

    QAndroidJniEnvironment env;
    QAndroidJniObject data = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/Usbserial", "read", "()[B");
    const jint length = QAndroidJniObject::callStaticMethod<jint>(
        "org/cagnulen/qdomyoszwift/Usbserial", "readLen", "()I");
    if (env->ExceptionCheck() || length <= 0 || data.isNull()) {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return;
    }

    jbyteArray bytes = data.object<jbyteArray>();
    jbyte *raw = env->GetByteArrayElements(bytes, nullptr);
    if (raw) {
        receiveBuffer.append(reinterpret_cast<const char *>(raw), length);
        env->ReleaseByteArrayElements(bytes, raw, JNI_ABORT);
    }
#else
    Q_UNUSED(receiveBuffer);
#endif
}

QByteArray sportstechf75treadmill::commandFrame(int command, int data1, int data2) const {
    // This is the exact six-byte frame built by the APK's SerialPort class:
    // FB command data1 data2 checksum FC, checksum over command..data2.
    if ((command == kCommandRun || command == 0x0A) && data2 > 250) {
        data1 |= 0x80;
        data2 -= 250;
    }

    QByteArray frame;
    frame.append(static_cast<char>(kFrameStart));
    frame.append(static_cast<char>(command & 0xFF));
    frame.append(static_cast<char>(data1 & 0xFF));
    frame.append(static_cast<char>(data2 & 0xFF));
    frame.append(static_cast<char>((command + data1 + data2) & 0xFF));
    frame.append(static_cast<char>(kFrameEnd));
    return frame;
}

void sportstechf75treadmill::sendCommand(int command, int data1, int data2) {
#ifdef Q_OS_ANDROID
    if (!serialAvailable()) {
        return;
    }

    const QByteArray frame = commandFrame(command, data1, data2);
    QAndroidJniEnvironment env;
    jbyteArray bytes = env->NewByteArray(frame.size());
    env->SetByteArrayRegion(bytes, 0, frame.size(), reinterpret_cast<const jbyte *>(frame.constData()));
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "write", "([B)V", bytes);
    env->DeleteLocalRef(bytes);
    qDebug() << "Sportstech F75 TX" << frame.toHex(' ');
#else
    Q_UNUSED(command);
    Q_UNUSED(data1);
    Q_UNUSED(data2);
#endif
}

void sportstechf75treadmill::handleFrame(const QByteArray &frame) {
    if (frame.size() != 5 || static_cast<uint8_t>(frame.at(1)) != 0x60) {
        return;
    }

    const int incline = static_cast<uint8_t>(frame.at(2));
    const int speed = static_cast<uint8_t>(frame.at(3));
    const int checksum = (static_cast<uint8_t>(frame.at(1)) + incline + speed) & 0xFF;
    if (checksum != static_cast<uint8_t>(frame.at(4))) {
        qWarning() << "Sportstech F75 RX checksum error" << frame.toHex(' ');
        return;
    }

    // The APK's 0x60 telemetry frame carries integer km/h and incline values.
    speedSensor(speed);
    inclinationSensor(incline, incline);
}

void sportstechf75treadmill::parseFrames() {
    while (true) {
        const int start = receiveBuffer.indexOf(static_cast<char>(kTelemetryStart));
        if (start < 0) {
            receiveBuffer.clear();
            return;
        }
        if (start > 0) {
            receiveBuffer.remove(0, start);
        }
        if (receiveBuffer.size() < 2) {
            return;
        }

        const uint8_t function = static_cast<uint8_t>(receiveBuffer.at(1));
        int frameLength = 0;
        if (function == 0x60 || function == 0x21 || function == 0x55 || function == 0x10 || function == 0x40) {
            frameLength = 5;
        } else {
            // Unknown FD functions are discarded one byte at a time so that a
            // later valid frame can still be recovered from the stream.
            receiveBuffer.remove(0, 1);
            continue;
        }
        if (receiveBuffer.size() < frameLength) {
            return;
        }

        const QByteArray frame = receiveBuffer.left(frameLength);
        receiveBuffer.remove(0, frameLength);
        handleFrame(frame);
    }
}

void sportstechf75treadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0) {
        percentage = 0;
    }
    changeInclination(grade, percentage);
}

void sportstechf75treadmill::update() {
    if (!serialAvailable()) {
        return;
    }

    readSerial();
    parseFrames();

    QSettings settings;
    update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toDouble()));

    // Keep the standard QZ virtual-device behavior for treadmill profiles.
    if (!firstStateChanged && !hasVirtualDevice()) {
        const bool virtualDeviceEnabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        const bool forceBike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike).toBool();
        if (virtualDeviceEnabled) {
            if (!forceBike) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &sportstechf75treadmill::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &sportstechf75treadmill::changeInclinationRequested);
                setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &sportstechf75treadmill::changeInclinationRequested);
                setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
        }
        emit connectedAndDiscovered();
        firstStateChanged = true;
    }

    const int requestedSpeed = qBound(0, qRound(lastRequestedSpeed().value()), 25);
    const int requestedIncline = qBound(0, qRound(lastRequestedInclination().value()), 25);

    if (requestStart != -1) {
        sendCommand(kCommandStart, requestedIncline, requestedSpeed);
        requestStart = -1;
        lastCommandSpeed = requestedSpeed;
        lastCommandIncline = requestedIncline;
        commandActive = true;
        emit tapeStarted();
    }

    if (requestStop != -1) {
        sendCommand(kCommandStop, 0, 0);
        requestStop = -1;
        commandActive = false;
        lastCommandSpeed = -1;
        lastCommandIncline = -100;
    }

    if (commandActive && (requestedSpeed != lastCommandSpeed || requestedIncline != lastCommandIncline)) {
        sendCommand(kCommandRun, requestedIncline, requestedSpeed);
        lastCommandSpeed = requestedSpeed;
        lastCommandIncline = requestedIncline;
    }
}
