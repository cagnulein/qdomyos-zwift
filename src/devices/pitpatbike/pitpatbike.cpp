#include "pitpatbike.h"
#include "homeform.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

namespace {
uint16_t pitpatReadBE16(const QByteArray &packet, int offset) {
    return (static_cast<uint16_t>(static_cast<uint8_t>(packet.at(offset))) << 8) |
           static_cast<uint16_t>(static_cast<uint8_t>(packet.at(offset + 1)));
}

uint8_t pitpatXorCrc(const QByteArray &packet) {
    uint8_t crc = 0;
    for (int i = 1; i < packet.size() - 2; ++i)
        crc ^= static_cast<uint8_t>(packet.at(i));
    return crc;
}

uint16_t pitpatEstimatedWatts(double cadence, double resistance) {
    if (cadence <= 0.0)
        return 0;

    // The S01PRO does not expose power in its legacy telemetry. Until a
    // device-specific curve is calibrated, reuse the default Echelon Connect
    // Sport 1-32 resistance table. Both bikes expose the same 32 resistance
    // levels, making this a useful provisional estimate for virtual cycling apps.
    static const double wattTable[33][11] = {
        {0.0, 1.0, 2.2, 4.8, 9.5, 13.6, 16.7, 22.6, 26.3, 29.2, 47.0},
        {0.0, 1.0, 2.2, 4.8, 9.5, 13.6, 16.7, 22.6, 26.3, 29.2, 47.0},
        {0.0, 1.3, 3.0, 5.4, 10.4, 14.5, 18.5, 24.6, 27.6, 33.5, 49.5},
        {0.0, 1.5, 3.7, 6.7, 11.7, 15.9, 19.6, 26.1, 30.8, 35.2, 51.2},
        {0.0, 1.6, 4.7, 7.5, 13.7, 17.6, 22.6, 29.0, 36.9, 42.6, 57.2},
        {0.0, 1.8, 5.2, 8.0, 14.8, 19.1, 23.5, 32.5, 37.5, 50.8, 61.8},
        {0.0, 1.9, 5.7, 8.7, 15.6, 20.2, 25.5, 33.5, 39.6, 52.1, 65.3},
        {0.0, 2.0, 6.2, 9.5, 16.8, 21.8, 28.1, 37.0, 42.8, 57.8, 68.4},
        {0.0, 2.1, 6.8, 10.8, 18.2, 23.6, 29.5, 40.0, 47.6, 60.5, 72.1},
        {0.0, 2.2, 7.3, 11.5, 19.3, 26.3, 33.5, 45.3, 51.8, 66.7, 76.8},
        {0.0, 2.4, 7.9, 12.7, 20.8, 29.8, 37.6, 52.2, 56.2, 73.5, 83.6},
        {0.0, 2.6, 8.5, 13.5, 23.5, 33.6, 41.9, 55.1, 59.0, 78.6, 89.7},
        {0.0, 2.7, 9.1, 14.2, 25.6, 35.4, 45.3, 57.3, 62.8, 81.3, 95.0},
        {0.0, 2.9, 9.6, 16.8, 29.1, 37.5, 49.6, 62.5, 69.0, 84.7, 99.3},
        {0.0, 3.0, 10.0, 22.3, 31.2, 40.3, 51.8, 65.0, 70.0, 92.6, 108.2},
        {0.0, 3.2, 10.4, 24.0, 36.6, 42.5, 56.3, 74.0, 85.0, 98.2, 123.5},
        {0.0, 3.5, 10.9, 25.1, 38.5, 47.6, 65.4, 83.0, 93.0, 114.8, 136.8},
        {0.0, 3.7, 11.5, 26.0, 41.0, 53.2, 71.6, 90.0, 100.0, 121.7, 149.2},
        {0.0, 4.0, 12.1, 27.5, 43.6, 56.0, 82.3, 101.0, 113.6, 143.0, 162.8},
        {0.0, 4.2, 12.7, 29.7, 46.7, 64.2, 87.9, 109.2, 128.9, 154.0, 172.3},
        {0.0, 4.5, 13.7, 32.0, 50.0, 71.8, 95.6, 113.8, 135.6, 165.0, 185.0},
        {0.0, 4.7, 14.9, 34.5, 54.2, 77.0, 100.7, 127.0, 147.6, 180.0, 200.0},
        {0.0, 5.0, 15.8, 36.5, 58.3, 83.4, 110.1, 136.0, 168.1, 196.0, 213.5},
        {0.0, 5.6, 17.0, 39.5, 64.3, 88.8, 123.4, 154.0, 182.0, 210.0, 235.0},
        {0.0, 6.1, 18.2, 44.0, 70.7, 99.9, 133.3, 166.0, 198.0, 230.0, 253.5},
        {0.0, 6.8, 19.4, 49.0, 79.0, 108.8, 147.2, 185.0, 217.0, 255.2, 278.0},
        {0.0, 7.6, 22.0, 54.8, 88.0, 127.0, 167.0, 212.0, 244.0, 287.0, 305.0},
        {0.0, 8.7, 26.0, 62.0, 100.0, 145.0, 190.0, 242.0, 281.0, 315.1, 350.0},
        {0.0, 9.2, 30.0, 71.0, 114.4, 161.6, 215.1, 275.1, 317.0, 358.5, 390.0},
        {0.0, 9.8, 36.0, 82.5, 134.5, 195.3, 252.5, 313.7, 360.0, 420.3, 460.0},
        {0.0, 10.5, 43.0, 95.0, 157.1, 228.4, 300.1, 374.1, 403.8, 487.8, 540.0},
        {0.0, 12.5, 48.0, 99.3, 162.2, 232.9, 310.4, 400.3, 435.5, 530.5, 589.0},
        {0.0, 13.0, 53.0, 102.0, 170.3, 242.0, 320.0, 427.9, 475.2, 570.0, 625.0}};

    int level = static_cast<int>(resistance + 0.5);
    if (level < 1)
        level = 1;
    else if (level > 32)
        level = 32;

    const double *wattsOfLevel = wattTable[level];
    int wattStep = static_cast<int>(cadence / 10.0);
    double watts = 0.0;
    if (wattStep >= 10) {
        watts = (cadence / 100.0) * wattsOfLevel[10];
    } else {
        const double wattBase = wattsOfLevel[wattStep];
        watts = (((wattsOfLevel[wattStep + 1] - wattBase) / 10.0) *
                 (static_cast<int>(cadence) % 10)) +
                wattBase;
    }

    if (watts <= 0.0)
        return 0;
    if (watts >= 65535.0)
        return 65535;
    return static_cast<uint16_t>(watts + 0.5);
}
} // namespace

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

