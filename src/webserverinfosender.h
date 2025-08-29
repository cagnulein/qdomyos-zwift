#ifndef WEBSERVERINFOSENDER_H
#define WEBSERVERINFOSENDER_H
#include "templateinfosender.h"
#include <QHttpServer>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QtWebSockets/QWebSocketServer>
#include <QTcpServer>
#include <QMutex>
#include <QPointer>

class QNoCookieJar : public QNetworkCookieJar {
    Q_OBJECT
  public:
    QNoCookieJar(QObject *parent = nullptr) : QNetworkCookieJar(parent) {}
    virtual ~QNoCookieJar() {}
    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const { return QList<QNetworkCookie>(); }
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url) { return false; }
};

class WebServerInfoSender : public TemplateInfoSender {
    Q_OBJECT
  public:
    WebServerInfoSender(const QString &id, QObject *parent = 0);
    virtual ~WebServerInfoSender();
    virtual bool isRunning() const;
    virtual bool send(const QString &data);
  private:
    QHttpServer *httpServer = nullptr;
    QWebSocketServer *webSocketServer = nullptr;
    QTcpServer *tcpServer = nullptr;  // Used for HTTP server binding
    QStringList folders;
    bool listenHttp();
    bool listenWebSocket();
    void processFetcher(QWebSocket *sender, const QByteArray &data);
    QTimer watchdogTimer;
  protected:
    virtual void innerStop();
    int port = 0;
    int wsPort = 0;
    virtual bool init();
    QList<QPointer<QWebSocket>> clients;
    QNetworkAccessManager *fetcher = nullptr;
    QList<QPointer<QWebSocket>> sendToClients;
    QHash<QString, QString> relative2Absolute;
    QHash<QNetworkReply *, QPair<QJsonObject, QWebSocket *>> reply2Req;
    mutable QMutex clientsMutex;
  private slots:
    void watchdogEvent();
    void onNewWebSocketConnection();
    void handleFetcherRequest(QNetworkReply *reply);
    void processTextMessage(QString message);
    void processFetcherRawRequest(QByteArray message);
    void processFetcherRequest(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
    void ignoreSSLErrors(QNetworkReply *, const QList<QSslError> &);
};
#endif // WEBSERVERINFOSENDER_H
