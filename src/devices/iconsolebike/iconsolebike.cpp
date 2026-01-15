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
        // Metrics packet received
        Cadence = GetCadenceFromPacket(data);
        Speed = GetSpeedFromPacket(data);
        m_watt = GetPowerFromPacket(data);
        Resistance = GetResistanceFromPacket(data);

        emit debug(QStringLiteral("Metrics - Cadence: %1, Speed: %2, Power: %3, Resistance: %4")
                       .arg(Cadence.value())
                       .arg(Speed.value())
                       .arg(m_watt.value())
                       .arg(Resistance.value()));
    }
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
    if (packet.length() < 9) {
        return 0;
    }

    // Bytes 6-7: Cadence (16-bit big-endian)
    uint16_t raw = ((uint8_t)packet[6] << 8) | (uint8_t)packet[7];

    // Idle value seems to be 0x0101 (257)
    // Active values around 0x0328-0x032B (800-811)
    // Assuming formula: actual_rpm = raw / 10 where 800 = 80 RPM
    if (raw <= 257) {
        return 0;  // No movement
    }

    return raw / 10;  // Convert to actual RPM
}

double iconsolebike::GetSpeedFromPacket(const QByteArray &packet) {
    if (packet.length() < 10) {
        return 0.0;
    }

    // Bytes 8-9: Speed (16-bit big-endian)
    uint16_t raw = ((uint8_t)packet[8] << 8) | (uint8_t)packet[9];

    // Idle value: 0x0101 (257)
    // Active values: 0x0141-0x0142 (321-322)
    // Formula TBD - need more data points
    if (raw <= 257) {
        return 0.0;
    }

    // Temporary formula: (raw - 257) / 10
    return (raw - 257) / 10.0;
}

uint16_t iconsolebike::GetPowerFromPacket(const QByteArray &packet) {
    if (packet.length() < 18) {
        return 0;
    }

    // Bytes 16-17: Power or accumulated distance (16-bit big-endian)
    uint16_t raw = ((uint8_t)packet[16] << 8) | (uint8_t)packet[17];

    // Values seen: 0x0A3E-0x0A58 (2622-2648)
    // Could be instantaneous power or accumulated distance
    // For now, treat as power in watts
    if (raw <= 257) {
        return 0;
    }

    // Temporary formula - needs calibration
    return raw / 10;
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
