#ifndef DIRCONPROCESSOR_H
#define DIRCONPROCESSOR_H

#include "characteristics/characteristicwriteprocessor.h"
#include "dirconpacket.h"
#include "qmdnsengine/hostname.h"
#include "qmdnsengine/provider.h"
#include "qmdnsengine/server.h"
#include "qmdnsengine/service.h"
#include <QHash>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class DirconProcessorCharacteristic : public QObject {
  public:
    DirconProcessorCharacteristic(QObject *parent = nullptr) : QObject(parent), uuid(0), type(0), writeP(0) {}
    DirconProcessorCharacteristic(quint16 my_uuid, quint8 my_type, const QByteArray &data,
                                  CharacteristicWriteProcessor *w, QObject *parent = nullptr)
        : QObject(parent), read_values(data), uuid(my_uuid), type(my_type), writeP(w) {}
    ~DirconProcessorCharacteristic() {}
    DirconProcessorCharacteristic &operator=(const DirconProcessorCharacteristic &cp) {
        read_values = cp.read_values;
        uuid = cp.uuid;
        type = cp.type;
        writeP = cp.writeP;
        return *this;
    }
    operator QString() const {
        return QString(QStringLiteral("uuid=%1 read=%2 type=%3"))
            .arg(uuid, 4, 16, QLatin1Char('0'))
            .arg(QString(read_values.toHex()))
            .arg(type);
    }
    QByteArray read_values;
    quint16 uuid;
    quint8 type;
    CharacteristicWriteProcessor *writeP;
};

class DirconProcessorService : public QObject {
  public:
    DirconProcessorService(const QString &name, quint16 my_uuid, int my_machine_id, QObject *parent = nullptr)
        : QObject(parent), name(name), uuid(my_uuid), machine_id(my_machine_id) {}
    QString name;
    quint16 uuid;
    int machine_id;
    operator QString() const {
        QString charsprint = QString();
        foreach (DirconProcessorCharacteristic *c, chars) { charsprint += *c + QStringLiteral(","); }
        return QString(QStringLiteral("servname=%1 uuid=%2 machine=%3 [%4]"))
            .arg(name)
            .arg(uuid, 4, 16, QLatin1Char('0'))
            .arg(machine_id)
            .arg(charsprint);
    }
    QList<DirconProcessorCharacteristic *> chars;
};

#define DP_BASE_UUID "0000u-0000-1000-8000-00805F9B34FB"
// QString("%1").arg(iTest & 0xFFFF, 4, 16);

class DirconProcessorClient : public QObject {
  public:
    DirconProcessorClient(QTcpSocket *sock) : QObject(sock), sock(sock) {}
    quint8 seq = 0;
    quint8 notification_seq = 0;
    QList<quint16> char_notify;
    QTcpSocket *sock;
    QByteArray buffer;
};

class DirconProcessor : public QObject {
    Q_OBJECT
    QList<DirconProcessorService *> services;
    QTcpServer *server = 0;
    QString mac;
    quint16 serverPort;
    QString serialN;
    QString serverName;
    QMdnsEngine::Server *mdnsServer = 0;
    QMdnsEngine::Provider *mdnsProvider = 0;
    QMdnsEngine::Hostname *mdnsHostname = 0;
    QHash<QTcpSocket *, DirconProcessorClient *> clientsMap;
    bool initServer();
    void initAdvertising();
    DirconPacket processPacket(DirconProcessorClient *client, const DirconPacket &pkt);
    QString convertUUIDFromUINT16ToString (quint16 uuid);

  public:
    ~DirconProcessor();
    explicit DirconProcessor(const QList<DirconProcessorService *> &services, const QString &serv_name,
                             quint16 serv_port, const QString &serv_sn, const QString &mac, QObject *parent = nullptr);
    bool sendCharacteristicNotification(quint16 uuid, const QByteArray &data);
    bool init();
  private slots:
    void tcpDataAvailable();
    void tcpDisconnected();
    void tcpNewConnection();
  signals:
    void onCharacteristicRead(quint16 uuid);
    void onCharacteristicWrite(quint16 uuid, QByteArray data);
    void onCharacteristicNotificationSwitch(quint16 uuid, char switchval);
};

#endif // DIRCONPROCESSOR_H
