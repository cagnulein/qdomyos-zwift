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
constexpr int kKeyEventFunction = 0x20;
constexpr int kKeyEventStart = 0x67;
constexpr int kKeyEventStop = 0x68;
constexpr int kKeyEventInclineUpObserved = 0x0A;
constexpr int kKeyEventInclineUpVendor = 0x0D;
constexpr int kCommandStart = 0xA2;
constexpr int kCommandStop = 0x00;
// The vendor sends A2 as a one-second countdown: 03, 02, 01, 00.
constexpr int kStartCountdownSeconds = 3;
constexpr int kTimerTicksPerSecond = 10;
constexpr int kInitialRunningFrames = 10;
constexpr int kSpeedProtocolScale = 10;
}

sportstechf75treadmill::sportstechf75treadmill(bool noHeartService) {
    this->noHeartService = noHeartService;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    qDebug() << "Sportstech F75 diagnostic command tracing enabled";

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
    sendStartupCommand();
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
    if (env->ExceptionCheck() || length <= 0) {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return;
    }

    jbyteArray bytes = data.object<jbyteArray>();
    if (!bytes) {
        return;
    }
    jbyte *raw = env->GetByteArrayElements(bytes, nullptr);
    if (raw) {
        const QByteArray chunk(reinterpret_cast<const char *>(raw), length);
        qDebug() << "Sportstech F75 RX chunk" << chunk.toHex(' ');
        receiveBuffer.append(chunk);
        qDebug() << "Sportstech F75 RX buffer" << receiveBuffer.toHex(' ');
        env->ReleaseByteArrayElements(bytes, raw, JNI_ABORT);
    }
#else
    Q_UNUSED(receiveBuffer);
#endif
}

QByteArray sportstechf75treadmill::startupFrame() const {
    // The vendor APK sends this immediately after opening the UART:
    // SendOtherCommand(0x20, new int[12]).  Its frame is
    // FB 5A title payload[12] checksum FC, with checksum over bytes 1..14.
    QByteArray frame(17, static_cast<char>(0));
    frame[0] = static_cast<char>(kFrameStart);
    frame[1] = static_cast<char>(0x5A);
    frame[2] = static_cast<char>(0x20);
    frame[15] = static_cast<char>((0x5A + 0x20) & 0xFF);
    frame[16] = static_cast<char>(kFrameEnd);
    return frame;
}

void sportstechf75treadmill::sendStartupCommand() {
#ifdef Q_OS_ANDROID
    if (serialAvailable()) {
        sendFrame(startupFrame());
    }
#endif
}

QByteArray sportstechf75treadmill::runningProtocolFrame(int protocolSpeed, int incline, bool allowZero) const {
    // The vendor sends this A0 running-parameter frame after the A2 countdown.
    // The first three values are workout counters; zero is safe for the first
    // control probe while the proven speed/incline fields remain at offsets 8/9.
    // Frame: FB A0 time[2] distance[2] calories[2] speed incline checksum FC.
    QByteArray frame(12, static_cast<char>(0));
    frame[0] = static_cast<char>(kFrameStart);
    frame[1] = static_cast<char>(0xA0);
    frame[8] = static_cast<char>(qBound(allowZero ? 0 : kSpeedProtocolScale, protocolSpeed, 255));
    frame[9] = static_cast<char>(qBound(0, incline, 255));

    int checksum = 0;
    for (int i = 1; i <= 9; ++i) {
        checksum += static_cast<uint8_t>(frame.at(i));
    }
    frame[10] = static_cast<char>(checksum & 0xFF);
    frame[11] = static_cast<char>(kFrameEnd);
    return frame;
}

QByteArray sportstechf75treadmill::runningFrame(double speed, int incline, bool allowZero) const {
    // QZ keeps speed in km/h; the A0 wire field is deci-km/h.
    return runningProtocolFrame(qRound(speed * kSpeedProtocolScale), incline, allowZero);
}

void sportstechf75treadmill::sendRunningProtocolCommand(int protocolSpeed, int incline, bool allowZero) {
#ifdef Q_OS_ANDROID
    if (serialAvailable()) {
        sendFrame(runningProtocolFrame(protocolSpeed, incline, allowZero));
    }
#else
    Q_UNUSED(protocolSpeed);
    Q_UNUSED(incline);
    Q_UNUSED(allowZero);
#endif
}

