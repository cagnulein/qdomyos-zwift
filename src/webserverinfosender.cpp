#include "webserverinfosender.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QtWebSockets/QWebSocket>

WebServerInfoSender::WebServerInfoSender(const QString &id, QObject *parent) : TemplateInfoSender(id, parent) {
    fetcher = new QNetworkAccessManager(this);
    fetcher->setCookieJar(new QNoCookieJar());
    connect(fetcher, SIGNAL(finished(QNetworkReply *)), this, SLOT(handleFetcherRequest(QNetworkReply *)));
    connect(fetcher, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), this,
            SLOT(ignoreSSLErrors(QNetworkReply *, const QList<QSslError> &)));
}
WebServerInfoSender::~WebServerInfoSender() { innerStop(); }

void WebServerInfoSender::ignoreSSLErrors(QNetworkReply *repl, const QList<QSslError> &) { repl->ignoreSslErrors(); }

bool WebServerInfoSender::listen() {
    if (!innerTcpServer) {
        innerTcpServer = new QTcpServer(this);
        connect(innerTcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));
    }
    if (!innerTcpServer->isListening()) {
        if (innerTcpServer->listen(QHostAddress::Any, port)) {
            if (!port) {
                settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_port"),
                                  port = innerTcpServer->serverPort());
            }
            httpServer->bind(innerTcpServer);

            connect(&watchdogTimer, SIGNAL(timeout()), this, SLOT(watchdogEvent()));
            watchdogTimer.start(5000);

            return true;
        } else {
            delete innerTcpServer;
            innerTcpServer = 0;
        }
    }
    return false;
}

