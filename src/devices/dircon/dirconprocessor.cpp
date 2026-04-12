#include "dirconprocessor.h"
#include "dirconpacket.h"
#include "qzsettings.h"
#include <QSettings>
#include <QHostInfo>

DirconProcessor::DirconProcessor(const QList<DirconProcessorService *> &my_services, const QString &serv_name,
                                 quint16 serv_port, const QString &serv_sn, const QString &my_mac, QObject *parent)
    : QObject(parent), services(my_services), mac(my_mac), serverPort(serv_port), serialN(serv_sn),
      serverName(serv_name) {
    qDebug() << "In the constructor of dircon processor for" << serverName;
    QSettings settings;
    rouvy_compatibility = settings.value(QZSettings::rouvy_compatibility, QZSettings::default_rouvy_compatibility).toBool();
    foreach (DirconProcessorService *my_service, my_services) { my_service->setParent(this); }
}

DirconProcessor::~DirconProcessor() {}

QString DirconProcessor::convertUUIDFromUINT16ToString (quint16 uuid) {
    if(uuid == ZWIFT_PLAY_CHAR1_ENUM_VALUE)
        return ZWIFT_PLAY_CHAR1_UUID_STRING;
    if(uuid == ZWIFT_PLAY_CHAR2_ENUM_VALUE)
        return ZWIFT_PLAY_CHAR2_UUID_STRING;
    if(uuid == ZWIFT_PLAY_CHAR3_ENUM_VALUE)
        return ZWIFT_PLAY_CHAR3_UUID_STRING;
    return "";
}

bool DirconProcessor::initServer() {
    qDebug() << "Initializing dircon tcp server for" << serverName;
    if (!server) {
        server = new QTcpServer(this);
        connect(server, SIGNAL(newConnection()), this, SLOT(tcpNewConnection()));
        qDebug() << "Dircon TCP Server built" << serverName;
    }
    if (!server->isListening()) {
        qDebug() << "Dircon TCP Server trying to listen" << serverPort;
        // Listen only on IPv4 for Apple TV/Windows compatibility (like Elite Avanti) when Rouvy compatibility is enabled
        return server->listen(rouvy_compatibility ? QHostAddress::AnyIPv4 : QHostAddress::Any, serverPort);
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
        qDebug() << "Dircon Adv init for" << serverName;
        mdnsServer = new QMdnsEngine::Server(this);
        mdnsHostname = new QMdnsEngine::Hostname(mdnsServer, serverName.toUtf8() + QByteArrayLiteral("H"), this);
        mdnsProvider = new QMdnsEngine::Provider(mdnsServer, mdnsHostname, this);
        QMdnsEngine::Service mdnsService;
        mdnsService.setType(rouvy_compatibility ? "_wahoo-fitness-tnp._tcp.local" : "_wahoo-fitness-tnp._tcp.local.");
        mdnsService.setName(serverName.toUtf8());
        mdnsService.addAttribute(QByteArrayLiteral("mac-address"), mac.toUtf8());
        mdnsService.addAttribute(QByteArrayLiteral("serial-number"), serialN.toUtf8());
        QString ble_uuids;
        if (rouvy_compatibility) {
            QStringList uuid_list;
            foreach (DirconProcessorService *service, services) {
                // Filter: only advertise 0x1826 for KICKR (skip 0x1818, 0x1816)
                if(service->uuid == 0x1818 || service->uuid == 0x1816) {
                    continue;
                }

                if(service->uuid == ZWIFT_PLAY_ENUM_VALUE) {
                    uuid_list.append(ZWIFT_PLAY_UUID_STRING);
                } else {
                    // Use short format with 0x prefix (Apple TV/Windows compatibility)
                    uuid_list.append(QString(QStringLiteral("0x%1"))
                        .arg(service->uuid, 4, 16, QLatin1Char('0')));
                }
            }
            ble_uuids = uuid_list.join(",");
        } else {
            int i = 0;
            foreach (DirconProcessorService *service, services) {
                if(service->uuid == ZWIFT_PLAY_ENUM_VALUE) {
                    ble_uuids += ZWIFT_PLAY_UUID_STRING +
                        ((i++ < services.size() - 1) ? QStringLiteral(",") : QStringLiteral(""));
                } else {
                    ble_uuids += QString(QStringLiteral(DP_BASE_UUID))
                        .replace("u", QString(QStringLiteral("%1")).arg(service->uuid, 4, 16, QLatin1Char('0'))) +
                    ((i++ < services.size() - 1) ? QStringLiteral(",") : QStringLiteral(""));
                }
            }
        }
        mdnsService.addAttribute(QByteArrayLiteral("ble-service-uuids"), ble_uuids.toUtf8());
        mdnsService.setPort(serverPort);
        mdnsProvider->update(mdnsService);
        qDebug() << "Dircon Adv init for" << serverName << " end";
    }
}

