#ifndef DIRCONPROCESSOR_H
#define DIRCONPROCESSOR_H

#include "characteristicwriteprocessor.h"
#include "dirconpacket.h"
#include <QHash>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "qmdnsengine/server.h"
#include "qmdnsengine/provider.h"
#include "qmdnsengine/hostname.h"
#include "qmdnsengine/service.h"

class DirconProcessorCharacteristic : public QObject {
  public:
    DirconProcessorCharacteristic(QObject *parent = nullptr) : QObject(parent), uuid(0), type(0), writeP(0) {}
    DirconProcessorCharacteristic(quint16 my_uuid, quint8 my_type, const QByteArray &data,
                                  CharacteristicWriteProcessor *w, QObject *parent = nullptr)
        : QObject(parent), read_values(data), uuid(my_uuid), type(my_type), writeP(w) {}
    DirconProcessorCharacteristic(const DirconProcessorCharacteristic &cp) { this->operator=(cp); }
    ~DirconProcessorCharacteristic() {}
    DirconProcessorCharacteristic &operator=(const DirconProcessorCharacteristic &cp) {
        read_values = cp.read_values;
        uuid = cp.uuid;
        type = cp.type;
        writeP = cp.writeP;
        return *this;
    }
    QByteArray read_values;
    quint16 uuid;
    quint8 type;
    CharacteristicWriteProcessor *writeP;
};

class DirconProcessorService : public QObject {
  public:
    DirconProcessorService(const QString &name, const QString &serv_name, quint16 serv_port, const QString &serv_sn,
                           quint16 my_uuid, QObject *parent = nullptr)
        : QObject(parent), serverPort(serv_port), serialN(serv_sn), serverName(serv_name), name(name), uuid(my_uuid) {}
    quint16 serverPort;
    QString serialN;
    QString serverName;
    QString name;
    quint16 uuid;
    QList<DirconProcessorCharacteristic *> chars;
};

#define DP_BASE_UUID "0000u-0000-1000-8000-00805F9B34FB"
// QString("%1").arg(iTest & 0xFFFF, 4, 16);

class DirconProcessorClient : public QObject {
  public:
    DirconProcessorClient(QTcpSocket *sock) : QObject(sock), sock(sock) {}
    quint8 seq = 0;
    QList<quint16> char_notify;
    QTcpSocket *sock;
    QByteArray buffer;
};

class DirconProcessor : public QObject {
    Q_OBJECT
    DirconProcessorService *service;
    QTcpServer *server = 0;
    QString mac;
    QMdnsEngine::Server *mdnsServer = 0;
    QMdnsEngine::Provider *mdnsProvider = 0;
    QMdnsEngine::Hostname *mdnsHostname = 0;
    QHash<QTcpSocket *, DirconProcessorClient *> clientsMap;
    bool initServer();
    void initAdvertising();
    DirconPacket processPacket(DirconProcessorClient *client, const DirconPacket &pkt);

  public:
    ~DirconProcessor();
    explicit DirconProcessor(DirconProcessorService *service, const QString &mac, QObject *parent = nullptr);
    bool sendCharacteristicNotification(quint16 uuid, const QByteArray &data);
    bool init();
  private slots:
    void tcpDataAvailable();
    void tcpDisconnected();
    void tcpNewConnection();
    void advOK();
    // void advError(QZeroConf::error_t err);
  signals:
    void onCharacteristicRead(quint16 uuid);
    void onCharacteristicWrite(quint16 uuid, QByteArray data);
    void onCharacteristicNotificationSwitch(quint16 uuid, char switchval);
};

#endif // DIRCONPROCESSOR_H
