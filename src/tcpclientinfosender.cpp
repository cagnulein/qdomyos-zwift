#include "tcpclientinfosender.h"

TcpClientInfoSender::TcpClientInfoSender(const QString& id, QObject * parent):TemplateInfoSender(id, parent) {

}
TcpClientInfoSender::~TcpClientInfoSender() {
    innerStop();
}

bool TcpClientInfoSender::isRunning() const {
    return tcpSocket && tcpSocket->state() == QTcpSocket::ConnectedState;
}


bool TcpClientInfoSender::send(const QString& data) {
    if (isRunning())
        return tcpSocket->write(data.toLatin1()) > 0;
    else if (tcpSocket)
        qDebug() << "TcpSocket state is "<<tcpSocket->state();
    return false;
}

void TcpClientInfoSender::innerStop() {
    if (tcpSocket) {
        if (isRunning()) {
            tcpSocket->close();
            disconnect(tcpSocket, SIGNAL(connectionClosed()));
            disconnect(tcpSocket, SIGNAL(readyRead()));
            disconnect(tcpSocket, SIGNAL(socketError(int)));
            disconnect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)));
            connect(tcpSocket, SIGNAL(connectionClosed()), tcpSocket, SLOT(deleteLater()));
            tcpSocket = 0;
        }
        else {
            tcpSocket->deleteLater();
            tcpSocket = 0;
        }
    }
}

void TcpClientInfoSender::readyRead()
{
    QByteArray read = tcpSocket->readAll();
    qDebug() << "Measage received" << read;
    emit onDataReceived(read);
}

bool TcpClientInfoSender::init() {
    bool ok;
    ip = settings.value("template_" + templateId + "_ip", "127.0.0.1").toString();
    port = settings.value("template_" + templateId + "_port", "4321").toInt(&ok);
    if (!ok)
        port = 4321;
    if (ip.isEmpty())
        ip = "127.0.0.1";
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(debugConnected()));
    connect(tcpSocket, SIGNAL(connectionClosed()), this, SLOT(reinit()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(tcpSocket, SIGNAL(error(int)), this, SLOT(socketError(int)));
    connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    tcpSocket->connectToHost(ip, (uint16_t)port);
    return true;
}

void TcpClientInfoSender::debugConnected() {
    qDebug() << "Connected"<<tcpSocket->state();
}

void TcpClientInfoSender::socketError(int err) {
    qDebug() << "SocketError"<< err;
    reinit();
}

void TcpClientInfoSender::stateChanged(QAbstractSocket::SocketState socketState) {
    qDebug() << "Socket State Changed to"<< socketState;
    if (socketState == QAbstractSocket::SocketState::UnconnectedState)
        reinit();
}
