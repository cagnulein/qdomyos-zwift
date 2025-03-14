#include "tcpclientinfosender.h"

TcpClientInfoSender::TcpClientInfoSender(const QString &id, QObject *parent) : TemplateInfoSender(id, parent) {}
TcpClientInfoSender::~TcpClientInfoSender() {
    TcpClientInfoSender::innerStop(); // NOTE: clang-analyzer-optin-cplusplus-virtualcall
}

bool TcpClientInfoSender::isRunning() const { return tcpSocket && tcpSocket->state() == QTcpSocket::ConnectedState; }

bool TcpClientInfoSender::send(const QString &data) {
    if (isRunning()) {
        return tcpSocket->write(data.toLatin1()) > 0;
    } else if (tcpSocket) {
        qDebug() << QStringLiteral("TcpSocket state is ") << tcpSocket->state();
    }
    return false;
}

void TcpClientInfoSender::innerStop() {
    if (tcpSocket) {
        if (TcpClientInfoSender::isRunning()) {
            tcpSocket->close();
            disconnect(tcpSocket, SIGNAL(connectionClosed()));
            disconnect(tcpSocket, SIGNAL(readyRead()));
            disconnect(tcpSocket, SIGNAL(socketError(int)));
            disconnect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)));
            connect(tcpSocket, SIGNAL(connectionClosed()), tcpSocket, SLOT(deleteLater()));
            tcpSocket = 0;
        } else {
            tcpSocket->deleteLater();
            tcpSocket = 0;
        }
    }
}

void TcpClientInfoSender::readyRead() {
    QByteArray read = tcpSocket->readAll();
    qDebug() << QStringLiteral("Message received") << read;
    emit onDataReceived(read);
}

bool TcpClientInfoSender::init() {
    bool ok;
    ip = settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_ip"), QStringLiteral("127.0.0.1"))
             .toString();
    port = settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_port"), QStringLiteral("4321"))
               .toInt(&ok);
    if (!ok) {
        port = 4321;
    }
    if (ip.isEmpty()) {
        ip = QStringLiteral("127.0.0.1");
    }
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QAbstractSocket::connected, this, &TcpClientInfoSender::debugConnected);
    connect(tcpSocket, SIGNAL(connectionClosed()), this, SLOT(reinit()));
    connect(tcpSocket, &QIODevice::readyRead, this, &TcpClientInfoSender::readyRead);
    connect(tcpSocket, SIGNAL(error(int)), this, SLOT(socketError(int)));
    connect(tcpSocket, &QAbstractSocket::stateChanged, this, &TcpClientInfoSender::stateChanged);
    tcpSocket->connectToHost(ip, (uint16_t)port);
    return true;
}

void TcpClientInfoSender::debugConnected() { qDebug() << "Connected" << tcpSocket->state(); }

void TcpClientInfoSender::socketError(int err) {
    qDebug() << QStringLiteral("SocketError") << err;
    reinit();
}

void TcpClientInfoSender::stateChanged(QAbstractSocket::SocketState socketState) {
    qDebug() << QStringLiteral("Socket State Changed to") << socketState;
    if (socketState == QAbstractSocket::SocketState::UnconnectedState) {
        reinit();
    }
}