pitpatbike::pitpatbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                         double bikeResistanceGain) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &pitpatbike::update);
    refresh->start(200ms);
}

void pitpatbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                              bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (!gattCommunicationChannelService || !m_control) {
        qDebug() << QStringLiteral("writeCharacteristic error because the BLE service/controller is missing");
        return;
    }

    // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
    // one for the resistance changed event (spontaneous), and one for the other ones.
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                        QStringLiteral(" // ") + info;
    }

    loop.exec();
}

void pitpatbike::forceResistance(resistance_t requestResistance) {
    uint8_t noOpData[] = {0x6a, 0x06, 0x51, 0x82, 0x01, 0x01, 0xd5, 0x43};
    noOpData[5] = requestResistance;

    uint8_t crc = 0;
    for(int i = 0; i < sizeof(noOpData) - 2; i++) {
        crc ^= noOpData[i];
    }
    noOpData[6] = crc ^ 0x6A;

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("force resistance"), false, true);
}

void pitpatbike::sendPoll() {
    uint8_t noOpData[] = {0x6a, 0x05, 0xfd, 0xf8, 0x43};
    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
}

void pitpatbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        update_metrics(true, watts());

        // sending poll every 2 seconds
        if (sec1Update++ >= (1000 / refresh->interval())) {
            sec1Update = 0;
            sendPoll();
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance)
                requestResistance = max_resistance;
            else if (requestResistance <= 0)
                requestResistance = 1;

            if (requestResistance != currentResistance().value()) {
                qDebug() << QStringLiteral("writing resistance ") + QString::number(requestResistance);
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestStart != -1) {
            qDebug() << QStringLiteral("starting...");

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            qDebug() << QStringLiteral("stopping...");
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void pitpatbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString());
}

resistance_t pitpatbike::pelotonToBikeResistance(int pelotonResistance) {
    for (resistance_t i = 1; i < max_resistance; i++) {
        if (bikeResistanceToPeloton(i) <= pelotonResistance && bikeResistanceToPeloton(i + 1) > pelotonResistance) {
            return i;
        }
    }
    if (pelotonResistance < bikeResistanceToPeloton(1))
        return 1;
    else
        return max_resistance;
}