void sportstechf75treadmill::sendRunningCommand(double speed, int incline, bool allowZero) {
#ifdef Q_OS_ANDROID
    if (serialAvailable()) {
        sendFrame(runningFrame(speed, incline, allowZero));
    }
#else
    Q_UNUSED(speed);
    Q_UNUSED(incline);
    Q_UNUSED(allowZero);
#endif
}

void sportstechf75treadmill::publishCommandedMetrics(double speed, int incline) {
    // The vendor A0 frame carries the controller state that the vendor app is
    // commanding. This console does not provide a non-zero measured speed or
    // incline frame while running, so expose that state to QZ as a display
    // proxy until a real RX measurement is identified. Keep the distinction
    // explicit in the log and never leave stale values after pause/stop.
    if (currentSpeed().value() == speed && currentInclination().value() == incline) {
        return;
    }
    qDebug() << "Sportstech F75 commanded metrics proxy speed" << speed << "incline" << incline;
    speedSensor(speed);
    inclinationSensor(incline, incline);
}

QByteArray sportstechf75treadmill::commandFrame(int command, int data1, int data2) const {
    // This is the exact six-byte frame built by the APK's SerialPort class:
    // FB command data1 data2 checksum FC, checksum over command..data2.
    // Keep zero exclusive to the explicit stop command. This guard belongs at
    // the frame encoder so every active caller is protected, even if a future
    // path bypasses update()'s requested-speed normalization.
    if (command == 0x0A && data2 > 250) {
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
    qDebug() << "Sportstech F75 commandFrame command" << command << "data1" << data1 << "data2" << data2
             << "frame" << frame.toHex(' ');
    return frame;
}

void sportstechf75treadmill::sendFrame(const QByteArray &frame) {
#ifdef Q_OS_ANDROID
    if (!serialAvailable()) {
        return;
    }

    QAndroidJniEnvironment env;
    qDebug() << "Sportstech F75 sendFrame before JNI write" << frame.size() << "bytes" << frame.toHex(' ');
    jbyteArray bytes = env->NewByteArray(frame.size());
    env->SetByteArrayRegion(bytes, 0, frame.size(), reinterpret_cast<const jbyte *>(frame.constData()));
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "write", "([B)V", bytes);
    env->DeleteLocalRef(bytes);
    qDebug() << "Sportstech F75 TX" << frame.toHex(' ');
#else
    Q_UNUSED(frame);
#endif
}

void sportstechf75treadmill::sendCommand(int command, int data1, int data2) {
    sendFrame(commandFrame(command, data1, data2));
}

