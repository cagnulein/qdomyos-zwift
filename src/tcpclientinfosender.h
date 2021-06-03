#ifndef TCPCLIENTINFOSENDER_H
#define TCPCLIENTINFOSENDER_H

#include "templateinfosender.h"
#include <QTcpSocket>

class TcpClientInfoSender : public TemplateInfoSender {
    Q_OBJECT
  public:
    TcpClientInfoSender(const QString &id, QObject *parent = 0);
    virtual ~TcpClientInfoSender();
    virtual bool isRunning() const;
    virtual bool send(const QString &data);

  protected:
    QTcpSocket *tcpSocket = 0;
    QString ip;
    int port;
    virtual bool init();
    virtual void innerStop();
  private slots:
    void readyRead();
    void debugConnected();
    void socketError(int err);
    void stateChanged(QAbstractSocket::SocketState socketState);
};

#endif // TCPCLIENTINFOSENDER_H
