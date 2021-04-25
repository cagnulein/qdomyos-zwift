#include "webserverinfosender.h"
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>


WebServerInfoSender::WebServerInfoSender(const QString& id, QObject * parent):TemplateInfoSender(id, parent) {
    fetcher = new QNetworkAccessManager(this);
    fetcher->setCookieJar(new QNoCookieJar());
    connect(fetcher, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleFetcherRequest(QNetworkReply*)));
    connect(fetcher, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), this, SLOT(ignoreSSLErrors(QNetworkReply *, const QList<QSslError> &)));
}
WebServerInfoSender::~WebServerInfoSender() {
    innerStop();
}

void WebServerInfoSender::ignoreSSLErrors(QNetworkReply * repl, const QList<QSslError> &) {
    repl->ignoreSslErrors();
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
        bool rv = true, oldrv = false;
        for (QWebSocket * client: sendToClients) {
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
        clients.clear();
        sendToClients.clear();
        reply2Req.clear();
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

void WebServerInfoSender::handleFetcherRequest(QNetworkReply* reply) {
    QPair<QString, QWebSocket *> reqIdRequester = reply2Req.value(reply);
    QString req = reqIdRequester.first;
    QWebSocket * requester = reqIdRequester.second;
    if (!req.isEmpty() && requester) {
        QNetworkReply::NetworkError error = reply->error();
        QString statusText = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray body = reply->readAll();
        QJsonObject out, init;
        QList<QNetworkReply::RawHeaderPair> rHeaders = reply->rawHeaderPairs();
        QJsonArray headers;
        for (auto p: rHeaders) {
            for (auto line: p.second.split('\n')) {
                QJsonArray arrv;
                arrv.append(p.first.constData());
                arrv.append(line.constData());
                headers.append(arrv);
            }
        }
        init["headers"] = headers;
        init["status"] = statusCode;
        init["statusText"] = statusText;
        out["body"] = QJsonValue(body.constData());
        out["init"] = init;
        out["req"] = req;
        out["DBG"] = error;
        QJsonDocument toSend(out);
        requester->sendTextMessage(toSend.toJson());
        reply2Req.remove(reply);
    }
    reply->deleteLater();
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

void WebServerInfoSender::processFetcherRequest(QString data) {
    processFetcher(qobject_cast<QWebSocket *>(sender()), data.toUtf8());
}

void WebServerInfoSender::processFetcherRawRequest(QByteArray data) {
    processFetcher(qobject_cast<QWebSocket *>(sender()), data);
}

void WebServerInfoSender::processFetcher(QWebSocket * sender, const QByteArray& data) {
    qDebug() << "Fetch Request Received" << data;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    if (jsonResponse.isObject()) {
        QJsonObject jsonObject = jsonResponse.object();
        if (jsonObject.contains("req") && jsonObject.contains("url")) {
            QString req = jsonObject["req"].toString();
            QString url = jsonObject["url"].toString();
            QNetworkRequest request(url);
            QString method = "GET";
            QJsonValue tmpv;
            request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
            if ((tmpv = jsonObject.value("method")).isString())
                method = tmpv.toString();
            if ((tmpv = jsonObject.value("headers")).isObject()) {
                QVariantHash headers = tmpv.toObject().toVariantHash();
                QVariantHash::const_iterator i = headers.constBegin();
                while (i != headers.constEnd()) {
                    request.setRawHeader(i.key().toUtf8(), i.value().toString().toUtf8());
                    ++i;
                }
            }
            QNetworkReply * repl;
            if (method.toLower() == "post") {
                QByteArray body;
                if ((tmpv = jsonObject.value("body")).isString())
                    body = tmpv.toString().toUtf8();
                repl = fetcher->post(request, body);
            }
            else {
                repl = fetcher->get(request);
            }
            reply2Req[repl] = QPair<QString, QWebSocket *>(req, sender);
        }
    }
}

void WebServerInfoSender::onNewConnection()
{
    QWebSocket *pSocket = httpServer->nextPendingWebSocketConnection();
    QUrl requestUrl = pSocket->requestUrl();
    qDebug() << "WebSocket connection"<<requestUrl;
    if (requestUrl.path() == "/fetcher") {
        connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processFetcherRequest(QString)));
        connect(pSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processFetcherRawRequest(QByteArray)));
    }
    else {
        connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
        connect(pSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processBinaryMessage(QByteArray)));
        sendToClients << pSocket;
    }
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    clients << pSocket;
}

void WebServerInfoSender::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        clients.removeAll(pClient);
        if (!sendToClients.removeAll(pClient)) {
            QMutableHashIterator<QNetworkReply *, QPair<QString, QWebSocket *>> i(reply2Req);
            while (i.hasNext()) {
                i.next();
                if (i.value().second == pClient) {
                    i.remove();
                    break;
                }
            }
        }
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

