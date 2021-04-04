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
    return isRunning() && tcpSocket->write(data.toLatin1()) > 0;
}

void TcpClientInfoSender::innerStop() {
    if (tcpSocket) {
        if (isRunning()) {
            tcpSocket->close();
            disconnect(tcpSocket, SIGNAL(disconnected()));
            connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));
        }
        else
            delete tcpSocket;
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
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(reinit()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    tcpSocket->connectToHost(ip, (uint16_t)port);
    return true;
}

