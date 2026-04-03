#include "iconsolebike.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

iconsolebike::iconsolebike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                           double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    connect(refresh, &QTimer::timeout, this, &iconsolebike::update);
    refresh->start(1s);
}

iconsolebike::~iconsolebike() {
    if (socket) {
        socket->close();
    }
}

void iconsolebike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    // Look for ICONSOLE+ device
    if (device.name().toUpper().startsWith(QStringLiteral("ICONSOLE+"))) {
        bluetoothDevice = device;

        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
                &iconsolebike::serviceDiscovered);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled, this,
                &iconsolebike::serviceCanceled);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this,
                &iconsolebike::serviceFinished);

        // Start discovery
        QSettings settings;
        bool discoveryCompleted = settings.value(QZSettings::toorxtreadmill_discovery_completed,
                                                QZSettings::default_toorxtreadmill_discovery_completed).toBool();
        qDebug() << QStringLiteral("iconsolebike::deviceDiscovered - discoveryCompleted:") << discoveryCompleted;

        if (discoveryCompleted) {
            discoveryAgent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);
        } else {
            discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        }
        return;
    }
}

void iconsolebike::serviceFinished(void) {
    qDebug() << QStringLiteral("iconsolebike::serviceFinished") << socket;
    if (!socket) {
        socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

        connect(socket, &QBluetoothSocket::readyRead, this, &iconsolebike::readSocket);
        connect(socket, &QBluetoothSocket::connected, this,
                QOverload<>::of(&iconsolebike::rfCommConnected));
        connect(socket, &QBluetoothSocket::disconnected, this, &iconsolebike::disconnected);
        connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this,
                &iconsolebike::onSocketErrorOccurred);

#ifdef Q_OS_ANDROID
        socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);
#endif

        emit debug(QStringLiteral("Create socket"));
        if (!found) {
            qDebug() << QStringLiteral("iconsolebike::serviceFinished, no service found, trying workaround");
            socket->connectToService(bluetoothDevice.address(), QBluetoothUuid(QBluetoothUuid::SerialPort));
        } else {
            socket->connectToService(serialPortService);
        }
        emit debug(QStringLiteral("ConnectToService done"));
    }
}

void iconsolebike::serviceCanceled(void) {
    qDebug() << QStringLiteral("iconsolebike::serviceCanceled");
}

void iconsolebike::serviceDiscovered(const QBluetoothServiceInfo &service) {
    qDebug() << QStringLiteral("Found new service: ") << service.serviceName() << '('
             << service.serviceUuid().toString() << ") " << service.device().address()
             << bluetoothDevice.address();

    if (found == true) {
        qDebug() << QStringLiteral("iconsolebike::serviceDiscovered socket already initialized");
        return;
    }

    qDebug() << QStringLiteral("iconsolebike::serviceDiscovered") << service;
    if ((service.serviceName().startsWith(QStringLiteral("SerialPort")) ||
         service.serviceName().startsWith(QStringLiteral("Serial Port"))) &&
        service.serviceUuid() == QBluetoothUuid(QStringLiteral("00001101-0000-1000-8000-00805f9b34fb"))) {
        serialPortService = service;
        found = true;
        emit debug(QStringLiteral("Serial port service found"));
    }
}

void iconsolebike::send(const uint8_t *buffer, int size) {
    QByteArray byteArray((const char *)buffer, size);
    qDebug() << ">>" << byteArray.toHex(' ');
    emit debug(QStringLiteral(" >> ") + byteArray.toHex(' '));
    if (socket && socket->isOpen()) {
        socket->write((const char *)buffer, size);
    }
}

