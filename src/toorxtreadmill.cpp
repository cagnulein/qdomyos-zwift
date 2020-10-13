#include "toorxtreadmill.h"
#include <QMetaEnum>
#include <QBluetoothLocalDevice>

toorxtreadmill::toorxtreadmill()
{

}

void toorxtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    if(device.name().startsWith("TRX ROUTE KEY"))
    {
        bttreadmill = device;

        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
                this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));

        // Start a discovery
        discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        return;
    }
}

// In your local slot, read information about the found devices
void toorxtreadmill::serviceDiscovered(const QBluetoothServiceInfo &service)
{
    if(service.device().address() == bttreadmill.address())
    {
        debug("Found new service:" + service.serviceName()
                 + '(' + service.serviceUuid().toString() + ')');

        if(service.serviceUuid() == QBluetoothUuid::SerialPort)
        {
            debug("Serial port service found");
            discoveryAgent->stop();

            serialPortService = service;
            socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
            debug("Create socket");
            socket->connectToService(serialPortService);
            debug("ConnectToService done");

            connect(socket, &QBluetoothSocket::readyRead, this, &toorxtreadmill::readSocket);
            connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&toorxtreadmill::rfCommConnected));
            connect(socket, &QBluetoothSocket::disconnected, this, &toorxtreadmill::disconnected);
            connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error),
                    this, &toorxtreadmill::onSocketErrorOccurred);
        }
    }
}

void toorxtreadmill::rfCommConnected()
{
    debug("connected " + socket->peerName());
}

void toorxtreadmill::readSocket()
{
    if (!socket)
        return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        debug(socket->peerName() +
                             QString::fromUtf8(line.constData(), line.length()));
    }
}

void toorxtreadmill::onSocketErrorOccurred(QBluetoothSocket::SocketError error)
{
    debug("onSocketErrorOccurred " + QString::number(error));
}