void sportstechf75treadmill::handleFrame(const QByteArray &frame) {
    qDebug() << "Sportstech F75 RX candidate" << frame.toHex(' ');
    if (frame.size() != 6 || static_cast<uint8_t>(frame.at(0)) != kTelemetryStart ||
        static_cast<uint8_t>(frame.at(5)) != 0xFE) {
        qDebug() << "Sportstech F75 RX ignored frame" << frame.toHex(' ');
        return;
    }

    const int function = static_cast<uint8_t>(frame.at(1));
    if (function == kKeyEventFunction) {
        const int payload = static_cast<uint8_t>(frame.at(2));
        const int key = static_cast<uint8_t>(frame.at(3));
        const int reportedChecksum = static_cast<uint8_t>(frame.at(4));
        const int expectedChecksum = (function + payload + key) & 0xFF;
        // The F75 uses the normal checksum for the other FD 20 events, but
        // the observed Start/Stop frames use key+payload instead:
        // FD 20 01 67 68 FE and FD 20 01 68 69 FE.
        const bool startStopChecksum =
            (key == kKeyEventStart || key == kKeyEventStop) && reportedChecksum == ((payload + key) & 0xFF);
        // Tristan's isolated F75 capture reported 0x0A for Incline+, while
        // the vendor parser documents 0x0D. That frame carries the checksum
        // of the vendor alias (0x2E), so accept that specific observed form.
        const bool observedInclineUpChecksum =
            key == kKeyEventInclineUpObserved && reportedChecksum ==
                ((function + payload + kKeyEventInclineUpVendor) & 0xFF);
        if (payload != 0x01 ||
            (reportedChecksum != expectedChecksum && !startStopChecksum && !observedInclineUpChecksum)) {
            qWarning() << "Sportstech F75 key-event checksum error" << frame.toHex(' ')
                       << "expected" << expectedChecksum;
            return;
        }

        qDebug() << "Sportstech F75 physical key event" << Qt::hex << key << Qt::dec;
        if (key == kKeyEventStart) {
            // QZ's Start action is also its pause/resume toggle. The UI slot
            // receives this signal with send_event_to_device=false, so the
            // physical Start/Resume action is not echoed to the UART.
            emit buttonHWStart();
        } else if (key == kKeyEventStop) {
            // Stop is terminal in QZ, unlike the Start/Pause toggle.
            emit buttonHWStop();
        } else if (key == kKeyEventInclineUpObserved || key == kKeyEventInclineUpVendor) {
            // 0x0A is the empirical F75 code and 0x0D is the vendor-parser
            // code. Both represent one Incline+ action, never two steps.
            emit buttonHWInclineUp();
        } else {
            qDebug() << "Sportstech F75 unhandled physical key event" << Qt::hex << key << Qt::dec;
        }
        return;
    }

    if (function != 0x60) {
        qDebug() << "Sportstech F75 RX ignored function" << Qt::hex << function << Qt::dec;
        return;
    }

    const int incline = static_cast<uint8_t>(frame.at(2));
    const int protocolSpeed = static_cast<uint8_t>(frame.at(3));
    const int checksum = (static_cast<uint8_t>(frame.at(1)) + incline + protocolSpeed) & 0xFF;
    if (checksum != static_cast<uint8_t>(frame.at(4))) {
        qWarning() << "Sportstech F75 RX checksum error" << frame.toHex(' ');
        return;
    }

    // The APK's six-byte 0x60 telemetry frame is:
    // FD 60 incline speed checksum FE. Speed is in deci-km/h on the wire.
    const double speed = static_cast<double>(protocolSpeed) / kSpeedProtocolScale;
    qDebug() << "Sportstech F75 RX telemetry speed" << speed << "protocol" << protocolSpeed << "incline" << incline;
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
        if (function == 0x20 || function == 0x60 || function == 0x21 || function == 0x55 || function == 0x10 ||
            function == 0x40 || function == 0x75 || function == 0x92) {
            frameLength = 6;
        } else {
            // Unknown FD functions are discarded one byte at a time so that a
            // later valid frame can still be recovered from the stream.
            qDebug() << "Sportstech F75 RX unknown FD function" << Qt::hex << function
                     << Qt::dec << receiveBuffer.toHex(' ');
            receiveBuffer.remove(0, 1);
            continue;
        }
        if (receiveBuffer.size() < frameLength) {
            qDebug() << "Sportstech F75 RX partial frame" << receiveBuffer.toHex(' ')
                     << "need" << frameLength;
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

    const double rawRequestedSpeed = lastRequestedSpeed().value();
    const double requestedSpeed = qBound(0.0, rawRequestedSpeed, 25.0);
    const int requestedIncline = qBound(0, qRound(lastRequestedInclination().value()), 25);
    // The vendor app starts the belt with a non-zero target speed.  QZ can
    // otherwise reach Start with a zero target, which produces an A2 start
    // frame that leaves the controller stopped.  Keep the minimum only for
    // the active command path; a later user speed request replaces it.
    const double commandSpeed = qMax(0.1, requestedSpeed);

    if (requestStart != -1) {
        qDebug() << "Sportstech F75 START speed raw" << rawRequestedSpeed << "requested" << requestedSpeed
                 << "command" << commandSpeed << "incline" << requestedIncline
                 << "countdown" << kStartCountdownSeconds;
        // The vendor starts with A2 00 03, then decrements once per second.
        sendCommand(kCommandStart, 0, kStartCountdownSeconds);
        requestStart = -1;
        commandActive = false;
        startCountdownValue = kStartCountdownSeconds - 1;
        startCountdownTicks = 0;
        runningCommandTicks = 0;
        runningBurstFrames = 0;
        stopRunningFramePending = false;
        stopRampPending = false;
        pauseRampPending = false;
        pauseRampProtocolSpeed = -1;
        pauseRampTicks = 0;
        stopRunningFrameTicks = 0;
        lastCommandSpeed = commandSpeed;
        lastCommandIncline = requestedIncline;
        emit tapeStarted();
    }

    bool stopRequestedThisTick = false;
    if (requestStop != -1) {
        const bool pauseRequested = requestPause != -1;
        qDebug() << "Sportstech F75 STOP" << (pauseRequested ? "pause" : "stop");
        if (pauseRequested) {
            // The vendor pauses by ramping the A0 speed field down by one
            // deci-km/h every 100 ms. It does not send FB 00 at the start of
            // this ramp; that command is reserved for the final Stop action.
            pauseRampProtocolSpeed = qBound(0, qRound(lastCommandSpeed * kSpeedProtocolScale), 255);
            pauseRampIncline = qBound(0, lastCommandIncline, 255);
            pauseRampTicks = 0;
            pauseRampPending = true;
            stopRampPending = false;
            stopRunningFramePending = false;
        } else {
            // A normal F75 stop uses the same smooth A0 speed ramp as pause.
            // Keep the final FB 00 command until the belt has reached zero;
            // an emergency-stop path, if added later, can still send FB 00
            // immediately without reusing this normal-stop state.
            pauseRampProtocolSpeed = qBound(0, qRound(lastCommandSpeed * kSpeedProtocolScale), 255);
            pauseRampIncline = qBound(0, lastCommandIncline, 255);
            pauseRampTicks = 0;
            pauseRampPending = true;
            stopRampPending = true;
            stopRunningFramePending = false;
        }
        requestStop = -1;
        requestPause = -1;
        commandActive = false;
        startCountdownValue = -1;
        startCountdownTicks = 0;
        runningCommandTicks = 0;
        runningBurstFrames = 0;
        lastCommandSpeed = -1.0;
        lastCommandIncline = -100;
    }

    if (stopRunningFramePending && !stopRequestedThisTick && ++stopRunningFrameTicks >= 1) {
        sendRunningCommand(0, 0, true);
        stopRunningFramePending = false;
        stopRunningFrameTicks = 0;
    }

    if (pauseRampPending) {
        if (pauseRampProtocolSpeed > 0) {
            --pauseRampProtocolSpeed;
            if (pauseRampProtocolSpeed == 0) {
                // The vendor marks the paused state with incline=0xFF after
                // the speed ramp reaches zero.
                sendRunningProtocolCommand(0, 0xFF, true);
                if (stopRampPending) {
                    sendCommand(kCommandStop, 0, 0);
                    stopRampPending = false;
                    pauseRampPending = false;
                }
            } else {
                sendRunningProtocolCommand(pauseRampProtocolSpeed, pauseRampIncline, true);
            }
            pauseRampTicks = 0;
        } else if (stopRampPending) {
            sendRunningProtocolCommand(0, 0xFF, true);
            sendCommand(kCommandStop, 0, 0);
            stopRampPending = false;
            pauseRampPending = false;
            pauseRampTicks = 0;
        } else if (++pauseRampTicks >= kTimerTicksPerSecond) {
            sendRunningProtocolCommand(0, 0xFF, true);
            pauseRampTicks = 0;
        }
    }

    // Do not send A0 during the countdown. The vendor emits A2 02, A2 01 and
    // A2 00 at roughly one-second intervals before the first running frame.
    if (!commandActive && startCountdownValue >= 0) {
        if (++startCountdownTicks >= kTimerTicksPerSecond) {
            sendCommand(kCommandStart, 0, startCountdownValue);
            startCountdownTicks = 0;
            if (startCountdownValue == 0) {
                startCountdownValue = -1;
                commandActive = true;
                runningCommandTicks = 0;
                runningBurstFrames = kInitialRunningFrames;
                qDebug() << "Sportstech F75 countdown complete; starting A0 running frames";
            } else {
                --startCountdownValue;
            }
        }
    }

    bool runningFrameSent = false;
    if (commandActive && (commandSpeed != lastCommandSpeed || requestedIncline != lastCommandIncline)) {
        qDebug() << "Sportstech F75 RUN speed raw" << rawRequestedSpeed << "requested" << requestedSpeed
                 << "command" << commandSpeed << "incline" << requestedIncline;
        sendRunningCommand(commandSpeed, requestedIncline);
        runningCommandTicks = 0;
        runningBurstFrames = 0;
        lastCommandSpeed = commandSpeed;
        lastCommandIncline = requestedIncline;
        runningFrameSent = true;
    }

    // The vendor sends A0 every ~100 ms for the first ten frames, then every
    // ~1 s. This cadence is part of the start/run contract for this console.
    if (commandActive && !runningFrameSent) {
        if (runningBurstFrames > 0) {
            sendRunningCommand(commandSpeed, requestedIncline);
            --runningBurstFrames;
            runningCommandTicks = 0;
        } else if (++runningCommandTicks >= kTimerTicksPerSecond) {
            sendRunningCommand(commandSpeed, requestedIncline);
            runningCommandTicks = 0;
        }
    }

    publishCommandedMetrics(commandActive ? commandSpeed : 0, commandActive ? requestedIncline : 0);
}