double pitpatbike::bikeResistanceToPeloton(double resistance) {
    QSettings settings;
    // 0,0097x3 - 0,4972x2 + 10,126x - 37,08
    double p = ((pow(resistance, 3) * 0.0097) - (0.4972 * pow(resistance, 2)) + (10.126 * resistance) - 37.08);
    if (p < 0) {
        p = 0;
    }
    return (p * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
           settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
}

void pitpatbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    // PitPat's current three-in-one protocol uses the legacy 0x6a framing.
    // The startup frame is 34 bytes and advertises the actual bike layout.
    if (newValue.size() >= 4 && static_cast<uint8_t>(newValue.at(0)) == 0x6a &&
        static_cast<uint8_t>(newValue.at(3)) == 0x2c) {
        legacyProtocol = true;
        if (newValue.size() < 34) {
            qDebug() << QStringLiteral("PitPat startup frame is too short") << newValue.size();
            return;
        }

        qDebug() << QStringLiteral("PitPat startup: min resistance ") +
                        QString::number(pitpatReadBE16(newValue, 9)) + QStringLiteral(", max resistance ") +
                        QString::number(pitpatReadBE16(newValue, 11)) + QStringLiteral(", serial ") +
                        QString::fromLatin1(newValue.constData() + 13, 16) + QStringLiteral(", version ") +
                        QString::number(static_cast<uint8_t>(newValue.at(29))) + QStringLiteral(", model ") +
                        QString::number(static_cast<uint8_t>(newValue.at(31))) +
                        QStringLiteral(", crc ") +
                        (pitpatXorCrc(newValue) == static_cast<uint8_t>(newValue.at(newValue.size() - 2))
                             ? QStringLiteral("ok")
                             : QStringLiteral("invalid"));

        // The official PitPat app answers the 0x2c startup frame before it starts
        // polling or sending resistance commands. Until this handshake is ACKed,
        // the S01PRO remains in its startup/scan state.
        if (!initDone) {
            uint8_t legacyHandshake[] = {0x6a, 0x05, 0x50, 0x1b, 0x01, 0x4f, 0x43};
            writeCharacteristic(legacyHandshake, sizeof(legacyHandshake), QStringLiteral("legacy handshake"), false,
                                false);
        }
        return;
    }

    // ACK observed in the official app immediately after the legacy handshake.
    if (legacyProtocol && newValue == QByteArray::fromHex("6a06b01b0100ac43")) {
        qDebug() << QStringLiteral("PitPat legacy handshake acknowledged");
        initDone = true;
        sec1Update = 0;
        return;
    }

    if (legacyProtocol) {
        // Legacy telemetry fields are documented by the APK's parser and were
        // confirmed against a live HCI snoop from the official PitPat app.
        if (newValue.size() < 28 || static_cast<uint8_t>(newValue.at(0)) != 0x6a ||
            static_cast<uint8_t>(newValue.at(3)) != 0x02)
            return;

        // If the ACK notification was lost but telemetry has already started,
        // the bike necessarily accepted the handshake, so it is safe to proceed.
        if (!initDone) {
            qDebug() << QStringLiteral("PitPat legacy telemetry received; handshake accepted");
            initDone = true;
            sec1Update = 0;
        }

        if (pitpatXorCrc(newValue) != static_cast<uint8_t>(newValue.at(newValue.size() - 2)))
            qDebug() << QStringLiteral("PitPat telemetry CRC mismatch") << newValue.toHex(' ');

        QSettings settings;
        const uint8_t cadence = static_cast<uint8_t>(newValue.at(25));
        const uint16_t rawSpeed = pitpatReadBE16(newValue, 23);

        Resistance = static_cast<uint8_t>(newValue.at(5));
        m_pelotonResistance = m_pelotonResistance.value();
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            Cadence = cadence;
        }

        // Model 2 reports velocity in mm/s; QZ speed is expressed in km/h.
        Speed = static_cast<double>(rawSpeed) / 1000.0;
        Distance = static_cast<double>(pitpatReadBE16(newValue, 10)) / 1000.0;
        KCal = static_cast<double>(pitpatReadBE16(newValue, 12)) / 10.0;
        Heart = static_cast<uint8_t>(newValue.at(18));
        CrankRevs = pitpatReadBE16(newValue, 16);
        LastCrankEventTime += cadence > 0 ? static_cast<uint16_t>(1024.0 / (static_cast<double>(cadence) / 60.0)) : 0;

        // The S01PRO does not report watts. Use the Echelon Connect Sport 1-32
        // table as a provisional estimate until a PitPat-specific curve is calibrated.
        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            m_watt = pitpatEstimatedWatts(Cadence.value(), Resistance.value());
        }
        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

        qDebug() << QStringLiteral("PitPat legacy metrics: resistance ") + QString::number(Resistance.value()) +
                        QStringLiteral(", speed ") + QString::number(Speed.value()) + QStringLiteral(", distance ") +
                        QString::number(Distance.value()) + QStringLiteral(", cadence ") + QString::number(Cadence.value()) +
                        QStringLiteral(", watts ") + QString::number(watts()) + QStringLiteral(", calories ") +
                        QString::number(KCal.value()) + QStringLiteral(", heart ") + QString::number(Heart.value()) +
                        QStringLiteral(", crank revs ") + QString::number(CrankRevs);
        return;
    }

    if (newValue.length() != 30) {
        return;
    }

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

    double distance = GetDistanceFromPacket(newValue);

    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = ((uint8_t)newValue.at(25));
    }
    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = 0.37497622 * ((double)Cadence.value());
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }

    m_watt = (uint16_t)((uint8_t)newValue.at(24)) + ((uint16_t)((uint8_t)newValue.at(23)) << 8);
    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        Heart = (uint8_t)KeepAwakeHelper::heart();
    } else
