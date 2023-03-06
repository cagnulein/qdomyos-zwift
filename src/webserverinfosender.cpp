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
        innerTcpServer = new QSslServer(sslconf, this);
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

            static const char g_privateKey[] = R"(-----BEGIN RSA PRIVATE KEY-----
                                               MIIEowIBAAKCAQEAvZXqT9ygZNP+v5bjIpfBmaJD/oxXC0/ZkoS3hcoDJ2G/2kaY
                                               +BASMVjo9c/4cB0GvjTnK19+H1HxGbLBC6Mc91ULYJQDzpDIROaa8kxTlL+HtN+t
                                               3wG/EnKdhlvqYTIGNl4bjHES0DvgBL04jWpyGhP2hKHvVrYDIcl4PwUns9WTTZ1p
                                               oL6GwO/8urZvW4ulCsloF/vU0o1sg8AnLH7cHNVQYf5hlFxdQidGU8LgxxTu/zdi
                                               nLQOQ3ntZBKh2czKl4FrIsUBP9OOAm5q82f456cSOiVUS5CUEzLijzWcwXJnRXbK
                                               7kLnCR05dhnrCVMo6+eaKReYIHJtUdJ6JxqdCQIDAQABAoIBAQCbeULzhVof9d6X
                                               XfUdHFjPVkvuh0am5y/lQWODm1wkwrvkFDtES2GwCFrbo1b5EkTCXX4qiqBSrQ9M
                                               5NP9aCHMD3lKFXhdwUwLfRYamE9+z+dJhJoBJVx9k3LtiZlXZf/EPD7ZWK12CXQ4
                                               Trj++Alkq3Rh5q7xbEh7P86V1/NsPNO+u+NAQQIiHrFuRN5LD2JpMjlzUzKiGPHH
                                               w2ETmFN6Q3+0zSgRgOlXT/ngkNfeCa47CM+rhk5KbrCbbBX373s5zcyAYdWhLeiC
                                               wPWeJpiPwTcjFswwn/KUO/Q/ahM6vd2Pky5MbwzdULYlELfKaV7tfJEE6LxBsCgZ
                                               OEDuhbABAoGBAN0RRMLYJvlUI0H8gHMtRnYZ/IJA8Txu9BTl7ioFoLBqTsf1wBOX
                                               YtKbYcNQn9hSZfVg8dCMJIs7GIFIj5OHMxmGlceK8oUZMeVatonM03cyEmklqKgJ
                                               IDqkcIcFY6ZmoB48AEVoJi2DKukrSTac7FhCpIGS+T0fhJwP/oEzO/cJAoGBANuL
                                               II4SPDEYB37g/ZcplHq8NhTLGMynlPXtwFeD5/uMO+cXNfXHi7FRfTR5NY9YrE5v
                                               ChHVAsEgo8nKS6XtQDpTrBw+shv1E7YORLn/E9zVQxTBmPcVSZqu7oUdaE0bwXly
                                               yLinfcT3sJfWSvy5VPOLA2FwE2dty6Ro1RwznPYBAoGAJzCCghATiKuzvGTo2II0
                                               NYeRzf15Rn9c4b+JIwXwYJXX9XbJYC+8Ix3MjPVkANNKo99VvuS5Lb2CMJkOatbS
                                               5jmHn+Iv8AxxXflg1rjq2/ySavV0GqXTgtHX6Pe5nKwgwntciPFXo0XXe5Ji3j2f
                                               YF4gD6Bgcsd9mLT1Jy5P+RECgYAhYA4r6vPBluCU1klW8JhE7FXgZVu4yQUpOeKS
                                               q1fZ8uCsaefnHFXNOBErp/2PdDCFn/XM7jAEAh1EMP+9Ess/0raIeLOE8oXzmEm0
                                               aOWxHa6b+i5x6YeN/2f9TpIrnruT5fJDuBqzld//xTETKG2UtJ5n8qzQfW2G6fTz
                                               25jSAQKBgHTcxDV9h284OkbMg05ya6P4OUg2TqqhUTrtyRcbL/9NYIqQXQdMgWy6
                                               yPSmK+RLCpia8eGC2ocRF5InrtOJo8LST+xi/o2bWS8tOycL5zb6bo/FY7K4uuOs
                                               IegWu/1bAIDWYQ+bgMOeyVh271MYfyqYeqp+dPHtBy64dy+zcVRs
                                               -----END RSA PRIVATE KEY-----)";

            static const char g_certificate[] = R"(-----BEGIN CERTIFICATE-----
                                                MIIEETCCAvmgAwIBAgIUHSfcF0FdHtaWapJ45czsrPVgWTkwDQYJKoZIhvcNAQEL
                                                BQAwgZgxCzAJBgNVBAYTAklUMQ4wDAYDVQQIDAVJdGFseTESMBAGA1UEBwwJTWFy
                                                YW5lbGxvMRwwGgYDVQQKDBNRWiBkaSBSb2JlcnRvIFZpb2xhMQswCQYDVQQLDAJN
                                                TzEcMBoGA1UEAwwTUVogZGkgUm9iZXJ0byBWaW9sYTEcMBoGCSqGSIb3DQEJARYN
                                                aW5mb0BxemFwcC5pdDAeFw0yMzAzMDYxNjAxNDNaFw0yNTA2MDgxNjAxNDNaMIGY
                                                MQswCQYDVQQGEwJJVDEOMAwGA1UECAwFSXRhbHkxEjAQBgNVBAcMCU1hcmFuZWxs
                                                bzEcMBoGA1UECgwTUVogZGkgUm9iZXJ0byBWaW9sYTELMAkGA1UECwwCTU8xHDAa
                                                BgNVBAMME1FaIGRpIFJvYmVydG8gVmlvbGExHDAaBgkqhkiG9w0BCQEWDWluZm9A
                                                cXphcHAuaXQwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC9lepP3KBk
                                                0/6/luMil8GZokP+jFcLT9mShLeFygMnYb/aRpj4EBIxWOj1z/hwHQa+NOcrX34f
                                                UfEZssELoxz3VQtglAPOkMhE5pryTFOUv4e0363fAb8Scp2GW+phMgY2XhuMcRLQ
                                                O+AEvTiNanIaE/aEoe9WtgMhyXg/BSez1ZNNnWmgvobA7/y6tm9bi6UKyWgX+9TS
                                                jWyDwCcsftwc1VBh/mGUXF1CJ0ZTwuDHFO7/N2KctA5Dee1kEqHZzMqXgWsixQE/
                                                044CbmrzZ/jnpxI6JVRLkJQTMuKPNZzBcmdFdsruQucJHTl2GesJUyjr55opF5gg
                                                cm1R0nonGp0JAgMBAAGjUTBPMB8GA1UdIwQYMBaAFOvhYXq8o414vNrtzg3TLIqE
                                                MhXBMAkGA1UdEwQCMAAwCwYDVR0PBAQDAgTwMBQGA1UdEQQNMAuCCWxvY2FsaG9z
                                                dDANBgkqhkiG9w0BAQsFAAOCAQEARcEbZhTn+d8RslYZPCL62HOo8ON3SNlmMCAc
                                                IFRUnLrpU9izOQbsjCiB9Hcnpi7uJ6NCZ7ziWqpvKwSveBKa5mXWcwHoJ3HoNdn9
                                                yOUMqWVQfau2hL5VyaNkxrjvn7YpHZ/KNnkg3d6F0NAPiWOC9P55OSsVL3uC2sOl
                                                qXASLijMxMfrHhbCJSZUb9QupUw0qEdb3Ey/QfGYZoLfxQV4UY11WcjyqBgpIRFJ
                                                PCsa4khdNgG0LmQ7UTI0IqXHpJWWEwfHYDLo1lNeQR9jJc7UvMGaqpRLGAPk4UK7
                                                +s19Qt+S5DS8USQdm2yBm8ZesM9Tdlexi4+HvQcdhQHWztj0EQ==
                                                -----END CERTIFICATE-----)";
            sslconf.setLocalCertificate(QSslCertificate(g_certificate));
            sslconf.setPrivateKey(QSslKey(g_privateKey, QSsl::Rsa));
            sslconf.setProtocol(QSsl::SecureProtocols);

            httpServer->sslSetup(sslconf);
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
            connect(httpServer, &QHttpServer::missingHandler, this, &WebServerInfoSender::missingHandler);
            return true;
        } else {
            reinit();
        }
    }
    return false;
}

void WebServerInfoSender::missingHandler(const QHttpServerRequest &request, QTcpSocket *socket) {
    qDebug() << request << socket;
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
