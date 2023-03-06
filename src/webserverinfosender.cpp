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
MIIEpQIBAAKCAQEA2hmgJiA3ynK4iHNEx2DYJ0PSC0hCC4z5LP+2jjc0EiGV8Ehh
0L/jp1j9eNBSLpDpAWvMSXEnh75qtVDkMgw9CR3+AAQYpTeEP3CtfRZCJeVFR1hz
dwYHK6TwOP6YSg3HRFwIx1Dnf73FxUg4aW3jfJLvjAvdNfbdmRclRVEwsBIB20zW
yEOtBdFFDCMqofttMAzeuUKJ4Sqx1Hi/qMSoggfuTE4VzASfeAd+OIso+nlDKkvC
mKdDemAV8nPqkeC+zi5NNlHavGbtyMPOpDOjHu0r5frK2PcE/ImrxSq4wMkgRxvz
yQRj4v/offZV8Lnkx/btgUG7fm5ics57TZe2cQIDAQABAoIBAQC1XQ3h5nS+cvuk
7EXLjmrw6+Lk4ueY91fV5acHSDySvz8SGv7MOmWzKbtOFd3hPzujpOKyR9hZdy0B
Lg002RyAUmJ/3Ogpq6rbnnWGSVYkcS1VEOajmW2pC3V/z6cQHbkQzX+3Tz3S6hpl
3ZKbiXfchcuRp+QCFhFcVvT1WXilUCpekfMOvApwtz8wa8/EBrnBrEfnvDXp2foT
uepO93KVLYxB6H/8w+hDHlDNOShrYnVmoLlYSY8k0A3iAluar2jhSMCMqnNPUZNc
eQQ0auHCtv0bcbUr0DgmRtw0KeynYN8lxPL1TCXEnuq1TcOCnsG9g9yFBeJ5bnv9
hxBkLfbJAoGBAPt/sZ8y+yXtpMHd24Tn+I5J41Tv4uotzdkpAMESIM4zH1iOVHRg
HiasFWtfDoPDAHzJ9ySPjzpx2Iw1DLPxPTnxkjbRNJ+/a9ymjb2MfKJy+CiQxnR1
yOHyofydSPm4S72ACu+QqtB+W4D/dbFrmDdMqa+Lir29ahqRGJ6Sx5+XAoGBAN4A
6DM/uCbU0uBvfoDCOFGnuVZvMSFi/+6GCissFb04CA9vnlw0KUOsZR+LTaA5uPAf
Mg3ISx4h48+XFwLuo6vdqeYPrqTU4R/kiES4Th4nlIwx4EKYQSleVOpusxQFrG7H
/Jkbvy9yfKrRCLHlgKhl07XQpnfTCBHS3vl0DJs3AoGAPYTjzvxqgMGbvqxlSSJx
I2UhjaJMUFo58zfbbl7f1XCd50yRjhIA7hbD+oq7AzINCXlh0fin4XT9snY7kDyf
bHASAwSzqavqV4QKQeZ+ikIzP8ECOnShWdu/EgrrUeJp7IXIpwctMWjnOyvB/09B
Ya2ICF/K5VTTAoKhkhB3cisCgYEAiL3ISGDUHd7y8Njb85UtuWX0679bZLQk7hdT
xhnGXiFYHaHjyek7DXrvYomcS7eiKQ0d+I1Jb4ryVhg7TUwONjs3fgRkzxRsggdy
w9k+DKbWga/q8Tn00PVQmoGK/icseaPm+6uyjBgZn+FXWLQDFjZ6O8AD+URb8RaQ
ztfjN+sCgYEA7+NzbfSTz3Jr/kS6X8zJX1PnTXNn/Q1GSWsAJ+LQdsoarsqCgh4h
f8Oqiexds7UXuoF7y6FQ0A61+axwCJJkpaXlRHV4kyuoJ0Wnmxc3Wn6t5FjWNKKJ
VAeILWcFFqENbPgWaDb7u1i4zvXDLl05Wds84aN1Ls7P9Bxn3A+hGmE=
-----END RSA PRIVATE KEY-----)";

            static const char g_certificate[] = R"(-----BEGIN CERTIFICATE-----
MIID/zCCAuegAwIBAgIUavB9rwuakAonloAUpLLwpajWlzswDQYJKoZIhvcNAQEL
BQAwgY4xCzAJBgNVBAYTAklUMQ4wDAYDVQQIDAVJdGFseTESMBAGA1UEBwwJTWFy
YW5lbGxvMRwwGgYDVQQKDBNRWiBkaSBSb2JlcnRvIFZpb2xhMQswCQYDVQQLDAJN
TzESMBAGA1UEAwwJbG9jYWxob3N0MRwwGgYJKoZIhvcNAQkBFg1pbmZvQHF6YXBw
Lml0MCAXDTIzMDMwNjE1Mjk0NFoYDzIwNzMwMjIxMTUyOTQ0WjCBjjELMAkGA1UE
BhMCSVQxDjAMBgNVBAgMBUl0YWx5MRIwEAYDVQQHDAlNYXJhbmVsbG8xHDAaBgNV
BAoME1FaIGRpIFJvYmVydG8gVmlvbGExCzAJBgNVBAsMAk1PMRIwEAYDVQQDDAls
b2NhbGhvc3QxHDAaBgkqhkiG9w0BCQEWDWluZm9AcXphcHAuaXQwggEiMA0GCSqG
SIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaGaAmIDfKcriIc0THYNgnQ9ILSEILjPks
/7aONzQSIZXwSGHQv+OnWP140FIukOkBa8xJcSeHvmq1UOQyDD0JHf4ABBilN4Q/
cK19FkIl5UVHWHN3BgcrpPA4/phKDcdEXAjHUOd/vcXFSDhpbeN8ku+MC9019t2Z
FyVFUTCwEgHbTNbIQ60F0UUMIyqh+20wDN65QonhKrHUeL+oxKiCB+5MThXMBJ94
B344iyj6eUMqS8KYp0N6YBXyc+qR4L7OLk02Udq8Zu3Iw86kM6Me7Svl+srY9wT8
iavFKrjAySBHG/PJBGPi/+h99lXwueTH9u2BQbt+bmJyzntNl7ZxAgMBAAGjUTBP
MB8GA1UdIwQYMBaAFJo+RDnyVlRurUkWSPf2vwiRFx/TMAkGA1UdEwQCMAAwCwYD
VR0PBAQDAgTwMBQGA1UdEQQNMAuCCWxvY2FsaG9zdDANBgkqhkiG9w0BAQsFAAOC
AQEAfNgMX4WC5ONORRHOHHItKPL4kCH/Co1bXV9yNi8RL9hEUniB0WYipMudTa4e
FZsWwT1HwkERA4f6+cZB1xsdqJqBhFwPwaUmi6A36YF3nRM1ObtuY0g/VLJqR/6R
vglccAlI0S8ciIE6x+9/O1YGGjY3kz1HRaEw0SlMXqMS3QUfFnzTeS+nAjs9aZ6K
FNn5rA+PGueZvSSmJXNlrRYwEsLnnOKBRkaGYBo/91lCBbh/nlru5ITfqVb3U81W
KQllMd0tsPNIoi4UJrpM6YmVlGV1tGh5NqdAmZSJM3SYbRpsPA1JntARMxfu3Hh1
nwzfKXPJNzflLRqxL28YfX45Nw==
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