#endif
    {
        if (heartRateBeltName.startsWith(QLatin1String("Disabled"))) {
            update_hr_from_external();
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    // these useless lines are needed to calculate the AVG resistance and AVG peloton resistance since
    // echelon just send the resistance values when it changes
    Resistance = newValue.at(5);
    m_pelotonResistance = m_pelotonResistance.value();

    qDebug() << QStringLiteral("Current Local elapsed: ") + GetElapsedFromPacket(newValue).toString();
    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current Distance: ") + QString::number(distance);
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

QTime pitpatbike::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    QTime t(0, convertedData / 60, convertedData % 60);
    return t;
}

double pitpatbike::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = pitpatReadBE16(packet, 7);
    double data = ((double)convertedData) / 100.0f;
    return data;
}

void pitpatbike::btinit() {
    // Give the S01PRO a short window to advertise its 0x2c startup frame before
    // enabling the normal poll/control loop. Existing PitPat devices that do not
    // use this startup handshake continue on the old path after the timeout.
    if (legacyProtocol && initDone)
        return;

    initDone = false;
    sec1Update = 0;
    QTimer::singleShot(1500ms, this, [this]() {
        if (!legacyProtocol && m_control && m_control->state() != QLowEnergyController::UnconnectedState) {
            qDebug() << QStringLiteral("PitPat legacy startup frame not seen; enabling existing protocol path");
            initDone = true;
        }
    });
}

void pitpatbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((uint16_t)0xfbb1);
    QBluetoothUuid _gattNotify1CharacteristicId((uint16_t)0xfbb2);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &pitpatbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &pitpatbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &pitpatbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &pitpatbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            && !h
#endif
#endif
        ) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_rower =
                settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                    if (virtual_device_rower) {
                        qDebug() << QStringLiteral("creating virtual rower interface...");
                        auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                        // connect(virtualRower,&virtualrower::debug ,this,&echelonrower::debug);
                        this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                    } else {
                        qDebug() << QStringLiteral("creating virtual bike interface...");
                        auto virtualBike =
                            new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                        // connect(virtualBike,&virtualbike::debug ,this,&pitpatbike::debug);
                        connect(virtualBike, &virtualbike::changeInclination, this, &pitpatbike::changeInclination);
                        this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                    }
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void pitpatbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void pitpatbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void pitpatbike::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((uint16_t)0xfbb0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &pitpatbike::stateChanged);
    if(gattCommunicationChannelService != nullptr) {
        gattCommunicationChannelService->discoverDetails();
    } else {
        if(homeform::singleton())
            homeform::singleton()->setToastRequested("Bluetooth Service Error! Restart the bike!");
        m_control->disconnectFromDevice();
    }
}

void pitpatbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("pitpatbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void pitpatbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("pitpatbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void pitpatbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
                    device.address().toString() + ')';
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &pitpatbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &pitpatbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &pitpatbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &pitpatbike::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    qDebug() << QStringLiteral("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
}

bool pitpatbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t pitpatbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

void pitpatbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        legacyProtocol = false;
        sec1Update = 0;
        m_control->connectToDevice();
    }
}