void iconsolebike::btinit() {
    qDebug() << QStringLiteral("iconsolebike::btinit - phase:") << initPhase;

    switch (initPhase) {
    case 0: {
        // Send hello packet
        uint8_t hello[] = {0xF0, 0xA0, 0x00, 0x00, 0x90};
        send(hello, sizeof(hello));
        initPhase++;
        break;
    }
    case 1: {
        // Send init confirm
        uint8_t init_confirm[] = {0xF0, 0xA0, 0x01, 0x01, 0x92};
        send(init_confirm, sizeof(init_confirm));
        initPhase++;
        break;
    }
    case 2: { // Data: f0a1010193
        uint8_t frame[] = {0xF0, 0xA1, 0x01, 0x01, 0x93};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 3: { // Data: f0a50101049b
        uint8_t frame[] = {0xF0, 0xA5, 0x01, 0x01, 0x04, 0x9B};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 4: { // Data: f0a501010299
        uint8_t frame[] = {0xF0, 0xA5, 0x01, 0x01, 0x02, 0x99};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 5: { // Data: f0a60101079f
        uint8_t frame[] = {0xF0, 0xA6, 0x01, 0x01, 0x07, 0x9F};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 6: { // Data: f0a50101049b
        uint8_t frame[] = {0xF0, 0xA5, 0x01, 0x01, 0x04, 0x9B};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 7: { // Data: f0a501010299
        uint8_t frame[] = {0xF0, 0xA5, 0x01, 0x01, 0x02, 0x99};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 8: { // Data: f0a60101079f
        uint8_t frame[] = {0xF0, 0xA6, 0x01, 0x01, 0x07, 0x9F};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 9: { // Data: f0a2010194
        uint8_t frame[] = {0xF0, 0xA2, 0x01, 0x01, 0x94};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 10: { // Data: f0a2010194
        uint8_t frame[] = {0xF0, 0xA2, 0x01, 0x01, 0x94};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 11: { // Data: f0a6010108a0
        uint8_t frame[] = {0xF0, 0xA6, 0x01, 0x01, 0x08, 0xA0};
        send(frame, sizeof(frame));
        initPhase++;
        break;
    }
    case 12: {
        // Initialization complete
        initDone = true;
        emit debug(QStringLiteral("Init complete"));
        break;
    }
    }
}

void iconsolebike::rfCommConnected() {
    emit debug(QStringLiteral("Connected to ICONSOLE+ bike"));
    qDebug() << QStringLiteral("iconsolebike::rfCommConnected");

    // Start initialization
    initPhase = 0;
    QThread::msleep(400);
    btinit();
}

void iconsolebike::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    qDebug() << QStringLiteral("iconsolebike::onSocketErrorOccurred") << error;
    emit debug(QStringLiteral("Socket error: ") + QString::number(error));
}

void iconsolebike::readSocket() {
    if (!socket) {
        return;
    }

    QByteArray data = socket->readAll();
    qDebug() << "<<" << data.toHex(' ');
    emit debug(QStringLiteral(" << ") + data.toHex(' '));

    // Check if we're in init phase
    if (!initDone && data.length() >= 5) {
        // Check for init responses
        btinit();
        return;
    }

    // Parse metrics if this is a polling response
    if (initDone && data.length() >= 20 && data[0] == (char)0xF0 && data[1] == (char)0xB2) {
        // Metrics packet received.
        // packet[8] == 0x01 marks a valid cadence/speed/power frame.
        // When packet[8] == 0x02 the cadence field (packet[9]) carries
        // unrelated data; skip updating those metrics to avoid the
        // jumping readings seen on the Ride6 iPlus at low resistance.
        QSettings settings;
        Resistance = GetResistanceFromPacket(data);
        if ((uint8_t)data[8] == 0x01) {
            Cadence = GetCadenceFromPacket(data);
            Speed = GetSpeedFromPacket(data);
            m_watt = GetPowerFromPacket(data);
        }

        // Calculate KCal based on watts and body weight
        if (watts()) {
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                     QDateTime::currentDateTime()))));
        }

        // Calculate distance based on speed
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

        // Update crank revolutions and event time for cadence sensor
        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        // Update peloton resistance
        m_pelotonResistance = bikeResistanceToPeloton(Resistance.value());

        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

        emit debug(QStringLiteral("Metrics - Cadence: %1, Speed: %2, Power: %3, Resistance: %4, Distance: %5, KCal: %6")
                       .arg(Cadence.value())
                       .arg(Speed.value())
                       .arg(m_watt.value())
                       .arg(Resistance.value())
                       .arg(Distance.value())
                       .arg(KCal.value()));
    }
}

uint16_t iconsolebike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void iconsolebike::update() {
    if (!socket || !socket->isOpen()) {
        emit disconnected();
        return;
    }

    if (initDone) {
        // Initialize virtual device if needed
        if (!this->hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled)
                    .toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }

        // Update metrics
        update_metrics(true, watts());

        // Send polling command every second
        if (sec1update++ >= (1000 / refresh->interval())) {
            sec1update = 0;

            // Send F0 A2 polling packet
            uint8_t poll[] = {0xF0, 0xA2, 0x01, 0x01, 0x94};
            send(poll, sizeof(poll));
        }

        // Handle resistance change requests
        if (requestResistance != -1) {
            if (requestResistance > maxResistance()) {
                requestResistance = maxResistance();
            } else if (requestResistance < 0) {
                requestResistance = 0;
            }

            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestPower != 0) {
            requestPower = 0;
        }
    } else if (initPhase > 0) {
        // Waiting for init to complete
        // btinit() will be called from readSocket() when responses arrive
    } else {
        // Start init if not started
        btinit();
    }
}

void iconsolebike::forceResistance(resistance_t requestResistance) {
    // Protocol: F0 A6 01 01 <RES> <CHECKSUM>
    uint8_t resistance[] = {0xF0, 0xA6, 0x01, 0x01, 0x00, 0x00};

    resistance[4] = (uint8_t)requestResistance;

    // Calculate checksum
    resistance[5] = 0;
    for (int i = 0; i < 5; i++) {
        resistance[5] += resistance[i];
    }

    send(resistance, sizeof(resistance));
}

uint16_t iconsolebike::GetCadenceFromPacket(const QByteArray &packet) {
    if (packet.length() < 10) {
        return 0;
    }

    // packet[9] encodes cadence in RPM directly (1:1 mapping).
    // Verified against three sessions: [9]=58→60 rpm, [9]=79→80 rpm, [9]=97→100 rpm.
    uint8_t raw = (uint8_t)packet[9];

    if (raw <= 1) {
        return 0;
    }

    return raw;
}

double iconsolebike::GetSpeedFromPacket(const QByteArray &packet) {
    if (packet.length() < 10) {
        return 0.0;
    }

    // Speed is derived from cadence via the fixed gear ratio of this bike.
    // Empirically measured constant across three sessions:
    //   22 kph / 58 rpm = 0.379,  30 kph / 79 rpm = 0.380,  37 kph / 97 rpm = 0.381
    uint8_t raw = (uint8_t)packet[9];

    if (raw <= 1) {
        return 0.0;
    }

    return raw * 0.38;
}

uint16_t iconsolebike::GetPowerFromPacket(const QByteArray &packet) {
    if (packet.length() < 10) {
        return 0;
    }

    // Bytes [16-17] form a big-endian uint16 that encodes resistance-adjusted
    // power.  Unlike bytes [6-7] (which vary only with cadence), [16-17]
    // increases both with cadence and with resistance, so it correctly
    // reflects the change in effort when the load is varied at constant rpm.
    //
    // Calibration from three independent sessions (Maarten, 2026-03-29):
    //   res=8, 58 rpm  → [16-17]≈2367, bike display=60 W  → 2367/41=57.7 W  (-3.8%)
    //   res=8, 79 rpm  → [16-17]≈3906, bike display=90 W  → 3906/41=95.3 W  (+5.9%)  ← average 42.4
    //   res=8, 97 rpm  → [16-17]≈5338, bike display=130 W → 5338/41=130.2 W (+0.2%)
    // Cross-check (Kinomap btsnoop 2026-04-01, res 8→11 at ~66 rpm):
    //   res= 8, 66 rpm → [16-17]≈2648 → 64.6 W   (cadence-only formula gives 70.8 W — unchanged)
    //   res=11, 66 rpm → [16-17]≈3900 → 95.1 W   (correctly 47% higher at higher resistance)
    // The divisor 41 is the arithmetic mean of the three calibration points.
    if (packet.length() < 18) {
        return 0;
    }

    uint16_t power_raw = ((uint8_t)packet[16] << 8) | (uint8_t)packet[17];
    uint8_t cadence_raw = (uint8_t)packet[9];

    if (cadence_raw <= 1 || power_raw <= 0x0101) {
        return 0;
    }

    return qRound(power_raw / 41.0);
}

uint8_t iconsolebike::GetResistanceFromPacket(const QByteArray &packet) {
    if (packet.length() < 19) {
        return 0;
    }

    // Byte 18: Current resistance level (direct value)
    return (uint8_t)packet[18];
}

bool iconsolebike::connected() {
    return socket && socket->isOpen();
}

resistance_t iconsolebike::pelotonToBikeResistance(int pelotonResistance) {
    // Map Peloton resistance (0-100) to bike resistance (1-32)
    // This is a simple linear mapping that can be adjusted
    if (pelotonResistance <= 0) {
        return 1;
    }
    if (pelotonResistance >= 100) {
        return 32;
    }

    // Linear interpolation
    return 1 + ((pelotonResistance * 31) / 100);
}

double iconsolebike::bikeResistanceToPeloton(double bikeResistance) {
    // Map bike resistance (1-32) to Peloton resistance (0-100)
    // Inverse of pelotonToBikeResistance
    if (bikeResistance <= 1) {
        return 0;
    }
    if (bikeResistance >= 32) {
        return 100;
    }

    // Linear interpolation: pelotonRes = (bikeRes - 1) * 100 / 31
    return ((bikeResistance - 1) * 100) / 31.0;
}
