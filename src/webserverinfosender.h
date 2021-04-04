#ifndef WEBSERVERINFOSENDER_H
#define WEBSERVERINFOSENDER_H
#include "templateinfosender.h"
#include <QHttpServer>

class WebServerInfoSender : public TemplateInfoSender
{
    Q_OBJECT
public:
    WebServerInfoSender(const QString& id, QObject * parent = 0);
    virtual ~WebServerInfoSender();
    virtual bool isRunning() const;
    virtual bool send(const QString& data);
private:
    QHttpServer * httpServer = 0;
    QStringList folders;
    bool listen();
protected:
    virtual void innerStop();
    int port;
    QTcpServer * innerTcpServer = 0;
    virtual bool init();
    QList<QWebSocket *> clients;
    QHash<QString, QString> relative2Absolute;
private slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
};

#endif // WEBSERVERINFOSENDER_H
