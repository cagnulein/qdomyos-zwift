#include "webserverinfosender.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QtWebSockets/QWebSocket>

WebServerInfoSender::WebServerInfoSender(const QString &id, QObject *parent) : TemplateInfoSender(id, parent) {
    fetcher = new QNetworkAccessManager(this);
    fetcher->setCookieJar(new QNoCookieJar());
    connect(fetcher, &QNetworkAccessManager::finished, this, &WebServerInfoSender::handleFetcherRequest);
    connect(fetcher, &QNetworkAccessManager::sslErrors, this, &WebServerInfoSender::ignoreSSLErrors);
}

WebServerInfoSender::~WebServerInfoSender() {
    innerStop();
}

void WebServerInfoSender::ignoreSSLErrors(QNetworkReply *repl, const QList<QSslError> &) {
    repl->ignoreSslErrors();
}

bool WebServerInfoSender::listenHttp() {
    // In Qt6, QHttpServer doesn't provide direct listen/isListening methods
    // We need to use a QTcpServer and bind the HttpServer to it
    if (!tcpServer) {
        tcpServer = new QTcpServer(this);
    }

    if (!tcpServer->isListening()) {
        bool success = tcpServer->listen(QHostAddress::Any, port);
        if (success) {
            if (!port) {
                // Save the autoselected port
                port = tcpServer->serverPort();
                settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_port"), port);
            }

            // Bind HTTP server to the TCP server
            httpServer->bind(tcpServer);

            qDebug() << QStringLiteral("HTTP Server listening on port") << port;
            return true;
        } else {
            qDebug() << "Failed to start TCP server for HTTP:" << tcpServer->errorString();
            return false;
        }
    }

    return tcpServer->isListening();
}

bool WebServerInfoSender::listenWebSocket() {
    if (!webSocketServer) {
        webSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);

        connect(webSocketServer, &QWebSocketServer::newConnection, this, &WebServerInfoSender::onNewWebSocketConnection);
        connect(webSocketServer, &QWebSocketServer::serverError, [this](QWebSocketProtocol::CloseCode closeCode) {
            qDebug() << "WebSocketServer error: " << closeCode;
        });
    }

    // Use the port number right after HTTP port for WebSockets
    wsPort = port + 1;

    if (webSocketServer->listen(QHostAddress::Any, wsPort)) {
        settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_wsport"), wsPort);
        qDebug() << QStringLiteral("WebSocket Server listening on port") << wsPort;

        connect(&watchdogTimer, &QTimer::timeout, this, &WebServerInfoSender::watchdogEvent);
        watchdogTimer.start(5000);

        return true;
    } else {
        qDebug() << "Failed to start WebSocket server:" << webSocketServer->errorString();
        return false;
    }
}

bool WebServerInfoSender::isRunning() const {
    return tcpServer && tcpServer->isListening() &&
           webSocketServer && webSocketServer->isListening();
}

bool WebServerInfoSender::send(const QString &data) {
    if (isRunning() && !data.isEmpty()) {
        bool rv = true, oldrv = false;
        for (QWebSocket *client : sendToClients) {
            rv = client->sendTextMessage(data) > 0;
            if (!oldrv)
                oldrv = rv;
        }
        return rv;
    } else
        return false;
}

void WebServerInfoSender::innerStop() {
    // Stop watchdog timer
    watchdogTimer.stop();

    // Close WebSocket server
    if (webSocketServer) {
        if (webSocketServer->isListening())
            webSocketServer->close();
        webSocketServer->deleteLater();
        webSocketServer = nullptr;
    }

    // Close TCP server (which HTTP server is bound to)
    if (tcpServer) {
        if (tcpServer->isListening())
            tcpServer->close();
        tcpServer->deleteLater();
        tcpServer = nullptr;
    }

    // Delete HTTP server
    if (httpServer) {
        httpServer->deleteLater();
        httpServer = nullptr;
    }

    // Clear all collections
    clients.clear();
    sendToClients.clear();
    reply2Req.clear();
}

