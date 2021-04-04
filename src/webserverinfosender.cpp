#include "webserverinfosender.h"
#include <QWebSocket>


WebServerInfoSender::WebServerInfoSender(const QString& id, QObject * parent):TemplateInfoSender(id, parent) {

}
WebServerInfoSender::~WebServerInfoSender() {
    innerStop();
}

bool WebServerInfoSender::listen() {
    if (!innerTcpServer)
        innerTcpServer = new QTcpServer(this);
    if (!innerTcpServer->isListening()) {
        if (innerTcpServer->listen(QHostAddress::Any, port)) {
            httpServer->bind(innerTcpServer);
            return true;
        }
        else {
            delete innerTcpServer;
            innerTcpServer = 0;
        }
    }
    return false;
}

bool WebServerInfoSender::isRunning() const {
    return innerTcpServer && innerTcpServer->isListening();
}
bool WebServerInfoSender::send(const QString& data) {
    if (isRunning() && !data.isEmpty()) {
        bool rv, oldrv = false;
        for (QWebSocket * client: clients) {
            rv = client->sendTextMessage(data)  > 0;
            if (!oldrv)
                oldrv = rv;
        }
        return rv;
    }
    else
        return false;
}

void WebServerInfoSender::innerStop() {
    if (innerTcpServer) {
        if (isRunning())
            innerTcpServer->close();
        httpServer->deleteLater();
        innerTcpServer = 0;
        httpServer = 0;
    }
}

bool WebServerInfoSender::init() {
    bool ok;
    folders = settings.value("template_" + templateId + "_folders").toStringList();
    if (!folders.isEmpty()) {
        QString relative;
        int idx;
        port = settings.value("template_" + templateId + "_port", 6666).toInt(&ok);
        if (!ok)
            port = 6666;
        if (!httpServer)
            httpServer = new  QHttpServer(this);
        relative2Absolute.clear();
        for (auto fld : folders) {
            idx = fld.lastIndexOf('/');
            qDebug() << "Folder"<<fld;
            if (idx>0) {
                relative = fld.mid(idx + 1);
                qDebug() << "Relative"<<relative;
                relative2Absolute.insert(relative, fld);
                httpServer->route("/"+relative+"/<arg>", [this] (const QUrl &url, const QHttpServerRequest &request) {
                    QUrl urlreq = request.url();
                    QString path = urlreq.path().mid(1);
                    int idxreq = path.indexOf('/');
                    QString reqId = idxreq < 0? path:path.mid(0, idxreq);
                    qDebug() << "Path"<<path<<" req"<<reqId;
                    path = relative2Absolute.value(reqId);
                    if (path.isEmpty())
                        return QHttpServerResponse("text/plain",
                                                   "Unautorized",
                                                   QHttpServerResponder::StatusCode::Forbidden);
                    else
                        return QHttpServerResponse::fromFile(path + QStringLiteral("/%1").arg(url.path()));
                });
            }
        }
        if (listen()) {
            qDebug() << "WebServer listening on port" << port<< " "<<relative2Absolute;
            connect(httpServer, SIGNAL(newWebSocketConnection()), this, SLOT(onNewConnection()));
            return true;
        }
        else {
            reinit();
        }
    }
    return false;

}

void WebServerInfoSender::processTextMessage(QString message)
{
    /*QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendTextMessage(message);
    }*/
    qDebug() << "Message received:" << message;
    emit onDataReceived(message.toUtf8());
}

void WebServerInfoSender::onNewConnection()
{
    QWebSocket *pSocket = httpServer->nextPendingWebSocketConnection();
    qDebug() << "WebSocket connection"<<pSocket->requestUrl();
    connect(pSocket, SIGNAL(textMessageReceived), this, SLOT(processTextMessage));
    connect(pSocket, SIGNAL(binaryMessageReceived), this, SLOT(processBinaryMessage));
    connect(pSocket, SIGNAL(disconnected), this, SLOT(socketDisconnected));

    clients << pSocket;
}

void WebServerInfoSender::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void WebServerInfoSender::processBinaryMessage(QByteArray message)
{
    /*QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }*/
    qDebug() << "Binary Message received:" << message.toHex();
    emit onDataReceived(message);
}

