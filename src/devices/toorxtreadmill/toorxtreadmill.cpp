#include "toorxtreadmill.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <QThread>

using namespace std::chrono_literals;

toorxtreadmill::toorxtreadmill() {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &toorxtreadmill::update);
    refresh->start(1s);
}

void toorxtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    /*if (device.name().startsWith(QStringLiteral("TRX ROUTE KEY")) ||
        device.name().toUpper().startsWith(QStringLiteral("BH-TR-")))*/ {
        bluetoothDevice = device;
        if (device.name().toUpper().startsWith(QStringLiteral("MASTERT40"))) {
            MASTERT409 = true;
            qDebug() << "MASTERT409 workarkound enabled";
        }

        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
                &toorxtreadmill::serviceDiscovered);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled, this,
                &toorxtreadmill::serviceCanceled);
        connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this,
                &toorxtreadmill::serviceFinished);

        // Start a discovery - use FullDiscovery only if not done before
        QSettings settings;
        bool discoveryCompleted = settings.value(QZSettings::toorxtreadmill_discovery_completed, QZSettings::default_toorxtreadmill_discovery_completed).toBool();
        qDebug() << QStringLiteral("toorxtreadmill::deviceDiscovered - discoveryCompleted:") << discoveryCompleted;
        
        if (discoveryCompleted) {
            discoveryAgent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);
        } else {
            discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        }
        return;
    }
}

void toorxtreadmill::serviceFinished(void) {
    qDebug() << QStringLiteral("technogymmyruntreadmillrfcomm::serviceFinished") << socket;
    if (!socket) {
        socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

            connect(socket, &QBluetoothSocket::readyRead, this, &toorxtreadmill::readSocket);
            connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&toorxtreadmill::rfCommConnected));
            connect(socket, &QBluetoothSocket::disconnected, this, &toorxtreadmill::disconnected);
            connect(socket,
                    QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::errorOccurred),
                    this, &toorxtreadmill::onSocketErrorOccurred);


        emit debug(QStringLiteral("Create socket"));
        if(!found) {
            qDebug() << QStringLiteral("toorxtreadmill::serviceFinished, no service found, trying workaround");
            socket->connectToService(bluetoothDevice.address(), QBluetoothUuid(QBluetoothUuid::SerialPort));
        } else {
            socket->connectToService(serialPortService);
        }
        emit debug(QStringLiteral("ConnectToService done"));
    }
}

void toorxtreadmill::serviceCanceled(void) {
    qDebug() << QStringLiteral("toorxtreadmill::serviceCanceled");
}

// In your local slot, read information about the found devices
void toorxtreadmill::serviceDiscovered(const QBluetoothServiceInfo &service) {
    qDebug() << QStringLiteral("Found new service: ") << service.serviceName() << '(' << service.serviceUuid().toString() << ") " << service.device().address() << bluetoothDevice.address();

    if (found == true) {
        qDebug() << QStringLiteral("toorxtreadmill::serviceDiscovered socket already initialized");
        //return;
    }

    qDebug() << QStringLiteral("toorxtreadmill::serviceDiscovered") << service;
    if ((service.serviceName().startsWith(QStringLiteral("SerialPort")) ||
         service.serviceName().startsWith(QStringLiteral("Serial Port"))) &&
        // android 13 workaround
        service.serviceUuid() == QBluetoothUuid(QStringLiteral("00001101-0000-1000-8000-00805f9b34fb"))) {
            serialPortService = service;
            found = true;
            emit debug(QStringLiteral("Serial port service found"));
            //discoveryAgent->stop();
    }
}
void toorxtreadmill::send(char * buffer, int size) {
    QByteArray byteArray(buffer, size);
    qDebug() << ">>" << byteArray.toHex(' ');    
    socket->write((char *)buffer, size);
}

