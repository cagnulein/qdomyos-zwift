#include "iconceptbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include "qzsettings.h"

using namespace std::chrono_literals;

iconceptbike::iconceptbike() {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &iconceptbike::update);
    refresh->start(1s);
}

void iconceptbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    if (device.name().toUpper().startsWith(QStringLiteral("BH DUALKIT"))) {
        bluetoothDevice = device;

        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
                &iconceptbike::serviceDiscovered);

        // Start a discovery
        qDebug() << QStringLiteral("iconceptbike::deviceDiscovered");
        discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        return;
    }
}

// In your local slot, read information about the found devices
void iconceptbike::serviceDiscovered(const QBluetoothServiceInfo &service) {
    // this treadmill has more serial port, just the first one is the right one.
    if (socket != nullptr) {
        qDebug() << QStringLiteral("iconceptbike::serviceDiscovered socket already initialized");
        return;
    }

    qDebug() << QStringLiteral("iconceptbike::serviceDiscovered") << service;
    if (service.device().address() == bluetoothDevice.address()) {
        emit debug(QStringLiteral("Found new service: ") + service.serviceName() + '(' +
                   service.serviceUuid().toString() + ')');

        if (service.serviceName().startsWith(QStringLiteral("SerialPort")) ||
            service.serviceName().startsWith(QStringLiteral("Serial Port"))) {
            emit debug(QStringLiteral("Serial port service found"));
            discoveryAgent->stop();

            serialPortService = service;
            socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

            connect(socket, &QBluetoothSocket::readyRead, this, &iconceptbike::readSocket);
            connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&iconceptbike::rfCommConnected));
            connect(socket, &QBluetoothSocket::disconnected, this, &iconceptbike::disconnected);
            connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this,
                    &iconceptbike::onSocketErrorOccurred);

#ifdef Q_OS_ANDROID
            socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);
#endif

            emit debug(QStringLiteral("Create socket"));
            socket->connectToService(serialPortService);
            emit debug(QStringLiteral("ConnectToService done"));
        }
    }
}

void iconceptbike::update() {
    QSettings settings;

    if (initDone) {
        // ******************************************* virtual treadmill init *************************************
        if (!this->isVirtualDeviceSetUp() && !virtualBike && !this->isPelotonWorkaroundActive()) {
            QSettings settings;
            bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                virtualBike = new virtualbike(this, true);
                connect(virtualBike, &virtualbike::changeInclination, this, &iconceptbike::changeInclination);
            }
        }
        this->setVirtualDeviceSetUp();

        // ********************************************************************************************************

        if (requestResistance != -1) {
            if (requestResistance > 32) {
                requestResistance = 32;
            } else if (requestResistance < 1) {
                requestResistance = 1;
            }
            char res[] = {0x55, 0x11, 0x01, 0x12};
            res[3] = requestResistance;
            socket->write(res, sizeof(res));
            requestResistance = -1;
        }

        const char poll[] = {0x55, 0x17, 0x01, 0x01};
        socket->write(poll, sizeof(poll));
        emit debug(QStringLiteral("write poll"));

        update_metrics(true, watts());
    }
}

