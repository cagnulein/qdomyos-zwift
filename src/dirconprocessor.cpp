#include "dirconprocessor.h"
#include "dirconpacket.h"

DirconProcessor::DirconProcessor(DirconProcessorService *my_service, const QString &my_mac, QObject *parent)
    : QObject(parent), service(my_service), mac(my_mac) {
    qDebug() << "In the constructor of dircon processor for" << my_service->uuid;
    my_service->setParent(this);
}

DirconProcessor::~DirconProcessor() {}

bool DirconProcessor::initServer() {
    qDebug() << "Initializing dircon tcp server for" << service->uuid;
    if (!server) {
        server = new QTcpServer(this);
        connect(server, SIGNAL(newConnection()), this, SLOT(tcpNewConnection()));
        qDebug() << "Dircon TCP Server built" << service->uuid;
    }
    if (!server->isListening()) {
        qDebug() << "Dircon TCP Server trying to listen" << service->serverPort;
        return server->listen(QHostAddress::Any, service->serverPort);
    } else
        return true;
}

void DirconProcessor::initAdvertising() {
    /*    if (!zeroConf) {
            qDebug() << "Dircon Adv init for" << service->uuid;
            zeroConf = new QZeroConf(this);
            zeroConf->addServiceTxtRecord(
                "ble-service-uuids",
                QString(QStringLiteral(DP_BASE_UUID))
                    .replace("u", QString(QStringLiteral("%1")).arg(service->uuid, 4, 16, QLatin1Char('0'))));
            zeroConf->addServiceTxtRecord("mac-address", mac);
            zeroConf->addServiceTxtRecord("serial-number", service->serialN);
            zeroConf->startServicePublish(service->serverName.toUtf8().constData(), "_wahoo-fitness-tnp._tcp", "local",
                                          service->serverPort);
            connect(zeroConf, SIGNAL(servicePublished()), this, SLOT(advOK()));
            connect(zeroConf, SIGNAL(error(QZeroConf::error_t)), this, SLOT(advError(QZeroConf::error_t)));
        }*/
    if (!mdnsServer) {
        qDebug() << "Dircon Adv init for" << service->uuid;
        mdnsServer = new QMdnsEngine::Server(this);
        mdnsHostname =
            new QMdnsEngine::Hostname(mdnsServer, service->serverName.toUtf8() + QByteArrayLiteral("H"), this);
        mdnsProvider = new QMdnsEngine::Provider(mdnsServer, mdnsHostname, this);
        QMdnsEngine::Service mdnsService;
        mdnsService.setType("_wahoo-fitness-tnp._tcp.local.");
        mdnsService.setName(service->serverName.toUtf8());
        mdnsService.addAttribute(QByteArrayLiteral("mac-address"), mac.toUtf8());
        mdnsService.addAttribute(QByteArrayLiteral("serial-number"), service->serialN.toUtf8());
        mdnsService.addAttribute(
            QByteArrayLiteral("ble-service-uuids"),
            QString(QStringLiteral(DP_BASE_UUID))
                .replace("u", QString(QStringLiteral("%1")).arg(service->uuid, 4, 16, QLatin1Char('0')))
                .toUtf8());
        mdnsService.setPort(service->serverPort);
        mdnsProvider->update(mdnsService);
        qDebug() << "Dircon Adv init for" << service->uuid << " end";
    }
}

void DirconProcessor::advOK() {
    qDebug() << "Service Published OK uuid=" << service->uuid << " name=" << service->serverName
             << " sn=" << service->serialN;
}

/*void DirconProcessor::advError(QZeroConf::error_t err) {
    qDebug() << "Service Published ERROR (" << err << ") uuid=" << service->uuid << " name=" << service->serverName
             << " sn=" << service->serialN;
}*/

bool DirconProcessor::init() {
    qDebug() << "Dircon Processor init for" << service->uuid;
    bool rv = initServer();
    qDebug() << "Dircon TCP Server RV" << rv;
    if (rv)
        initAdvertising();
    else
        qDebug() << "Cannot init dircon TCP server at port" << service->serverPort;
    return rv;
}

void DirconProcessor::tcpNewConnection() {
    QTcpSocket *socket = server->nextPendingConnection();
    qDebug() << "New connection from" << socket->peerAddress().toString() << ":" << socket->peerPort()
             << " uuid = " << service->uuid;
    connect(socket, SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(tcpDataAvailable()));
    DirconProcessorClient *client = new DirconProcessorClient(socket);
    clientsMap.insert(socket, client);
}

void DirconProcessor::tcpDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    qDebug() << "Disconnection from" << socket->peerAddress().toString() << ":" << socket->peerPort()
             << " uuid = " << service->uuid;
    clientsMap.remove(socket);
    socket->deleteLater();
}

