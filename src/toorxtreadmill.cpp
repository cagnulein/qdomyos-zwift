#include "toorxtreadmill.h"
#include <QMetaEnum>
#include <QBluetoothLocalDevice>

toorxtreadmill::toorxtreadmill()
{
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(200);
}

void toorxtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("TRX ROUTE KEY"))
    {
        bluetoothDevice = device;

        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
                this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));

        // Start a discovery
        qDebug() << "toorxtreadmill::deviceDiscovered";
        discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        return;
    }
}

// In your local slot, read information about the found devices
void toorxtreadmill::serviceDiscovered(const QBluetoothServiceInfo &service)
{
    qDebug() << "toorxtreadmill::serviceDiscovered" << service;
    if(service.device().address() == bluetoothDevice.address())
    {
        debug("Found new service: " + service.serviceName()
                 + '(' + service.serviceUuid().toString() + ')');

        if(service.serviceName().startsWith("SerialPort") || service.serviceName().startsWith("Serial Port"))
        {
            debug("Serial port service found");
            discoveryAgent->stop();

            serialPortService = service;
            socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

            connect(socket, &QBluetoothSocket::readyRead, this, &toorxtreadmill::readSocket);
            connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&toorxtreadmill::rfCommConnected));
            connect(socket, &QBluetoothSocket::disconnected, this, &toorxtreadmill::disconnected);
            connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error),
                    this, &toorxtreadmill::onSocketErrorOccurred);

#ifdef Q_OS_ANDROID
            socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);
#endif

            debug("Create socket");
            socket->connectToService(serialPortService);
            debug("ConnectToService done");
        }
    }
}

void toorxtreadmill::update()
{
    if(initDone)
    {
        const char poll[] = {0x55, 0x17, 0x01, 0x01, 0x53};
        socket->write(poll, sizeof(poll));
        debug("write poll");
    }
}

void toorxtreadmill::rfCommConnected()
{
    debug("connected " + socket->peerName());
    initDone = true;
    emit connectedAndDiscovered();
}

void toorxtreadmill::readSocket()
{
    if (!socket)
        return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        debug(socket->peerName() +
                             QString::fromUtf8(line.constData(), line.length()));

        if(line.length() == 17)
        {
            elapsed = GetElapsedTimeFromPacket(line);
            Distance = GetDistanceFromPacket(line);
            KCal = GetCaloriesFromPacket(line);
            Speed = GetSpeedFromPacket(line);
            Inclination = GetInclinationFromPacket(line);
            Heart = GetHeartRateFromPacket(line);
        }
    }
}

uint8_t toorxtreadmill::GetHeartRateFromPacket(QByteArray packet)
{
    return packet.at(16);
}

uint8_t toorxtreadmill::GetInclinationFromPacket(QByteArray packet)
{
    return packet.at(15);
}

double toorxtreadmill::GetSpeedFromPacket(QByteArray packet)
{
    double convertedData = (double)(packet.at(13) << 8) + ((double)packet.at(14) / 100.0);
    return convertedData;
}

uint16_t toorxtreadmill::GetCaloriesFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(11) << 8) | packet.at(12);
    return convertedData;
}


uint16_t toorxtreadmill::GetDistanceFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(9) << 8) | packet.at(10);
    return convertedData;
}


uint16_t toorxtreadmill::GetElapsedTimeFromPacket(QByteArray packet)
{
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    return convertedData;
}

void toorxtreadmill::onSocketErrorOccurred(QBluetoothSocket::SocketError error)
{
    debug("onSocketErrorOccurred " + QString::number(error));
}