bool WebServerInfoSender::init() {
    bool ok;
    folders = settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_folders")).toStringList();
    if (!folders.isEmpty()) {
        QString relative;
        int idx;
        port = settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_port"), 6666).toInt(&ok);
        if (!ok)
            port = 6666;

        // Create HTTP server if not exists
        if (!httpServer)
            httpServer = new QHttpServer(this);

        relative2Absolute.clear();
        for (auto fld : folders) {
            idx = fld.lastIndexOf('/');
            qDebug() << QStringLiteral("Folder") << fld;
            if (idx > 0) {
                relative = fld.mid(idx + 1);
                qDebug() << QStringLiteral("Relative") << relative;
                relative2Absolute.insert(relative, fld);

                // Set up route for file serving
                httpServer->route(QStringLiteral("/") + relative + QStringLiteral("/<arg>"),
                                  [this](const QUrl &url, const QHttpServerRequest &request) {
                                      QUrl urlreq = request.url();
                                      QString path = urlreq.path().mid(1);
                                      int idxreq = path.indexOf('/');
                                      QString reqId = idxreq < 0 ? path : path.mid(0, idxreq);
                                      qDebug() << QStringLiteral("Path") << path << QStringLiteral("req") << reqId;
                                      path = relative2Absolute.value(reqId);
                                      if (path.isEmpty())
                                          return QHttpServerResponse("text/plain", "Unauthorized",
                                                                     QHttpServerResponder::StatusCode::Forbidden);
                                      else {
                                          path += QStringLiteral("/%1").arg(url.path());
                                          qDebug() << "File to look at:" << path;
                                          return QHttpServerResponse::fromFile(path);
                                      }
                                  });
            }
        }

        // Start the HTTP server
        if (listenHttp()) {
            qDebug() << QStringLiteral("HTTP Server listening on port") << port;

            // Start the WebSocket server on the next port
            if (listenWebSocket()) {
                qDebug() << QStringLiteral("WebSocket Server listening on port") << wsPort;
                return true;
            }
        }

        // If we reach here, something failed
        reinit();
    }
    return false;
}

void WebServerInfoSender::watchdogEvent() {
    // Check TCP server (which HTTP server is bound to)
    if (tcpServer) {
        if (tcpServer->serverError() != QAbstractSocket::UnknownSocketError) {
            qDebug() << "TCP Server error: " << tcpServer->serverError();
        }

        if (!tcpServer->isListening()) {
            qDebug() << QStringLiteral("TCP Server is not LISTENING!");
        }
    }

    // Check WebSocket server
    if (webSocketServer) {
        // Just check if there's an error string
        if (!webSocketServer->errorString().isEmpty()) {
            qDebug() << "WebSocketServer error: " << webSocketServer->errorString();
        }

        if (!webSocketServer->isListening()) {
            qDebug() << QStringLiteral("WebSocket Server is not LISTENING!");
        }
    }
}

void WebServerInfoSender::handleFetcherRequest(QNetworkReply *reply) {
    QPair<QJsonObject, QWebSocket *> reqIdRequester = reply2Req.value(reply);
    QString req = reqIdRequester.first.operator[](QStringLiteral("req")).toString();
    QWebSocket *requester = reqIdRequester.second;
    if (!req.isEmpty() && requester) {
        QNetworkReply::NetworkError error = reply->error();
        QString statusText = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray body = reply->readAll();
        QJsonObject out, init;
        QList<QNetworkReply::RawHeaderPair> rHeaders = reply->rawHeaderPairs();
        QJsonArray headers;
        for (auto p : rHeaders) {
            for (auto line : p.second.split('\n')) {
                QJsonArray arrv;
                arrv.append(QString::fromUtf8(p.first));
                arrv.append(QString::fromUtf8(line));
                headers.append(arrv);
            }
        }
        QString respType = reqIdRequester.first.operator[](QStringLiteral("responseType")).toString();
        init[QStringLiteral("headers")] = headers;
        init[QStringLiteral("status")] = statusCode;
        init[QStringLiteral("statusText")] = statusText;
        init[QStringLiteral("responseURL")] = reply->url().toString();
        if (respType == QStringLiteral("arraybuffer") || respType == QStringLiteral("blob"))
            out[QStringLiteral("body")] = QJsonValue(QString::fromUtf8(body.toBase64()));
        else
            out[QStringLiteral("body")] = QJsonValue(QString::fromUtf8(body));
        out[QStringLiteral("init")] = init;
        out[QStringLiteral("req")] = req;
        out[QStringLiteral("DBG")] = static_cast<int>(error);
        QJsonDocument toSend(out);
        requester->sendTextMessage(toSend.toJson());
        reply2Req.remove(reply);
    }
    reply->deleteLater();
}