bool DirconProcessor::init() {
    qDebug() << "Dircon Processor init for" << serverName;
    bool rv = initServer();
    qDebug() << "Dircon TCP Server RV" << rv;
    if (rv)
        initAdvertising();
    else
        qDebug() << "Cannot init dircon TCP server at port" << serverPort;
    return rv;
}

void DirconProcessor::tcpNewConnection() {
    QTcpSocket *socket = server->nextPendingConnection();
    qDebug() << "New connection from" << socket->peerAddress().toString() << ":" << socket->peerPort()
             << " uuid = " << serverName;
    connect(socket, SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(tcpDataAvailable()));
    DirconProcessorClient *client = new DirconProcessorClient(socket);
    clientsMap.insert(socket, client);

    if (rouvy_compatibility) {
        // Send initial notification for 0x2AD2 (Indoor Bike Data) - Apple TV/Windows compatibility
        // Elite Avanti sends this immediately after connection
        DirconPacket initPkt;
        initPkt.isRequest = false;
        initPkt.Identifier = DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION;
        initPkt.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
        initPkt.uuid = 0x2AD2;
        initPkt.additional_data = QByteArray(29, 0x00); // Empty data for now
        QByteArray initData = initPkt.encode(0);
        socket->write(initData);
        socket->flush();
        qDebug() << "Sent initial notification for 0x2AD2 to" << socket->peerAddress().toString();
    }
}

void DirconProcessor::tcpDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    qDebug() << "Disconnection from" << socket->peerAddress().toString() << ":" << socket->peerPort()
             << " uuid = " << serverName;
    clientsMap.remove(socket);
    socket->deleteLater();
}