void iconceptbike::rfCommConnected() {
    emit debug(QStringLiteral("connected ") + socket->peerName());

    const uint8_t init1[] = {0x55, 0x0c, 0x01, 0xff, 0x55, 0xbb, 0x01, 0xff, 0x55, 0x24, 0x01, 0xff, 0x55, 0x25, 0x01,
                             0xff, 0x55, 0x26, 0x01, 0xff, 0x55, 0x27, 0x01, 0xff, 0x55, 0x02, 0x01, 0xff, 0x55, 0x03,
                             0x01, 0xff, 0x55, 0x04, 0x01, 0xff, 0x55, 0x06, 0x01, 0xff, 0x55, 0x1f, 0x01, 0xff, 0x55,
                             0xa0, 0x01, 0xff, 0x55, 0xb0, 0x01, 0xff, 0x55, 0xb2, 0x01, 0xff, 0x55, 0xb3, 0x01, 0xff,
                             0x55, 0xb4, 0x01, 0xff, 0x55, 0xb5, 0x01, 0xff, 0x55, 0xb6, 0x01, 0xff, 0x55, 0xb7, 0x01,
                             0xff, 0x55, 0xb8, 0x01, 0xff, 0x55, 0xb9, 0x01, 0xff, 0x55, 0xba, 0x01, 0xff};
    const uint8_t init2[] = {0x55, 0x0b, 0x01, 0xff, 0x55, 0x18, 0x01, 0xff, 0x55, 0x19,
                             0x01, 0xff, 0x55, 0x1a, 0x01, 0xff, 0x55, 0x1b, 0x01, 0xff};
    const uint8_t init3[] = {0x55, 0x17, 0x01, 0x01, 0x55, 0xb5, 0x01, 0xff};
    const uint8_t init4[] = {0x55, 0x01, 0x06, 0x1e, 0x00, 0x3c, 0x00, 0xaa, 0x00};
    const uint8_t init5[] = {0x55, 0x15, 0x01, 0x00};
    const uint8_t init6[] = {0x55, 0x11, 0x01, 0x01};
    const uint8_t init7[] = {0x55, 0x0a, 0x01, 0x01};
    const uint8_t init8[] = {0x55, 0x07, 0x01, 0xff};

    socket->write((char *)init1, sizeof(init1));
    qDebug() << QStringLiteral(" init1 write");
    socket->write((char *)init2, sizeof(init2));
    qDebug() << QStringLiteral(" init2 write");
    QThread::msleep(2000);
    socket->write((char *)init3, sizeof(init3));
    qDebug() << QStringLiteral(" init3 write");
    QThread::msleep(2000);
    socket->write((char *)init3, sizeof(init3));
    qDebug() << QStringLiteral(" init3 write");
    QThread::msleep(500);
    socket->write((char *)init4, sizeof(init4));
    qDebug() << QStringLiteral(" init4 write");
    QThread::msleep(600);
    socket->write((char *)init5, sizeof(init5));
    qDebug() << QStringLiteral(" init5 write");
    QThread::msleep(600);
    socket->write((char *)init6, sizeof(init6));
    qDebug() << QStringLiteral(" init6 write");
    QThread::msleep(600);
    socket->write((char *)init7, sizeof(init7));
    qDebug() << QStringLiteral(" init7 write");
    QThread::msleep(600);
    socket->write((char *)init8, sizeof(init8));
    qDebug() << QStringLiteral(" init8 write");

    initDone = true;
    // requestStart = 1;
    emit connectedAndDiscovered();
}

void iconceptbike::readSocket() {
    if (!socket)
        return;

    while (socket->bytesAvailable()) {
        QByteArray line = socket->readAll();
        qDebug() << QStringLiteral(" << ") + line.toHex(' ');

        if (line.length() == 16) {
            elapsed = GetElapsedTimeFromPacket(line);
            Distance = GetDistanceFromPacket(line);
            KCal = GetCaloriesFromPacket(line);
            Speed = GetSpeedFromPacket(line);
            Cadence = (uint8_t)line.at(13);
            // Heart = GetHeartRateFromPacket(line);

            QSettings settings;
            QString heartRateBeltName = settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                    this->updateLockscreenEnergyDistanceHeartRate();
            }
            this->doPelotonWorkaround();

            emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
            emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
            // emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
            emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
            emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        }
    }
}

double iconceptbike::GetSpeedFromPacket(const QByteArray &packet) {
    double convertedData = ((double)((double)((uint8_t)packet.at(9))) + ((double)packet.at(10))) / 100.0;
    return convertedData;
}

uint16_t iconceptbike::GetCaloriesFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return convertedData;
}

uint16_t iconceptbike::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(5) << 8) | packet.at(6);
    return convertedData;
}

uint16_t iconceptbike::GetElapsedTimeFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    return convertedData;
}

void iconceptbike::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    emit debug(QStringLiteral("onSocketErrorOccurred ") + QString::number(error));
}

void *iconceptbike::VirtualBike() { return virtualBike; }

void *iconceptbike::VirtualDevice() { return VirtualBike(); }
