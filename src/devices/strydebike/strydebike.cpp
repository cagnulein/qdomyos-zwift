#include "strydebike.h"
#include "qzsettings.h"
#include "virtualdevices/virtualbike.h"
#include <QSettings>
#include <algorithm>
#include <cmath>
#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#endif

strydebike::strydebike(bool noHeartService, int8_t resistanceOffset, double resistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    QSettings settings;
    if (settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
        auto virtualBike = new virtualbike(this, true, noHeartService, resistanceOffset, resistanceGain);
        setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
    }
    connect(&refresh, &QTimer::timeout, this, &strydebike::update);
    crankTimer.start();
    refresh.start(200);
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/StrydeSerial", "start", "()V");
#else
    emit debug(QStringLiteral("Stryde serial requires Android"));
#endif
}

strydebike::~strydebike() {
    refresh.stop();
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/StrydeSerial", "stop", "()V");
#endif
}

void strydebike::update() {
#ifdef Q_OS_ANDROID
    for (int i = 0; i < 100; ++i) {
        QString event = QAndroidJniObject::callStaticObjectMethod(
            "org/cagnulen/qdomyoszwift/StrydeSerial", "nextEvent", "()Ljava/lang/String;").toString();
        if (event.isEmpty()) break;
        emit debug(QStringLiteral("Stryde serial ") + event);
        if (event == QLatin1String("OPEN")) {
            serialOpen = true;
            stale = false;
            lastFrame.start();
            if (!announced) { announced = true; emit connectedAndDiscovered(); }
        } else if (event == QLatin1String("CLOSED") || event.startsWith(QLatin1String("ERROR"))) {
            serialOpen = false;
            buffer.clear();
            Cadence = 0; m_watt = 0; Speed = 0;
        } else if (event.startsWith(QLatin1String("RX "))) {
            buffer.append(QByteArray::fromHex(event.mid(3).toLatin1()));
            while (buffer.size() >= 5) {
                QByteArray frame = buffer.left(5);
                buffer.remove(0, 5);
                const int cadence = static_cast<quint8>(frame.at(3));
                const int resistance = static_cast<quint8>(frame.at(4));
                const double rawPower = 11.0 * std::max(0.0, (cadence - 16.0) / 14.0) *
                    std::pow(33.0, resistance / 100.0) *
                    std::pow(7.0, ((cadence - 30.0) * resistance) / 70000.0);
                Cadence = cadence;
                Resistance = resistance;
                m_pelotonResistance = resistance;
                m_watt = std::min(65535.0, std::floor(rawPower));
                lastFrame.restart();
                stale = false;
                emit debug(QStringLiteral("Stryde frame %1 cadence=%2 resistance=%3 watts=%4 (experimental)")
                               .arg(QString::fromLatin1(frame.toHex(' '))).arg(cadence).arg(resistance).arg(watts()));
            }
        }
    }
#endif
    if (lastFrame.isValid() && lastFrame.elapsed() > 3000 && !stale) {
        stale = true;
        buffer.clear();
        Cadence = 0; m_watt = 0; Speed = 0;
        emit debug(QStringLiteral("Stryde serial timeout: telemetry stale"));
    }
    const qint64 elapsedMs = crankTimer.restart();
    if (Cadence.value() > 0) {
        crankRemainder += Cadence.value() * elapsedMs / 60000.0;
        while (crankRemainder >= 1.0) {
            CrankRevs++;
            LastCrankEventTime += static_cast<uint16_t>(61440.0 / Cadence.value());
            crankRemainder -= 1.0;
        }
    }
    update_metrics(false, watts());
    requestResistance = -1;
    requestPower = -1;
    if (requestStart != -1) { requestStart = -1; emit bikeStarted(); }
    if (requestStop != -1) requestStop = -1;
}