void WebServerInfoSender::acceptError(QAbstractSocket::SocketError socketError) {qDebug() << "WebServerInfoSender::acceptError" << socketError;}
bool WebServerInfoSender::isRunning() const { return innerTcpServer && innerTcpServer->isListening(); }
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
    folders = settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_folders")).toStringList();
    if (!folders.isEmpty()) {
        QString relative;
        int idx;
        port = settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_port"), 6666).toInt(&ok);
        if (!ok)
            port = 6666;
        if (!httpServer) {
            httpServer = new QHttpServer(this);

            // certificates to remove, just put this to test this
            // https://travistidwell.com/jsencrypt/demo/
            QSslCertificate certificate("MIIC8DCCAdigAwIBAgIUDexmFCL5/l7ELI710o4vdRqTAjEwDQYJKoZIhvcNAQELBQAwFDESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTIzMDMwMzE1NDgxOVoXDTIzMDQwMjE1NDgxOVowFDESMBAGA1UEAwwJbG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1F83nkl+Ais8BBg5MjQcufoZ1+sdWJ/+YWzAexFcVfN1pNzmTQfJrpJdIcTc0XlflqdW0sO42BJ7i/sh4a3oc8BbCdxLc6CMXbdn8YmD8Oqj97fZ7m21xJsweZhhlNGrawLMfir6cQKoZ+wQrXlnVYsFnY44eb4bSjewTHixPF1K+yTBQGnDQhpxjgVsvuW3iDMmA1V5pT6ruFGMAIm4bYfXgFW79e/dv0vtti2RPhhX7nZ9K+tQe5QtRJXFRdIhGpOeAKxFXS+/sMAxzcib9viROrBIoA1f2Qgyp81/s0uMcHcx3lzzWdQRmZcforDNxcHNhSkL90alzYMJWvJPkQIDAQABozowODAUBgNVHREEDTALgglsb2NhbGhvc3QwCwYDVR0PBAQDAgeAMBMGA1UdJQQMMAoGCCsGAQUFBwMBMA0GCSqGSIb3DQEBCwUAA4IBAQAe/txddQtVf9CTykaCujuT+3DdbqaM/+XxKrBlxZrGLPCpWZkbRcJ1w+ci/XfMQEAneeToVltuK46BA8pIyfjCfceGYpzVC6QBKDX0Te9XN+xIJKAtFZWvVCihZENdQuv49TE09JSq8pFLfPF4okwYTY3smJf+IhFCR3bi3tVB/FkM3ZBafahDMiAwbcNbFC0JW6Meynw74iaFRAjgmMbcqHYMIlJAPZrSBtBA+BLb1y9EaDZSYLZ6qCSw6dEtntdypMpu6pJS4TaPzRPvp2BG9x6i49H3zfZcl10mVcjnF02KECzWvjP5RG/oPC9119jk7RCSaEuyca+Sxnht9hpf");
            QSslKey privatekey("MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDUXzeeSX4CKzwEGDkyNBy5+hnX6x1Yn/5hbMB7EVxV83Wk3OZNB8mukl0hxNzReV+Wp1bSw7jYEnuL+yHhrehzwFsJ3EtzoIxdt2fxiYPw6qP3t9nubbXEmzB5mGGU0atrAsx+KvpxAqhn7BCteWdViwWdjjh5vhtKN7BMeLE8XUr7JMFAacNCGnGOBWy+5beIMyYDVXmlPqu4UYwAibhth9eAVbv1792/S+22LZE+GFfudn0r61B7lC1ElcVF0iEak54ArEVdL7+wwDHNyJv2+JE6sEigDV/ZCDKnzX+zS4xwdzHeXPNZ1BGZlx+isM3Fwc2FKQv3RqXNgwla8k+RAgMBAAECggEAA0/Q0DkZK/N8phuymKPTZtsNmB5kgMNN9jr57XFh3T1EsmN3e/AJJx3FMged4e5gnTrS0cqXkVcIcjBiYOCTZWq0gLzcuFrilXuMtWmNky41jGFjlqJDWWOKJ/tGWknCwJJ0mesVqLl/4s79m5bvE5htZ/2Vx+A9ELU5nJetO+G5SegH+ydeYwj6SPcerb3dg/KF0DzFRBuU2ciZYcDVZfProLDgXli9xoBXarzLc/ndjwQX45Jh5GBvpcQPIlIAhH3lz0zTWOUTaVc52nmnPG/wkNrHRP74XXdhZoqT5AmBUYDoGQUhV7WvXQ8DRM+Oue9jClxy58P5pzj2fqbEAQKBgQDp5g+0cZBanQsSRex0EapnWKeIu83j27G8OVO9iS07v58H7HoAD5kXJYu5sPVE+Py0aWbtLY+/FrxeZG87psM2AKsSIM2q061xJVhUjaV+A5veubFQ2I5fy3Ln8w5cu66e5Xr6hn9qB17nrVlyKAhtNkhrm+bZ53yxa0emIdh+cQKBgQDocG5AnOoOXhSNYsxpLlkOmr7GK1Qb1FwZvL/k7AkJQd5lN41Q4o3aVBXkGDQ+DKsI/2L/y7DeWr3AS+80XCTAZPN+DEwn4hODiz78YtQI4wlImTUK33unRUFpSQqtFVJxjG6KBeX2UzPEifRz6Sq/X1oT0ldrntkhK/jUCjGzIQKBgQCse+yu62RBfjfw5MGnInPgPF9nlN8THirmm/vl9Kf3vKqBBGE/dEE38Ycli5qDn31zaZruYr/zcce9cCEbAzJHu5xsBObGB82Kd7i4ubAFypGCYLui29+6QuTcqb+4oOr34FCdONvzC7Zv8MTaSy1TpEkpmdFWdb/dcjhnCeSF8QKBgQCLwAVZzb4fs0ryEuPJnXcoA7wN08E3Fj/lrYlGbu+j5Dl9a6AIcJ5PFV0wDaljYSR4PWxdVS9bEP2jH0SLm5bxIgEP2P70v8Vxwoe1IQpQ6YgMYSj2B5YF5OrGDYdgt0AhSwiu7YrsxeuLEFKsWhU8iGzVHBM5foEXo6NwgUyOYQKBgQDARvLvsfoZYgH43Qyf198GhnhGnsVtxFTxGQEz4QUwuqzA3j1hPmCK8FFKFYV07ArO9xw/cf+Da4X7KVLJhnYVWVzrJ29nelWfD1Hd1IMdhaqppV/Xvw1H5Hv4umN+qoaw+n8s5416qd62bqzHymXxEAEQVCpTSF21wmxH5/p8Bg==", QSsl::Rsa, QSsl::Pem);
            httpServer->sslSetup(certificate, privatekey);
        }
        relative2Absolute.clear();
        for (auto fld : folders) {
            idx = fld.lastIndexOf('/');
            qDebug() << QStringLiteral("Folder") << fld;
            if (idx > 0) {
                relative = fld.mid(idx + 1);
                qDebug() << QStringLiteral("Relative") << relative;
                relative2Absolute.insert(relative, fld);
                httpServer->route(QStringLiteral("/") + relative + QStringLiteral("/<arg>"),
                                  [this](const QUrl &url, const QHttpServerRequest &request) {
                                      QUrl urlreq = request.url();
                                      QString path = urlreq.path().mid(1);
                                      int idxreq = path.indexOf('/');
                                      QString reqId = idxreq < 0 ? path : path.mid(0, idxreq);
                                      qDebug() << QStringLiteral("Path") << path << QStringLiteral("req") << reqId;
                                      path = relative2Absolute.value(reqId);
                                      if (path.isEmpty())
                                          return QHttpServerResponse("text/plain", "Unautorized",
                                                                     QHttpServerResponder::StatusCode::Forbidden);
                                      else {
                                          path += QStringLiteral("/%1").arg(url.path());
                                          qDebug() << "File to look at:" << path;
                                          return QHttpServerResponse::fromFile(path);
                                      }
                                  });
            }
        }
        if (listen()) {
            qDebug() << QStringLiteral("WebServer listening on port") << port << QStringLiteral(" ")
                     << relative2Absolute;
            connect(httpServer, SIGNAL(newWebSocketConnection()), this, SLOT(onNewConnection()));
            return true;
        } else {
            reinit();
        }
    }
    return false;
}