DirconPacket DirconProcessor::processPacket(DirconProcessorClient *client, const DirconPacket &pkt) {
    DirconPacket out;
    if (pkt.isRequest) {
        bool cfound = false;
        DirconProcessorCharacteristic *cc;
        DirconProcessorService *service;
        out.isRequest = false;
        out.Identifier = pkt.Identifier;
        if (pkt.Identifier == DPKT_MSGID_DISCOVER_SERVICES) {
            out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
            foreach (service, services)
                out.uuids.append(service->uuid);
        } else if (pkt.Identifier == DPKT_MSGID_DISCOVER_CHARACTERISTICS) {
            foreach (service, services)
                if (service->uuid == pkt.uuid) {
                    cfound = true;
                    out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
                    out.uuid = pkt.uuid;
                    foreach (cc, service->chars) {
                        out.uuids.append(cc->uuid);
                        out.additional_data.append(cc->type);
                    }
                    break;
                }
            if (!cfound)
                out.ResponseCode = DPKT_RESPCODE_SERVICE_NOT_FOUND;
        } else if (pkt.Identifier == DPKT_MSGID_READ_CHARACTERISTIC) {
            foreach (service, services) {
                foreach (cc, service->chars) {
                    if (cc->uuid == pkt.uuid) {
                        cfound = true;
                        if (cc->type & DPKT_CHAR_PROP_FLAG_READ) {
                            out.uuid = pkt.uuid;
                            out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
                            out.additional_data.append(cc->read_values);
                            emit onCharacteristicRead(cc->uuid);
                        } else
                            out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_OPERATION_NOT_SUPPORTED;
                        break;
                    }
                }
                if (cfound)
                    break;
            }
            if (!cfound)
                out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_NOT_FOUND;
        } else if (pkt.Identifier == DPKT_MSGID_WRITE_CHARACTERISTIC) {
            foreach (service, services) {
                foreach (cc, service->chars) {
                    if (cc->uuid == pkt.uuid) {
                        cfound = true;
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
                        break;
                    }
                }
                if (cfound)
                    break;
            }
            if (!cfound)
                out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_NOT_FOUND;
        } else if (pkt.Identifier == DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS) {
            foreach (service, services) {
                foreach (cc, service->chars) {
                    if (cc->uuid == pkt.uuid) {
                        cfound = true;
                        if (cc->type & (DPKT_CHAR_PROP_FLAG_NOTIFY | DPKT_CHAR_PROP_FLAG_INDICATE)) {
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
                        break;
                    }
                }
                if (cfound)
                    break;
            }
            if (!cfound)
                out.ResponseCode = DPKT_RESPCODE_CHARACTERISTIC_NOT_FOUND;
        } else if (pkt.Identifier == DPKT_MSGID_UNKNOWN_0x07) {
            // Unknown message 0x07 - respond with success
            out.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
        }
    }
    return out;
}

bool DirconProcessor::sendCharacteristicNotification(quint16 uuid, const QByteArray &data) {
    DirconPacket pkt;
    QTcpSocket *socket;
    DirconProcessorClient *client;
    QSettings settings;
    bool rv = true, rvs;
    pkt.additional_data = data;
    pkt.Identifier = DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION;
    pkt.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    pkt.uuid = uuid;
    for (QHash<QTcpSocket *, DirconProcessorClient *>::iterator i = clientsMap.begin(); i != clientsMap.end(); ++i) {
        client = i.value();
        if (client->char_notify.indexOf(uuid) >= 0 || !settings.value(QZSettings::wahoo_rgt_dircon, QZSettings::default_wahoo_rgt_dircon).toBool()) {
            socket = i.key();
            rvs = socket->write(pkt.encode(0)) < 0;
            if (rvs)
                rv = false;
            qDebug() << serverName << "sending to" << socket->peerAddress().toString() << ":" << socket->peerPort()
                     << " notification for uuid = " << QString(QStringLiteral("%1")).arg(uuid, 4, 16, QLatin1Char('0'))
                     << "rv=" << (!rvs) << data.toHex(' ');
        }
    }
    return rv;
}

void DirconProcessor::tcpDataAvailable() {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    DirconProcessorClient *client = clientsMap.value(socket);
    QByteArray data = socket->readAll();
    qDebug() << "Data available for uuid " << serverName << ":" << data.toHex();
    if (client) {
        int buflimit, rembuf;
        client->buffer.append(data);
        while (1) {
            DirconPacket pkt;
            buflimit = pkt.parse(client->buffer, client->seq);
            qDebug() << "Pkt for uuid" << serverName << "parsed rv=" << buflimit << " ->" << pkt;
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
                qDebug() << "Sending resp for uuid" << serverName << ":" << resp;
                if (resp.Identifier != DPKT_MSGID_ERROR) {
                    QByteArray byteout = resp.encode(pkt.SequenceNumber);
                    if (byteout.size() && client && client->sock)
                        client->sock->write(byteout);
                }
            } else if (rembuf >= 0) {
                DirconPacket resp;
                resp.isRequest = false;
                resp.ResponseCode = DPKT_RESPCODE_UNEXPECTED_ERROR;
                resp.Identifier = pkt.Identifier;
                QByteArray byteout = resp.encode(pkt.SequenceNumber);
                if (byteout.size() && client && client->sock)
                    client->sock->write(byteout);
            } else
                break;
        }
    }
}
