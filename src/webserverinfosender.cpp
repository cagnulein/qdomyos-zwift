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

            static const char g_privateKey[] = R"(-----BEGIN PRIVATE KEY-----
                                               MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCfxIwp8v2ruyo7
                                               SQyxuHNMdRKNm6ETkNyTU64mGurtWq7rcMma9AsER7e3oFV2kYVUIxoBRbLrlYnx
                                               EcWMyEHopEm2LZbbqGNE3U7JZGGr84VNyfuRXqNM/QgRRjP4h2Tq9dT9AduNlQdl
                                               VDU7kfyJpHMfh6HNqdzZs48QEbpueypqoyCPevENZLKmbegisJh+A4ffnRO30t5M
                                               L5639kjnBygouKNSQpFwPM/zU+guBCR0KgQxdn7urgkqkqTE+lCOOnVvUJmU4mCP
                                               cM0+8Svaozzm20cwP5XEYzTjmUBSyGEZeGZSbUFCxEOCvLHm/FQqCsGAk8GopnhS
                                               h1qwO4TxAgMBAAECggEAe6JigNfb+7fjr2sRGrpM5v61s8WaOYThXFTmL/CmmbSS
                                               Jfnq2TE/ETnabovxdsaXsYtURWXFVk8rJtUE1bPmZPw1WngfBK5i21iS7n/yoVja
                                               cF78gPsGTb6FLrDv6MQFkVlZT8zPNNn9lik65HVNQspymBSiXn+zOvibnejKJ1GJ
                                               ePCggHMT8y1CreXr2BkDjNUR+4exd9QatJbfSSdmyc+Fs+gXZ2w+IrW114rM6GQb
                                               p56LfKDI2R3dH0zc/1g53cgoL7MgvqxuggjzIASGw4Xo3dysxsxBgpysB0y7AYrL
                                               7Jt1nE9QT6AT0F5su/2Q2g3ImTTiXqumPzO8TYtCAQKBgQDKuoOAJTyHuqETPwXn
                                               4QFQMdHczmZUEobeHuWHnidy8HV0Aw3cX5ZxRqZch1Dw9fOOYna6KJKaq2Yw5Ghz
                                               DcqlE1cP6Qmim8bmD4/z763/IIJ5T8J18p3AKEw5/Br+nbWp8N7ZMDz9xa6+qkrm
                                               Nfr9xMGdWQh5Oyu+Sknw2EtxkQKBgQDJwBL6x9evtCaOOGnA/0vpsoFMaFX0SBx+
                                               NRCv5ehrHW5D88Py88O0ymRHIEVaMMknghlcTUln58KYrk0kzqhNLoZYZVf6ivpt
                                               9qQaRnIUgy8krlrDiyvn4OFnhfh5DNm/nd+wikPJEwcI1q0z8R7+g9y3RKCNL6s2
                                               HQG3Jj8tYQKBgEoBVEfPUA1sP6i69Pj01nnj9exZHFnMeZdUSA392gDHbtju1HyC
                                               GHU5iTl13EJaRpLPtu+2J+52a1OlzctSWYtxR/Ly6yWFIFKTk8VE1Yuw3nBSgXZ1
                                               Hskq7MiuE1ynTc2/tFosldc99tB7ceQgCIPi85rxCXrX8twAWoWlL8VBAoGAPg06
                                               iTGAEEWLekC1niuncJh7lkGc9kZbpSGzITbIVnG14WaPRSFedzOsxgeR7RyGMgWc
                                               wTvwMOoiew1ZItIBB8Qgg/2foqquPbYXYRF7sv1qOZD6z5v1hBsxmMKm2qxuKLZo
                                               /4Z3NNgKWNiWaxxlWQi7kQ6lhuc7dKhVR7yWv+ECgYAuM2gtT83DyR4ymSO7Q6dv
                                               dT/UObzrkPfDmkRW2y4PUqAC6Pa/9+4HbBgHGpta0mIu7jwzSeDzvKhOf39TOdGd
                                               rgqC8coanU3KUQ0hQ3qxc31sK04xd0oL7IlHN84raYakjpldRcGDa5rMBvuAhX7l
                                               UtzKXXRaWNoRy0DfRWzoyw==
                                               -----END PRIVATE KEY-----)";

            static const char g_certificate[] = R"(-----BEGIN CERTIFICATE-----
                                                MIIGSTCCBTGgAwIBAgIMdxktQkXvAVtRN9TFMA0GCSqGSIb3DQEBCwUAMEwxCzAJ
                                                BgNVBAYTAkJFMRkwFwYDVQQKExBHbG9iYWxTaWduIG52LXNhMSIwIAYDVQQDExlB
                                                bHBoYVNTTCBDQSAtIFNIQTI1NiAtIEcyMB4XDTIyMDMyOTAyNTIyMVoXDTIzMDQz
                                                MDAyNTIyMFowHTEbMBkGA1UEAwwSKi5sb2NhbGhvc3QuZGlyZWN0MIIBIjANBgkq
                                                hkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAn8SMKfL9q7sqO0kMsbhzTHUSjZuhE5Dc
                                                k1OuJhrq7Vqu63DJmvQLBEe3t6BVdpGFVCMaAUWy65WJ8RHFjMhB6KRJti2W26hj
                                                RN1OyWRhq/OFTcn7kV6jTP0IEUYz+Idk6vXU/QHbjZUHZVQ1O5H8iaRzH4ehzanc
                                                2bOPEBG6bnsqaqMgj3rxDWSypm3oIrCYfgOH350Tt9LeTC+et/ZI5wcoKLijUkKR
                                                cDzP81PoLgQkdCoEMXZ+7q4JKpKkxPpQjjp1b1CZlOJgj3DNPvEr2qM85ttHMD+V
                                                xGM045lAUshhGXhmUm1BQsRDgryx5vxUKgrBgJPBqKZ4UodasDuE8QIDAQABo4ID
                                                WDCCA1QwDgYDVR0PAQH/BAQDAgWgMIGKBggrBgEFBQcBAQR+MHwwQwYIKwYBBQUH
                                                MAKGN2h0dHA6Ly9zZWN1cmUuZ2xvYmFsc2lnbi5jb20vY2FjZXJ0L2dzYWxwaGFz
                                                aGEyZzJyMS5jcnQwNQYIKwYBBQUHMAGGKWh0dHA6Ly9vY3NwMi5nbG9iYWxzaWdu
                                                LmNvbS9nc2FscGhhc2hhMmcyMFcGA1UdIARQME4wQgYKKwYBBAGgMgEKCjA0MDIG
                                                CCsGAQUFBwIBFiZodHRwczovL3d3dy5nbG9iYWxzaWduLmNvbS9yZXBvc2l0b3J5
                                                LzAIBgZngQwBAgEwCQYDVR0TBAIwADA/BgNVHR8EODA2MDSgMqAwhi5odHRwOi8v
                                                Y3JsLmdsb2JhbHNpZ24uY29tL2dzL2dzYWxwaGFzaGEyZzIuY3JsMC8GA1UdEQQo
                                                MCaCEioubG9jYWxob3N0LmRpcmVjdIIQbG9jYWxob3N0LmRpcmVjdDAdBgNVHSUE
                                                FjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwHwYDVR0jBBgwFoAU9c3VPAhQ+WpPOreX
                                                2laD5mnSaPcwHQYDVR0OBBYEFKJIjzgsaaVBT/uEaAvJNihZEmG5MIIBfgYKKwYB
                                                BAHWeQIEAgSCAW4EggFqAWgAdQDoPtDaPvUGNTLnVyi8iWvJA9PL0RFr7Otp4Xd9
                                                bQa9bgAAAX/TlrBWAAAEAwBGMEQCIHSp1pDooZSiB2vOUgXCcOjw4JZL0h3/41Jb
                                                XpDjEgOqAiBOFnt0KMGpLhgXKmOC7yllaNKRApke2awT2FDVZyaUTgB2AG9Tdqwx
                                                8DEZ2JkApFEV/3cVHBHZAsEAKQaNsgiaN9kTAAABf9OWsFQAAAQDAEcwRQIhAPna
                                                KZ58189iW3tkCKB/LSu94Hg5BDZ/Md23nLYYHBMhAiBviFfs4hzlRv3Ivae3r5NJ
                                                S9c3kUzAELD2eFWSOnj6iAB3AFWB1MIWkDYBSuoLm1c8U/DA5Dh4cCUIFy+jqh0H
                                                E9MMAAABf9OWsIMAAAQDAEgwRgIhAJBxU2mJB7HLFrmgTrfWCtJvPK3YUhvKG13G
                                                lrMMGrHoAiEA4me4djsIMHHabov1eQrrvkaI95xFTyqKfQl3FK6A9y4wDQYJKoZI
                                                hvcNAQELBQADggEBACXSYB2WTlREfpuquD75rZTja2qFkDkrFjnCes50tqqVxemF
                                                G4kGV7N/7aSr1T9BvW/DCzYy5UABabvEM+MGaHNcMibYqoSNWgfvAULZPVzL45Uz
                                                pghoz7xci2ol5/5ceNx7YEW0nehw2r7A4/6mjFYbkOIzBILz4zOlilmYHBTDkdMC
                                                W45YM5RbOqWCqGDwSuASH5hEoHDDsRhJksHjJNCoX0PKMilscqfVw5h80xRVEtVu
                                                DLBXL79GXXSQk2q1jxVv1i3o29TYw2bk3WCM8Uw2LJV2jerKF2+9ymboZKURKfHU
                                                e20WdjHUOZc3kHLXYGVjjU1/e+Thf6DrUn3YmPU=
                                                -----END CERTIFICATE-----
                                                -----BEGIN CERTIFICATE-----
                                                MIIETTCCAzWgAwIBAgILBAAAAAABRE7wNjEwDQYJKoZIhvcNAQELBQAwVzELMAkG
                                                A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv
                                                b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw0xNDAyMjAxMDAw
                                                MDBaFw0yNDAyMjAxMDAwMDBaMEwxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i
                                                YWxTaWduIG52LXNhMSIwIAYDVQQDExlBbHBoYVNTTCBDQSAtIFNIQTI1NiAtIEcy
                                                MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2gHs5OxzYPt+j2q3xhfj
                                                kmQy1KwA2aIPue3ua4qGypJn2XTXXUcCPI9A1p5tFM3D2ik5pw8FCmiiZhoexLKL
                                                dljlq10dj0CzOYvvHoN9ItDjqQAu7FPPYhmFRChMwCfLew7sEGQAEKQFzKByvkFs
                                                MVtI5LHsuSPrVU3QfWJKpbSlpFmFxSWRpv6mCZ8GEG2PgQxkQF5zAJrgLmWYVBAA
                                                cJjI4e00X9icxw3A1iNZRfz+VXqG7pRgIvGu0eZVRvaZxRsIdF+ssGSEj4k4HKGn
                                                kCFPAm694GFn1PhChw8K98kEbSqpL+9Cpd/do1PbmB6B+Zpye1reTz5/olig4het
                                                ZwIDAQABo4IBIzCCAR8wDgYDVR0PAQH/BAQDAgEGMBIGA1UdEwEB/wQIMAYBAf8C
                                                AQAwHQYDVR0OBBYEFPXN1TwIUPlqTzq3l9pWg+Zp0mj3MEUGA1UdIAQ+MDwwOgYE
                                                VR0gADAyMDAGCCsGAQUFBwIBFiRodHRwczovL3d3dy5hbHBoYXNzbC5jb20vcmVw
                                                b3NpdG9yeS8wMwYDVR0fBCwwKjAooCagJIYiaHR0cDovL2NybC5nbG9iYWxzaWdu
                                                Lm5ldC9yb290LmNybDA9BggrBgEFBQcBAQQxMC8wLQYIKwYBBQUHMAGGIWh0dHA6
                                                Ly9vY3NwLmdsb2JhbHNpZ24uY29tL3Jvb3RyMTAfBgNVHSMEGDAWgBRge2YaRQ2X
                                                yolQL30EzTSo//z9SzANBgkqhkiG9w0BAQsFAAOCAQEAYEBoFkfnFo3bXKFWKsv0
                                                XJuwHqJL9csCP/gLofKnQtS3TOvjZoDzJUN4LhsXVgdSGMvRqOzm+3M+pGKMgLTS
                                                xRJzo9P6Aji+Yz2EuJnB8br3n8NA0VgYU8Fi3a8YQn80TsVD1XGwMADH45CuP1eG
                                                l87qDBKOInDjZqdUfy4oy9RU0LMeYmcI+Sfhy+NmuCQbiWqJRGXy2UzSWByMTsCV
                                                odTvZy84IOgu/5ZR8LrYPZJwR2UcnnNytGAMXOLRc3bgr07i5TelRS+KIz6HxzDm
                                                MTh89N1SyvNTBCVXVmaU6Avu5gMUTu79bZRknl7OedSyps9AsUSoPocZXun4IRZZ
                                                Uw==
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
