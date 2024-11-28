#ifndef OSC_H
#define OSC_H

#include <QObject>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QHash>
#include <QVariant>
#include "bluetoothdevice.h"
#include "devices/bike.h"
#include "devices/treadmill.h"
#include "devices/rower.h"
#include "homeform.h"
#include "bluetooth.h"
#include <oscpp/client.hpp>
#include <oscpp/server.hpp>
#include <oscpp/print.hpp>
#include <iostream>

class OSC : public QObject
{
    Q_OBJECT
  public:
    explicit OSC(bluetooth* manager, QObject *parent = nullptr);

  private:
    QTimer* m_timer;
    bluetooth* bluetoothManager;

    size_t OSC_makePacket(void* buffer, size_t size);
    void OSC_handlePacket(const OSCPP::Server::Packet& packet);
    QUdpSocket* OSC_sendSocket = new QUdpSocket(this);
    QUdpSocket* OSC_recvSocket = new QUdpSocket(this);

  private slots:
    void publishWorkoutData();

  signals:

};

#endif // OSC_H