void WebServerInfoSender::watchdogEvent() {
    if(innerTcpServer->serverError() != QAbstractSocket::UnknownSocketError)
        qDebug() << "WebServerInfoSender is " << innerTcpServer->serverError();
    if(innerTcpServer && !innerTcpServer->isListening()) {
        qDebug() << QStringLiteral("innerTcpServer is not LISTENING!");
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
                arrv.append(p.first.constData());
                arrv.append(line.constData());
                headers.append(arrv);
            }
        }
        QString respType = reqIdRequester.first.operator[](QStringLiteral("responseType")).toString();
        init[QStringLiteral("headers")] = headers;
        init[QStringLiteral("status")] = statusCode;
        init[QStringLiteral("statusText")] = statusText;
        init[QStringLiteral("responseURL")] = reply->url().toString();
        if (respType == QStringLiteral("arraybuffer") || respType == QStringLiteral("blob"))
            out[QStringLiteral("body")] = QJsonValue(body.toBase64().constData());
        else
            out[QStringLiteral("body")] = QJsonValue(body.constData());
        out[QStringLiteral("init")] = init;
        out[QStringLiteral("req")] = req;
        out[QStringLiteral("DBG")] = error;
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

void WebServerInfoSender::onNewConnection() {
    QWebSocket *pSocket = httpServer->nextPendingWebSocketConnection();
    QUrl requestUrl = pSocket->requestUrl();
    qDebug() << QStringLiteral("WebSocket connection") << requestUrl;
    if (requestUrl.path() == QStringLiteral("/fetcher")) {
        connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processFetcherRequest(QString)));
        connect(pSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processFetcherRawRequest(QByteArray)));
    } else {
        connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
        connect(pSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processBinaryMessage(QByteArray)));
        sendToClients << pSocket;
    }
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    clients << pSocket;
}

void WebServerInfoSender::socketDisconnected() {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << QStringLiteral("socketDisconnected:") << pClient;
    if (pClient) {
        clients.removeAll(pClient);
        if (!sendToClients.removeAll(pClient)) {
            QMutableHashIterator<QNetworkReply *, QPair<QJsonObject, QWebSocket *>> i(reply2Req);
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

void WebServerInfoSender::processBinaryMessage(QByteArray message) {
    /*QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }*/
    //qDebug() << QStringLiteral("Binary Message received:") << message.toHex();
    emit onDataReceived(message);
}
