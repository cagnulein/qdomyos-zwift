#include "iconceptelliptical.h"
#include "keepawakehelper.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

iconceptelliptical::iconceptelliptical(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                       double bikeResistanceGain) {
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &iconceptelliptical::update);
    refresh->start(1s);
}

void iconceptelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
                &iconceptelliptical::serviceDiscovered);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this, &iconceptelliptical::serviceFinished);

        // Start a discovery
        qDebug() << QStringLiteral("iconceptelliptical::deviceDiscovered");
        discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        return;
    }
}

void iconceptelliptical::serviceFinished() {
    qDebug() << QStringLiteral("iconceptelliptical::serviceFinished") << socket;
    if (socket) {

        emit debug(QStringLiteral("Create socket"));
        socket->connectToService(serialPortService);
        emit debug(QStringLiteral("ConnectToService done"));
    }
}

// In your local slot, read information about the found devices
void iconceptelliptical::serviceDiscovered(const QBluetoothServiceInfo &service) {
    // this treadmill has more serial port, just the first one is the right one.
    if (socket != nullptr) {
        qDebug() << QStringLiteral("iconceptelliptical::serviceDiscovered socket already initialized");
        return;
    }

    qDebug() << QStringLiteral("iconceptelliptical::serviceDiscovered") << service;
    /*if (service.device().address() == bluetoothDevice.address())*/ {
        emit debug(QStringLiteral("Found new service: ") + service.serviceName() + '(' +
                   service.serviceUuid().toString() + ')');

        if (service.serviceName().startsWith(QStringLiteral("SerialPort")) ||
            service.serviceName().startsWith(QStringLiteral("Serial Port")) ||
            service.serviceUuid() == QBluetoothUuid(QStringLiteral("00001101-0000-1000-8000-00805f9b34fb"))) {

            emit debug(QStringLiteral("Serial port service found"));
            // discoveryAgent->stop(); // could lead to a crash?

            serialPortService = service;
            socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

            connect(socket, &QBluetoothSocket::readyRead, this, &iconceptelliptical::readSocket);
            connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&iconceptelliptical::rfCommConnected));
            connect(socket, &QBluetoothSocket::disconnected, this, &iconceptelliptical::disconnected);
            connect(socket,
                    QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::errorOccurred),
                    this, &iconceptelliptical::onSocketErrorOccurred);
        }
    }
}

void iconceptelliptical::update() {
    QSettings settings;

    if (initDone) {
        // ******************************************* virtual bike init *************************************
        QSettings settings;
        if (!firstStateChanged && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, true);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &iconceptelliptical::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &iconceptelliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset,
                                                       bikeResistanceGain);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &iconceptelliptical::changeInclinationRequested);
                    connect(virtualBike, &virtualbike::changeInclination, this, &iconceptelliptical::changeInclination);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstStateChanged = 1;
            }
        }
        // ********************************************************************************************************

        if (requestResistance != -1) {
            if (requestResistance > 12) {
                requestResistance = 12;
            } else if (requestResistance < 1) {
                requestResistance = 1;
            }
            char resValues[] = {0x08, 0x0a, 0x0b, 0x0d, 0x0e, 0x10, 0x11, 0x13, 0x14, 0x16, 0x17, 0x18};
            char res[] = {0x55, 0x11, 0x01, 0x12};
            res[3] = resValues[requestResistance - 1];
            qDebug() << QStringLiteral(">>") << QByteArray(res, sizeof(res)).toHex(' ');
            socket->write(res, sizeof(res));
            Resistance = requestResistance;
            requestResistance = -1;
        } else {
            const char poll[] = {0x55, 0x17, 0x01, 0x01};
            qDebug() << QStringLiteral(">>") << QByteArray(poll, sizeof(poll)).toHex(' ');
            socket->write(poll, sizeof(poll));
            emit debug(QStringLiteral("write poll"));
        }

        update_metrics(false, watts());
    }
}

