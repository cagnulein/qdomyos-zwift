#include "sportstechbike_serial.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

#include <QDateTime>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

sportstechbike_serial::sportstechbike_serial(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                             double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    connect(refresh, &QTimer::timeout, this, &sportstechbike_serial::update);
    refresh->start(200ms);
}

bool sportstechbike_serial::openSerial() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "open",
                                              "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
    serialOpen = true;
    emit debug(QStringLiteral("Sportstech serial port opened"));
    return true;
#else
    return false;
#endif
}

void sportstechbike_serial::writeCharacteristic(const uint8_t *data, uint8_t data_len, const QString &info,
                                                bool disable_log, bool wait_for_response) {
#ifdef Q_OS_ANDROID
    if (!serialOpen && !openSerial()) {
        return;
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    QAndroidJniEnvironment env;
    jbyteArray d = env->NewByteArray(data_len);
    env->SetByteArrayRegion(d, 0, data_len, reinterpret_cast<const jbyte *>(data));
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Usbserial", "write", "([B)V", d);
    env->DeleteLocalRef(d);

    if (wait_for_response) {
        packetReceivedSinceWrite = false;
        for (int i = 0; i < 15; i++) {
            QThread::msleep(20);
            pollSerialInput();
            if (packetReceivedSinceWrite) {
                break;
            }
        }
    }
#else
    Q_UNUSED(data);
    Q_UNUSED(data_len);
    Q_UNUSED(info);
    Q_UNUSED(disable_log);
    Q_UNUSED(wait_for_response);
#endif
}

void sportstechbike_serial::pollSerialInput() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject dd =
        QAndroidJniObject::callStaticObjectMethod("org/cagnulen/qdomyoszwift/Usbserial", "read", "()[B");
    jint len = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Usbserial", "readLen", "()I");
    if (len <= 0 || !dd.isValid()) {
        return;
    }

    QAndroidJniEnvironment env;
    jbyteArray d = dd.object<jbyteArray>();
    if (!d) {
        return;
    }
    jbyte *b = env->GetByteArrayElements(d, nullptr);
    QByteArray bytes((const char *)b, len);
    env->ReleaseByteArrayElements(d, b, JNI_ABORT);
    processIncomingData(bytes);
#endif
}

void sportstechbike_serial::processIncomingData(const QByteArray &bytes) {
    if (bytes.isEmpty()) {
        return;
    }
    serialRxBuffer.append(bytes);

    while (true) {
        int start = serialRxBuffer.indexOf((char)0xF2);
        if (start < 0) {
            serialRxBuffer.clear();
            return;
        }
        if (start > 0) {
            serialRxBuffer.remove(0, start);
        }
        if (serialRxBuffer.size() < 4) {
            return;
        }

        const int frameLen = ((uint8_t)serialRxBuffer.at(2)) + 4;
        if (frameLen < 4 || frameLen > 64) {
            serialRxBuffer.remove(0, 1);
            continue;
        }
        if (serialRxBuffer.size() < frameLen) {
            return;
        }

        QByteArray frame = serialRxBuffer.left(frameLen);
        serialRxBuffer.remove(0, frameLen);
        processFrame(frame);
    }
}

void sportstechbike_serial::processFrame(const QByteArray &frame) {
    if (frame.size() < 4) {
        return;
    }

    uint8_t sum = 0;
    for (int i = 0; i < frame.size() - 1; i++) {
        sum += (uint8_t)frame.at(i);
    }
    if (sum != (uint8_t)frame.at(frame.size() - 1)) {
        emit debug(QStringLiteral("sportstech serial checksum mismatch: ") + frame.toHex(' '));
        return;
    }

    packetReceivedSinceWrite = true;
    emit packetReceived();
    emit debug(QStringLiteral(" << ") + frame.toHex(' '));

    if (frame.size() != 20) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    lastPacket = frame;

    const double speed = GetSpeedFromPacket(frame);
    const double cadence = GetCadenceFromPacket(frame);
    const double resistance = GetResistanceFromPacket(frame);
    const double kcal = GetKcalFromPacket(frame);
    const double watt = GetWattFromPacket(frame);
    const bool disableHrFromMachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        Heart = (uint8_t)KeepAwakeHelper::heart();
    } else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            uint8_t heart = (uint8_t)frame.at(11);
            if (heart == 0 || disableHrFromMachinery) {
                update_hr_from_external();
            } else {
                Heart = heart;
            }
        }
    }

    if (!firstCharChanged) {
        const int elapsedMs = qMax(1, lastTimeCharChanged.msecsTo(QTime::currentTime()));
        Distance += ((speed / 3600.0) / (1000.0 / elapsedMs));
    }

    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = speed;
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(), fabs(now.msecsTo(Speed.lastChanged()) / 1000.0),
            this->speedLimit());
    }

    Resistance = resistance;
    emit resistanceRead(Resistance.value());
    KCal = kcal;
    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = cadence;
    }
    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        m_watt = watt;
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current cadence: ") + QString::number(cadence));
    emit debug(QStringLiteral("Current resistance: ") + QString::number(resistance));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current watt: ") + QString::number(watt));
    emit debug(QStringLiteral("Current Elapsed from the bike (not used): ") +
               QString::number(GetElapsedFromPacket(frame)));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    lastTimeCharChanged = QTime::currentTime();
    firstCharChanged = false;
}