void toorxtreadmill::update() {
    static int8_t start_phase = -1;
    QSettings settings;
    bool toorx_65s_evo = settings.value(QZSettings::toorx_65s_evo, QZSettings::default_toorx_65s_evo).toBool();

    if (initDone) {
        // ******************************************* virtual treadmill init *************************************
        if (!this->hasVirtualDevice()) {
            QSettings settings;
            bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual treadmill interface..."));
                auto virtualTreadMill = new virtualtreadmill(this, true);
                connect(virtualTreadMill, &virtualtreadmill::debug, this, &toorxtreadmill::debug);
                this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        // ********************************************************************************************************

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                // the treadmill send the speed in miles for some models
                double miles = 1;
                if (settings.value(QZSettings::sole_treadmill_miles, QZSettings::default_sole_treadmill_miles).toBool())
                    miles = 1.60934;                

                requestSpeed = requestSpeed / miles;
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                char speed[] = {0x55, 0x0f, 0x02, 0x01, 0x00};
                speed[3] = (uint8_t)(requestSpeed);
                speed[4] = (uint8_t)((requestSpeed - (double)((uint8_t)(requestSpeed))) * 100.0);
                send((char *)speed, sizeof(speed));
                Speed = requestSpeed;
            }
            requestSpeed = -1;
        } else if (requestInclination != -100) {
            if(requestInclination < 0)
                requestInclination = 0;
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                uint8_t incline[] = {0x55, 0x0a, 0x01, 0x01};
                incline[3] = requestInclination;
                socket->write((char *)incline, sizeof(incline));
                Inclination = requestInclination;
            }
            requestInclination = -100;
        } else if (requestStart != -1 && start_phase == -1) {
            emit debug(QStringLiteral("starting..."));
            //const uint8_t start[] = {0x55, 0x17, 0x01, 0x01, 0x55, 0xb5, 0x01, 0xff};
            //send((char *)start, sizeof(start));
            start_phase = 0;
            requestStart = -1;
            emit tapeStarted();
            if(MASTERT409) {
                Speed = 1;
                Inclination = 0;
            }
        } else if (start_phase != -1) {
            requestStart = -1;
            if(toorx_65s_evo) {
                switch (start_phase) {
                    case 0: {
                        const uint8_t start0[] = {0x55, 0x0a, 0x01, 0x02};
                        send((char *)start0, sizeof(start0));
                        start_phase++;
                        break;
                    }
                    case 1: {
                        const uint8_t start[] = {0x55, 0x01, 0x06, 0x1f, 0x00, 0x3c, 0x00, 0xaa, 0x00};
                        send((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }
                    case 2: {
                        const uint8_t start1[] = {0x55, 0x17, 0x01, 0x00};
                        send((char *)start1, sizeof(start1));
                        start_phase++;
                        break;
                    }
                    case 3: {
                        const uint8_t start1[] = {0x55, 0x15, 0x01, 0x00};
                        send((char *)start1, sizeof(start1));
                        start_phase++;
                        break;
                    }
                    case 4: {
                        const uint8_t start1[] = {0x55, 0x17, 0x01, 0x00};
                        send((char *)start1, sizeof(start1));
                        start_phase++;
                        break;
                    }
                    case 5: {
                        const char start2[] = {0x55, 0x0f, 0x02, 0x01, 0x00};
                        send((char *)start2, sizeof(start2));
                        start_phase++;
                        break;
                    }
                    case 6: {
                        const uint8_t start3[] = {0x55, 0x11, 0x01, 0x01};
                        send((char *)start3, sizeof(start3));
                        start_phase++;
                        break;
                    }
                    case 7: {
                        const uint8_t start1[] = {0x55, 0x17, 0x01, 0x00};
                        send((char *)start1, sizeof(start1));
                        start_phase++;
                        break;
                    }
                    case 8: {
                        const uint8_t start4[] = {0x55, 0x08, 0x01, 0x01};
                        send((char *)start4, sizeof(start4));
                        start_phase++;
                        break;
                    }
                    case 9:
                    case 10:
                    case 11:
                    case 12: {
                        const uint8_t start1[] = {0x55, 0x17, 0x01, 0x00};
                        send((char *)start1, sizeof(start1));
                        start_phase++;
                        break;
                    }
                    case 13:
                    case 14: {
                        const uint8_t start5[] = {0x55, 0x0a, 0x01, 0x01};
                        send((char *)start5, sizeof(start5));
                        start_phase++;
                        break;
                    }
                    case 15: {
                        const uint8_t start6[] = {0x55, 0x07, 0x01, 0xff};
                        send((char *)start6, sizeof(start6));
                        start_phase = -1;
                        break;
                    }
                }
            } else if(MASTERT409) {
                switch (start_phase) {
                    case 0: {
                        const uint8_t start0[] = {0x55, 0x01, 0x06, 0x2b, 0x00, 0x61, 0x00, 0xb0, 0x00, 0x55, 0xb9, 0x01, 0xff, 0x55, 0xb5, 0x01, 0xff};
                        socket->write((char *)start0, sizeof(start0));
                        start_phase++;
                        break;
                    }
                    case 1: {
                        const uint8_t start[] = {0x55, 0x17, 0x01, 0x01};
                        socket->write((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }
                    case 2: {
                        const uint8_t start1[] = {0x55, 0x0a, 0x01, 0x02};
                        socket->write((char *)start1, sizeof(start1));
                        start_phase++;
                        break;
                    }
                    case 3: {
                        const uint8_t start2[] = {0x55, 0x01, 0x06, 0x2b, 0x00, 0x61, 0x00, 0xb0, 0x00};
                        socket->write((char *)start2, sizeof(start2));
                        start_phase++;
                        break;
                    }
                    case 4: {
                        const uint8_t start3[] = {0x55, 0x15, 0x01, 0x00};
                        socket->write((char *)start3, sizeof(start3));
                        start_phase++;
                        break;
                    }
                    case 5: {
                        const uint8_t start4[] = {0x55, 0x0f, 0x02, 0x01, 0x00};
                        socket->write((char *)start4, sizeof(start4));
                        start_phase++;
                        break;
                    }
                    case 6: {
                        const uint8_t start[] = {0x55, 0x17, 0x01, 0x01};
                        socket->write((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }
                    case 7: {
                        const uint8_t start[] = {0x55, 0x11, 0x01, 0x00};
                        socket->write((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }
                    case 8: {
                        const uint8_t start[] = {0x55, 0x08, 0x01, 0x01};
                        socket->write((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }
                    case 9: {
                        const uint8_t start[] = {0x55, 0x0a, 0x01, 0x01};
                        socket->write((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }                    
                    case 10: {
                        const uint8_t start[] = {0x55, 0x07, 0x01, 0xff};
                        socket->write((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }                    
                    case 11: {
                        const uint8_t start[] = {0x55, 0x11, 0x01, 0x00};
                        socket->write((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }
                    case 12: {
                        const uint8_t start[] = {0x55, 0x0f, 0x02, 0x06, 0x00};
                        socket->write((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }
                    case 13: {
                        const uint8_t start[] = {0x55, 0x0f, 0x02, 0x06, 0x00};
                        socket->write((char *)start, sizeof(start));
                        start_phase = -1;
                        break;
                    }
                }
            } else {
                switch (start_phase) {
                    case 0: {
                        const uint8_t init2[] = {0x55, 0x0c, 0x01, 0xff, 0x55, 0xbb, 0x01, 0xff, 0x55, 0x24, 0x01, 0xff, 
                                            0x55, 0x25, 0x01, 0xff, 0x55, 0x26, 0x01, 0xff, 0x55, 0x27, 0x01, 0xff, 0x55, 0x02,
                                            0x01, 0xff, 0x55, 0x03, 0x01, 0xff, 0x55, 0x04, 0x01, 0xff, 0x55, 0x06, 0x01, 0xff,
                                            0x55, 0x1f, 0x01, 0xff, 0x55, 0xa0, 0x01, 0xff, 0x55, 0xb0, 0x01, 0xff, 0x55, 0xb2,
                                            0x01, 0xff, 0x55, 0xb3, 0x01, 0xff, 0x55, 0xb4, 0x01, 0xff, 0x55, 0xb5, 0x01, 0xff,
                                            0x55, 0xb6, 0x01, 0xff, 0x55, 0xb7, 0x01, 0xff, 0x55, 0xb8, 0x01, 0xff, 0x55, 0xb9,
                                            0x01, 0xff, 0x55, 0xba, 0x01, 0xff, 0x55, 0x0b, 0x01, 0xff, 0x55, 0x18, 0x01, 0xff,
                                            0x55, 0x19, 0x01, 0xff, 0x55, 0x1a, 0x01, 0xff, 0x55, 0x1b, 0x01, 0xff};
                        send((char *)init2, sizeof(init2));
                        start_phase++;
                        break;
                    }
                    case 1:
                        start_phase++;
                        break;
                    case 2: {
                        const uint8_t init3[] = {0x55, 0x01, 0x06, 0x2f, 0x00, 0x56, 0x00, 0xb7, 0x00, 0x55, 0xb9, 0x01, 0xff, 0x55, 0xb5, 0x01, 0xff};
                        send((char *)init3, sizeof(init3));
                        start_phase++;
                        break;
                    }
                    case 3: {
                        const uint8_t init4[] = {0x55, 0x17, 0x01, 0x01};
                        send((char *)init4, sizeof(init4));
                        start_phase++;
                        break;
                    }
                    case 4: {
                        const uint8_t start0[] = {0x55, 0x0a, 0x01, 0x02};
                        send((char *)start0, sizeof(start0));
                        start_phase++;
                        break;
                    }
                    case 5: {
                        const uint8_t start[] = {0x55, 0x01, 0x06, 0x1d, 0x00, 0x3c, 0x00, 0xaa, 0x00};
                        send((char *)start, sizeof(start));
                        start_phase++;
                        break;
                    }
                    case 6: {
                        const uint8_t start1[] = {0x55, 0x15, 0x01, 0x00};
                        send((char *)start1, sizeof(start1));
                        start_phase++;
                        break;
                    }
                    case 7: {
                        const char start2[] = {0x55, 0x0f, 0x02, 0x01, 0x00};
                        send((char *)start2, sizeof(start2));
                        start_phase++;
                        break;
                    }
                    case 8: {
                        const uint8_t start3[] = {0x55, 0x11, 0x01, 0x01};
                        send((char *)start3, sizeof(start3));
                        start_phase++;
                        break;
                    }
                    case 9: {
                        const uint8_t start4[] = {0x55, 0x08, 0x01, 0x01};
                        send((char *)start4, sizeof(start4));
                        start_phase++;
                        break;
                    }
                    case 10:
                    case 11: {
                        const uint8_t start5[] = {0x55, 0x0a, 0x01, 0x01};
                        send((char *)start5, sizeof(start5));
                        start_phase++;
                        break;
                    }
                    case 12: {
                        const uint8_t start6[] = {0x55, 0x07, 0x01, 0xff};
                        send((char *)start6, sizeof(start6));
                        start_phase = -1;
                        break;
                    }
                }
            qDebug() << " start phase " << start_phase;
            }
        } else {
            const char poll[] = {0x55, 0x17, 0x01, 0x01};
            send((char*)poll, sizeof(poll));
            emit debug(QStringLiteral("write poll"));
        }

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));
    }
}

void toorxtreadmill::rfCommConnected() {
    emit debug(QStringLiteral("connected ") + socket->peerName());

    this->initDone = true;
    this->requestStart = 1;
    
    // Mark discovery as completed for future connections
    QSettings settings;
    if (!settings.value(QZSettings::toorxtreadmill_discovery_completed, QZSettings::default_toorxtreadmill_discovery_completed).toBool()) {
        settings.setValue(QZSettings::toorxtreadmill_discovery_completed, true);
        qDebug() << QStringLiteral("toorxtreadmill discovery marked as completed");
    }
    
    emit this->connectedAndDiscovered();
}

void toorxtreadmill::readSocket() {
    if (!socket)
        return;

    while (socket->bytesAvailable()) {
        QByteArray line = socket->readAll();
        qDebug() << QStringLiteral(" << ") + line.toHex(' ');

        if (line.length() == 17 && line.at(1) != 0x27) {
            elapsed = GetElapsedTimeFromPacket(line);
            Distance = GetDistanceFromPacket(line);
            KCal = GetCaloriesFromPacket(line);
            Speed = GetSpeedFromPacket(line);
            Inclination = GetInclinationFromPacket(line);
            Heart = GetHeartRateFromPacket(line);

            emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
            emit debug(QStringLiteral("Current incline: ") + QString::number(Inclination.value()));
            emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
            emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
            emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        }
    }
}

uint8_t toorxtreadmill::GetHeartRateFromPacket(const QByteArray &packet) { return packet.at(16); }

uint8_t toorxtreadmill::GetInclinationFromPacket(const QByteArray &packet) { return packet.at(15); }

double toorxtreadmill::GetSpeedFromPacket(const QByteArray &packet) {
    QSettings settings;
    // the treadmill send the speed in miles for some models
    double miles = 1;
    if (settings.value(QZSettings::sole_treadmill_miles, QZSettings::default_sole_treadmill_miles).toBool())
        miles = 1.60934;

    double convertedData = ((double)((double)((uint8_t)packet.at(13)) * 100.0) + ((double)packet.at(14))) / 100.0;
    return convertedData * miles;
}

uint16_t toorxtreadmill::GetCaloriesFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(11) << 8) | packet.at(12);
    return convertedData;
}

double toorxtreadmill::GetDistanceFromPacket(const QByteArray &packet) {
    double convertedData = (double)((packet.at(9) << 8) | packet.at(10)) / 100.0;
    return convertedData;
}

uint16_t toorxtreadmill::GetElapsedTimeFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return convertedData;
}

void toorxtreadmill::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    qDebug() << QStringLiteral("onSocketErrorOccurred ") << error;
}