void iconceptelliptical::rfCommConnected() {
    emit debug(QStringLiteral("connected ") + socket->peerName());

    const uint8_t init1[] = {
        0x55, 0x0c, 0x01, 0xff, 0x55, 0xbb, 0x01, 0xff, 0x55, 0x24, 0x01, 0xff, 0x55, 0x25, 0x01, 0xff, 0x55, 0x26,
        0x01, 0xff, 0x55, 0x27, 0x01, 0xff, 0x55, 0x02, 0x01, 0xff, 0x55, 0x03, 0x01, 0xff, 0x55, 0x04, 0x01, 0xff,
        0x55, 0x06, 0x01, 0xff, 0x55, 0x1f, 0x01, 0xff, 0x55, 0xa0, 0x01, 0xff, 0x55, 0xb0, 0x01, 0xff, 0x55, 0xb2,
        0x01, 0xff, 0x55, 0xb3, 0x01, 0xff, 0x55, 0xb4, 0x01, 0xff, 0x55, 0xb5, 0x01, 0xff, 0x55, 0xb6, 0x01, 0xff,
        0x55, 0xb7, 0x01, 0xff, 0x55, 0xb8, 0x01, 0xff, 0x55, 0xb9, 0x01, 0xff, 0x55, 0xba, 0x01, 0xff, 0x55, 0x0b,
        0x01, 0xff, 0x55, 0x18, 0x01, 0xff, 0x55, 0x19, 0x01, 0xff, 0x55, 0x1a, 0x01, 0xff, 0x55, 0x1b, 0x01, 0xff};
    const uint8_t init2[] = {0x55, 0x0a, 0x01, 0x02, 0x55, 0x17, 0x01, 0x01};
    const uint8_t init3[] = {0x55, 0x01, 0x06, 0x34, 0x01, 0x63, 0x00, 0xb4, 0x00};
    const uint8_t init3b[] = {0x55, 0x17, 0x01, 0x01};
    const uint8_t init4[] = {0x55, 0x15, 0x01, 0x00};
    const uint8_t init5[] = {0x55, 0x11, 0x01, 0x01};
    const uint8_t init6[] = {0x55, 0x0a, 0x01, 0x01, 0x55, 0x0a, 0x01, 0x01};
    const uint8_t init6a[] = {0x55, 0x07, 0x01, 0xff};

    socket->write((char *)init1, sizeof(init1));
    qDebug() << QStringLiteral(" init1 write");
    QThread::msleep(2000);
    readSocket();
    QThread::msleep(1000);
    socket->write((char *)init2, sizeof(init2));
    qDebug() << QStringLiteral(" init2 write");
    QThread::msleep(1500);
    readSocket();
    socket->write((char *)init3, sizeof(init3));
    qDebug() << QStringLiteral(" init3 write");
    QThread::msleep(700);
    readSocket();
    socket->write((char *)init3b, sizeof(init3b));
    qDebug() << QStringLiteral(" init3b write");
    QThread::msleep(700);
    readSocket();
    socket->write((char *)init4, sizeof(init4));
    qDebug() << QStringLiteral(" init4 write");
    QThread::msleep(700);
    readSocket();
    socket->write((char *)init5, sizeof(init5));
    qDebug() << QStringLiteral(" init5 write");
    QThread::msleep(600);
    readSocket();
    socket->write((char *)init3b, sizeof(init3b));
    qDebug() << QStringLiteral(" init3b write");
    QThread::msleep(400);
    readSocket();
    socket->write((char *)init6, sizeof(init6));
    qDebug() << QStringLiteral(" init6 write");
    QThread::msleep(600);
    readSocket();
    QThread::msleep(500);
    socket->write((char *)init6a, sizeof(init6a));
    qDebug() << QStringLiteral(" init6a write");
    QThread::msleep(1000);
    readSocket();

    initDone = true;
    // requestStart = 1;
    emit connectedAndDiscovered();
}

void iconceptelliptical::readSocket() {
    if (!socket)
        return;

    while (socket->bytesAvailable()) {
        QByteArray line = socket->readAll();
        qDebug() << QStringLiteral(" << ") + line.toHex(' ');

        if (line.length() == 16) {
            QSettings settings;
            QString heartRateBeltName =
                settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
            elapsed = GetElapsedTimeFromPacket(line);
            Distance = GetDistanceFromPacket(line);
            KCal = GetCaloriesFromPacket(line);
            Speed = GetSpeedFromPacket(line);
            Cadence = (uint8_t)line.at(13);
            // Heart = GetHeartRateFromPacket(line);

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
            /*bool cadence =
                settings.value(QZSettings::treadmill_cadence_sensor, QZSettings::default_treadmill_cadence_sensor)
                    .toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence && h && firstStateChanged) {
                h->virtualtreadmill_setCadence(currentCrankRevolutions(), lastCrankEventTime());
                h->virtualtreadmill_setHeartRate((uint8_t)metrics_override_heartrate());
            }*/
#endif
#endif

            emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
            emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
            // emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
            emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
            emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
            qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());
        }
    }
}

double iconceptelliptical::GetSpeedFromPacket(const QByteArray &packet) {
    double convertedData = ((double)(((double)((uint8_t)packet.at(9))) * 256) + ((double)packet.at(10))) / 100.0;
    return convertedData;
}

uint16_t iconceptelliptical::GetCaloriesFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return convertedData;
}

uint16_t iconceptelliptical::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(5) << 8) | packet.at(6);
    return convertedData;
}

uint16_t iconceptelliptical::GetElapsedTimeFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    return convertedData;
}

void iconceptelliptical::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    qDebug() << QStringLiteral("onSocketErrorOccurred ") << error;
}

uint16_t iconceptelliptical::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

void iconceptelliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}