DirconPacket DirconProcessor::processPacket(DirconProcessorClient *client, const DirconPacket &pkt) {
    DirconPacket out;
    if (pkt.isRequest) {
        bool cfound = false;
        DirconProcessorCharacteristic *cc;
        out.isRequest = false;
        out.Identifier = pkt.Identifier;
        if (pkt.Identifier == DPKT_MSGID_DISCOVER_SERVICES) {
            out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
            out.uuids.append(service->uuid);
        } else if (pkt.Identifier == DPKT_MSGID_DISCOVER_CHARACTERISTICS) {
            if (pkt.uuid == service->uuid) {
                out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
                out.uuid = pkt.uuid;
                foreach (cc, service->chars) {
                    out.uuids.append(cc->uuid);
                    out.additional_data.append(cc->type);
                }
            } else
                out.ResponseCode = DPKT_RESPCODE_SERVICE_NOT_FOUND;
        } else if (pkt.Identifier == DPKT_MSGID_READ_CHARACTERISTIC) {
            foreach (cc, service->chars) {
                if (cc->uuid == pkt.uuid) {
                    cfound = true;
                    break;
                }
            }
            if (cfound) {
                if (cc->type & DPKT_CHAR_PROP_FLAG_READ) {
                    out.uuid = pkt.uuid;
                    out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
                    out.additional_data.append(cc->read_values);
                    emit onCharacteristicRead(cc->uuid);
                } else
                    out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_OPERATION_NOT_SUPPORTED;
            } else
                out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_NOT_FOUND;
        } else if (pkt.Identifier == DPKT_MSGID_WRITE_CHARACTERISTIC) {
            foreach (cc, service->chars) {
                if (cc->uuid == pkt.uuid) {
                    cfound = true;
                    break;
                }
            }
            if (cfound) {
                if (cc->type & DPKT_CHAR_PROP_FLAG_WRITE) {
                    int res;
                    if (cc->writeP &&
                        (res = cc->writeP->writeProcess(cc->uuid, pkt.additional_data, out.additional_data)) !=
                            CP_INVALID) {
                        out.uuid = pkt.uuid;
                        out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
                    } else
                        out.Identifier = DPKT_MSGID_ERROR;
                    emit onCharacteristicWrite(cc->uuid, pkt.additional_data);
                } else
                    out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_OPERATION_NOT_SUPPORTED;
            } else
                out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_NOT_FOUND;
        } else if (pkt.Identifier == DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS) {
            foreach (cc, service->chars) {
                if (cc->uuid == pkt.uuid) {
                    cfound = true;
                    break;
                }
            }
            if (cfound) {
                if (cc->type & DPKT_CHAR_PROP_FLAG_NOTIFY) {
                    int idx;
                    char notif = pkt.additional_data.at(0);
                    out.uuid = pkt.uuid;

                    if ((idx = client->char_notify.indexOf(pkt.uuid)) >= 0 && !notif)
                        client->char_notify.removeAt(idx);
                    else if (idx < 0 && notif)
                        client->char_notify.append(pkt.uuid);
                    out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
                    emit onCharacteristicNotificationSwitch(cc->uuid, notif);
                } else
                    out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_OPERATION_NOT_SUPPORTED;
            } else
                out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_NOT_FOUND;
        }
    }
    return out;
}

bool DirconProcessor::sendCharacteristicNotification(quint16 uuid, const QByteArray &data) {
    DirconPacket pkt;
    bool rv = true;
    pkt.additional_data = data;
    pkt.Identifier = DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION;
    pkt.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    pkt.uuid = uuid;
    for (QHash<QTcpSocket *, DirconProcessorClient *>::iterator i = clientsMap.begin(); i != clientsMap.end(); ++i) {
        DirconProcessorClient *client = i.value();
        if (client->char_notify.indexOf(uuid) >= 0) {
            if (i.key()->write(pkt.encode(0)) < 0)
                rv = false;
        }
    }
    return rv;
}

void DirconProcessor::tcpDataAvailable() {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    DirconProcessorClient *client = clientsMap.value(socket);
    QByteArray data = socket->readAll();
    qDebug() << "Data available for uuid " << service->uuid << ":" << data.toHex();
    if (client) {
        int buflimit, rembuf;
        client->buffer.append(data);
        while (1) {
            DirconPacket pkt;
            buflimit = pkt.parse(client->buffer, client->seq);
            qDebug() << "Pkt for uuid" << service->uuid << "parsed rv=" << buflimit << " ->" << pkt.toString();
            if (buflimit > 0) {
                rembuf = buflimit;
                if (pkt.isRequest)
                    client->seq = pkt.SequenceNumber;
                else if (pkt.Identifier != DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION)
                    client->seq += 1;
            } else if (buflimit < DPKT_PARSE_ERROR) {
                rembuf = -buflimit - DPKT_PARSE_ERROR;
                qDebug() << "Unexpected packet" << client->buffer.mid(0, rembuf).toHex();
            } else
                rembuf = -1;
            if (rembuf >= 0)
                client->buffer = client->buffer.mid(rembuf);
            if (buflimit > 0) {
                DirconPacket resp = processPacket(client, pkt);
                qDebug() << "Sending resp for uuid" << service->uuid << ":" << resp.toString();
                if (resp.Identifier != DPKT_MSGID_ERROR) {
                    QByteArray byteout = resp.encode(pkt.SequenceNumber);
                    if (byteout.size())
                        client->sock->write(byteout);
                }
            } else if (rembuf >= 0) {
                DirconPacket resp;
                resp.isRequest = false;
                resp.ResponseCode = DPKT_RESPCODE_UNEXPECTED_ERROR;
                resp.Identifier = pkt.Identifier;
                QByteArray byteout = resp.encode(pkt.SequenceNumber);
                if (byteout.size())
                    client->sock->write(byteout);
            } else
                break;
        }
    }
}