void WebServerInfoSender::processTextMessage(QString message) {
    /*QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendTextMessage(message);
    }*/
    //qDebug() << QStringLiteral("Message received:") << message;
    emit onDataReceived(message.toUtf8());
}

void WebServerInfoSender::processFetcherRequest(QString data) {
    processFetcher(qobject_cast<QWebSocket *>(sender()), data.toUtf8());
}

void WebServerInfoSender::processFetcherRawRequest(QByteArray data) {
    processFetcher(qobject_cast<QWebSocket *>(sender()), data);
}

void WebServerInfoSender::processFetcher(QWebSocket *sender, const QByteArray &data) {
    qDebug() << QStringLiteral("Fetch Request Received") << data;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    if (jsonResponse.isObject()) {
        QJsonObject jsonObject = jsonResponse.object();
        if (jsonObject.contains(QStringLiteral("req")) && jsonObject.contains(QStringLiteral("url"))) {
            QString req = jsonObject[QStringLiteral("req")].toString();
            QString url = jsonObject[QStringLiteral("url")].toString();
            QNetworkRequest request(url);
            QString method = QStringLiteral("GET");
            QJsonValue tmpv;
            request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
            if ((tmpv = jsonObject.value(QStringLiteral("method"))).isString())
                method = tmpv.toString();
            if ((tmpv = jsonObject.value(QStringLiteral("headers"))).isObject()) {
                QVariantHash headers = tmpv.toObject().toVariantHash();
                QVariantHash::const_iterator i = headers.constBegin();
                while (i != headers.constEnd()) {
                    request.setRawHeader(i.key().toUtf8(), i.value().toString().toUtf8());
                    ++i;
                }
            }
            QNetworkReply *repl;
            if (method.toLower() == QStringLiteral("post")) {
                QByteArray body;
                if ((tmpv = jsonObject.value(QStringLiteral("body"))).isString())
                    body = tmpv.toString().toUtf8();
                repl = fetcher->post(request, body);
            } else {
                repl = fetcher->get(request);
            }
            reply2Req[repl] = QPair<QJsonObject, QWebSocket *>(jsonObject, sender);
        }
    }
}

void WebServerInfoSender::onNewWebSocketConnection() {
    // Get the next pending connection from the WebSocket server
    QWebSocket *pSocket = webSocketServer->nextPendingConnection();
    if (!pSocket) {
        qDebug() << "No pending WebSocket connection available";
        return;
    }

    QUrl requestUrl = pSocket->requestUrl();
    qDebug() << QStringLiteral("WebSocket connection") << requestUrl;

           // Handle different types of WebSocket connections based on the path
    if (requestUrl.path() == QStringLiteral("/fetcher")) {
        connect(pSocket, &QWebSocket::textMessageReceived, this, &WebServerInfoSender::processFetcherRequest);
        connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebServerInfoSender::processFetcherRawRequest);
    } else {
        connect(pSocket, &QWebSocket::textMessageReceived, this, &WebServerInfoSender::processTextMessage);
        connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebServerInfoSender::processBinaryMessage);
        sendToClients << pSocket;
    }

    connect(pSocket, &QWebSocket::disconnected, this, &WebServerInfoSender::socketDisconnected);

           // Store the WebSocket connection
    QSharedPointer<QWebSocket> sharedSocket(pSocket);
    clients << sharedSocket;
}

void WebServerInfoSender::socketDisconnected() {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << QStringLiteral("socketDisconnected:") << pClient;
    if (pClient) {
        // Remove client from all collections
        for (int i = 0; i < clients.size(); ++i) {
            if (clients[i].data() == pClient) {
                clients.removeAt(i);
                break;
            }
        }

        if (!sendToClients.removeAll(pClient)) {
            // Check in the reply2Req map
            QMutableHashIterator<QNetworkReply *, QPair<QJsonObject, QWebSocket *>> i(reply2Req);
            while (i.hasNext()) {
                i.next();
                if (i.value().second == pClient) {
                    i.remove();
                    break;
                }
            }
        }

        // No need to delete - will be handled by QSharedPointer
    }
}

void WebServerInfoSender::processBinaryMessage(QByteArray message) {
    /*QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }*/
    //qDebug() << QStringLiteral("Binary Message received:") << message.toHex();
    emit onDataReceived(message);
}