void sportstechbike_serial::btinit(bool startTape) {
    Q_UNUSED(startTape);

    const uint8_t initData1[] = {0xf2, 0xc0, 0x00, 0xb2};
    const uint8_t initData2[] = {0xf2, 0xc1, 0x05, 0x01, 0xff, 0xff, 0xff, 0xff, 0xb5};
    const uint8_t initData3[] = {0xf2, 0xc4, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc0};
    const uint8_t initData4[] = {0xf2, 0xc3, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbb};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
    writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);

    initDone = true;
}

void sportstechbike_serial::update() {
    if (!serialOpen && !openSerial()) {
        return;
    }

    pollSerialInput();

    if (initRequest) {
        initRequest = false;
        btinit(false);
        emit connectedAndDiscovered();
        return;
    }

    if (!initDone) {
        return;
    }

    update_metrics(false, 0);

    if (sec1update++ == (1000 / refresh->interval())) {
        sec1update = 0;
    }

    uint8_t noOpData[] = {0xf2, 0xc3, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbe};
    if (requestResistance < 0) {
        requestResistance = 0;
    }
    if (requestResistance > maxResistance()) {
        requestResistance = maxResistance();
    }
    noOpData[4] = (uint8_t)requestResistance;
    noOpData[10] += (uint8_t)requestResistance;
    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
    pollSerialInput();
}

void sportstechbike_serial::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    if (openSerial()) {
        initRequest = true;
    } else {
        emit debug(QStringLiteral("Sportstech serial open failed"));
        emit disconnected();
    }
}

bool sportstechbike_serial::connected() { return serialOpen && initDone; }

uint16_t sportstechbike_serial::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

resistance_t sportstechbike_serial::maxResistance() {
    QSettings settings;
    const bool sportstechEsx500 =
        settings.value(QZSettings::sportstech_esx500, QZSettings::default_sportstech_esx500).toBool();
    return sportstechEsx500 ? 10 : 24;
}

uint16_t sportstechbike_serial::wattsFromResistance(double resistance) {
    // Polynomial regression valid for Sportstech protocol bikes.
    const double intercept = 14.4968;
    const double b1 = -4.1878;
    const double b2 = -0.5051;
    const double b3 = 0.00387;
    const double b4 = 0.2392;
    const double b5 = 0.01108;
    const double cadence = Cadence.value();

    const double power = intercept + (b1 * resistance) + (b2 * cadence) + (b3 * resistance * resistance) +
                         (b4 * resistance * cadence) + (b5 * cadence * cadence);
    return (uint16_t)qMax(0.0, power);
}

resistance_t sportstechbike_serial::resistanceFromPowerRequest(uint16_t power) {
    if (Cadence.value() == 0) {
        return 1;
    }

    for (resistance_t i = 1; i < maxResistance(); i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            return i;
        }
    }

    if (power < wattsFromResistance(1)) {
        return 1;
    }
    return maxResistance();
}

uint16_t sportstechbike_serial::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedDataSec = (uint8_t)packet.at(4);
    uint16_t convertedDataMin = (uint8_t)packet.at(3);
    return (convertedDataMin * 60U) + convertedDataSec;
}

double sportstechbike_serial::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = ((uint8_t)packet.at(12) << 8) | (uint8_t)packet.at(13);
    return (double)(convertedData) / 10.0f;
}

double sportstechbike_serial::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = ((uint8_t)packet.at(7) << 8) | (uint8_t)packet.at(8);
    return (double)(convertedData);
}

double sportstechbike_serial::GetWattFromPacket(const QByteArray &packet) {
    uint16_t convertedData = ((uint8_t)packet.at(9) << 8) | (uint8_t)packet.at(10);
    return (double)(convertedData);
}

double sportstechbike_serial::GetCadenceFromPacket(const QByteArray &packet) {
    return (uint8_t)packet.at(17);
}

double sportstechbike_serial::GetResistanceFromPacket(const QByteArray &packet) {
    return (uint8_t)packet.at(15);
}
