#include "inspireic15dserialbike.h"

#include "qzsettings.h"
#include "virtualdevices/virtualbike.h"

#include <QSettings>

int inspireic15dserialbike::stockResistance(int controllerResistance) {
    return static_cast<int>(static_cast<double>(controllerResistance) / 2.5);
}

int inspireic15dserialbike::stockPower(int cadence, int resistance) {
    if (cadence <= 0 || resistance < 0)
        return 0;

    const double watts = (((static_cast<double>(cadence) * 0.132115398) - 0.334360151) * resistance) +
                         ((static_cast<double>(cadence) * 0.013644937) + 1.583101673);
    return watts < 2.0 ? 0 : static_cast<int>(watts);
}

inspireic15dserialbike::inspireic15dserialbike(const QString &serialPort, bool noHeartService,
                                               int8_t bikeResistanceOffset, double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);

    QSettings settings;
    const bool metricPolling =
        settings.value(QZSettings::inspire_ic15d_metric_polling,
                       QZSettings::default_inspire_ic15d_metric_polling).toBool();
    reader = new inspireic15dserialreader(this, serialPort, metricPolling);
    reader->start();

    if (settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
        emit debug(QStringLiteral("creating receive-only IC15D virtual bike interface..."));
        auto virtualBike = new virtualbike(this, true, noHeartService, bikeResistanceOffset, bikeResistanceGain);
        this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
    }

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &inspireic15dserialbike::update);
    refresh->start(200);
}

inspireic15dserialbike::~inspireic15dserialbike() {
    if (reader) {
        reader->requestInterruption();
        reader->wait(1000);
    }
}

void inspireic15dserialbike::update() {
    bool portOpen;
    qint64 totalBytes;
    QByteArray lastChunk;
    QString error;
    int cadence;
    int power;
    int resistance;
    qint64 validFrames;
    reader->snapshot(portOpen, totalBytes, lastChunk, error, cadence, power, resistance, validFrames);

    if (portOpen != lastPortOpen || error != lastError) {
        if (portOpen)
            emit debug(QStringLiteral("IC15D serial port opened at 19200 8-N-1"));
        if (!error.isEmpty())
            emit debug(QStringLiteral("IC15D serial error: ") + error);
        lastPortOpen = portOpen;
        lastError = error;
    }
    if (totalBytes != lastReportedBytes) {
        emit debug(QStringLiteral("IC15D serial bytes received: ") + QString::number(totalBytes) +
                   QStringLiteral("; latest: ") + QString::fromLatin1(lastChunk.toHex(' ')));
        lastReportedBytes = totalBytes;
    }

    if (validFrames != lastValidFrames) {
        const int convertedResistance = resistance >= 0 ? stockResistance(resistance) : -1;
        const int calculatedPower = stockPower(cadence, convertedResistance);
        if (cadence >= 0) {
            Cadence = cadence;
            Speed = static_cast<double>(cadence) * 0.37497622;
        }
        if (cadence >= 0 && convertedResistance >= 0)
            m_watt = calculatedPower;
        if (convertedResistance >= 0) {
            Resistance = convertedResistance;
            emit resistanceRead(Resistance.value());
        }
        emit debug(QStringLiteral("IC15D metrics: cadence=") + QString::number(cadence) +
                   QStringLiteral(" rpm, power=") + QString::number(calculatedPower) +
                   QStringLiteral(" W (controller raw=") + QString::number(power) +
                   QStringLiteral("), resistance=") + QString::number(convertedResistance) +
                   QStringLiteral(" (controller raw=") + QString::number(resistance) + QStringLiteral(")"));
        lastValidFrames = validFrames;
    }

    // This adapter never acts on FTMS control requests.
    requestResistance = -1;
    requestPower = -1;
    requestInclination = -100;
    requestStart = -1;
    requestStop = -1;

    update_metrics(false, watts());
    if (portOpen && !connectionAnnounced) {
        emit connectedAndDiscovered();
        connectionAnnounced = true;
    }
}

bool inspireic15dserialbike::connected() { return lastPortOpen; }

uint16_t inspireic15dserialbike::watts() { return static_cast<uint16_t>(m_watt.value()); }

resistance_t inspireic15dserialbike::resistanceFromPowerRequest(uint16_t) {
    return static_cast<resistance_t>(Resistance.value());
}

resistance_t inspireic15dserialbike::pelotonToBikeResistance(int pelotonResistance) {
    return static_cast<resistance_t>(pelotonResistance);
}
