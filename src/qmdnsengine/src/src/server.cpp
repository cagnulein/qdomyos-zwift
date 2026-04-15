/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nathan Osman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <QtGlobal>

#ifdef Q_OS_UNIX
#  include <cerrno>
#  include <cstring>
#  include <sys/socket.h>
#endif

#include <QHostAddress>
#include <QNetworkInterface>

#include <qmdnsengine/dns.h>
#include <qmdnsengine/mdns.h>
#include <qmdnsengine/message.h>
#include <qmdnsengine/server.h>

#include "server_p.h"

using namespace QMdnsEngine;

ServerPrivate::ServerPrivate(Server *server)
    : QObject(server),
      q(server)
{
    connect(&timer, &QTimer::timeout, this, &ServerPrivate::onTimeout);
    connect(&ipv4Socket, &QUdpSocket::readyRead, this, &ServerPrivate::onReadyRead);
    connect(&ipv6Socket, &QUdpSocket::readyRead, this, &ServerPrivate::onReadyRead);

    timer.setInterval(60 * 1000);
    timer.setSingleShot(true);
    onTimeout();
}

bool ServerPrivate::bindSocket(QUdpSocket &socket, const QHostAddress &address)
{
    // Exit early if the socket is already bound
    if (socket.state() == QAbstractSocket::BoundState) {
        return true;
    }

    // I cannot find the correct combination of flags that allows the socket
    // to bind properly on Linux, so on that platform, we must manually create
    // the socket and initialize the QUdpSocket with it

#ifdef Q_OS_UNIX
    if (!socket.bind(address, MdnsPort, QAbstractSocket::ShareAddress)) {
        int arg = 1;
        if (setsockopt(socket.socketDescriptor(), SOL_SOCKET, SO_REUSEADDR,
                reinterpret_cast<char*>(&arg), sizeof(int))) {
            emit q->error(strerror(errno));
            return false;
        }
#endif
        if (!socket.bind(address, MdnsPort, QAbstractSocket::ReuseAddressHint)) {
            emit q->error(socket.errorString());
            return false;
        }
#ifdef Q_OS_UNIX
    }
#endif

    return true;
}

void ServerPrivate::onTimeout()
{
    // A timer is used to run a set of operations once per minute; first, the
    // two sockets are bound - if this fails, another attempt is made once per
    // timeout; secondly, all network interfaces are enumerated; if the
    // interface supports multicast, the socket will join the mDNS multicast
    // groups

    bool ipv4Bound = bindSocket(ipv4Socket, QHostAddress::AnyIPv4);
    bool ipv6Bound = bindSocket(ipv6Socket, QHostAddress::AnyIPv6);

    if (ipv4Bound || ipv6Bound) {
        const auto interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &networkInterface : interfaces) {
            if (networkInterface.flags() & QNetworkInterface::CanMulticast) {
                if (ipv4Bound) {
                    ipv4Socket.joinMulticastGroup(MdnsIpv4Address, networkInterface);
                }
                if (ipv6Bound) {
                    ipv6Socket.joinMulticastGroup(MdnsIpv6Address, networkInterface);
                }
            }
        }
    }

    timer.start();
}

void ServerPrivate::onReadyRead()
{
    // Read the packet from the socket
    QUdpSocket *socket = qobject_cast<QUdpSocket*>(sender());
    QByteArray packet;
    packet.resize(socket->pendingDatagramSize());
    QHostAddress address;
    quint16 port;
    socket->readDatagram(packet.data(), packet.size(), &address, &port);

    // Attempt to decode the packet
    Message message;
    if (fromPacket(packet, message)) {
        message.setAddress(address);
        message.setPort(port);
        emit q->messageReceived(message);
    }
}

Server::Server(QObject *parent)
    : AbstractServer(parent),
      d(new ServerPrivate(this))
{
}

void Server::sendMessage(const Message &message)
{
    QByteArray packet;
    toPacket(message, packet);
    if (message.address().protocol() == QAbstractSocket::IPv4Protocol) {
        d->ipv4Socket.writeDatagram(packet, message.address(), message.port());
    } else {
        d->ipv6Socket.writeDatagram(packet, message.address(), message.port());
    }
}

void Server::sendMessageToAll(const Message &message)
{
    QByteArray packet;
    toPacket(message, packet);
    d->ipv4Socket.writeDatagram(packet, MdnsIpv4Address, MdnsPort);
    d->ipv6Socket.writeDatagram(packet, MdnsIpv6Address, MdnsPort);
}
