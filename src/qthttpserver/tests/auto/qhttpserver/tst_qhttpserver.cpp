/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtHttpServer module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtHttpServer/qhttpserver.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverrouterrule.h>

#if QT_CONFIG(concurrent)
#  include <QtHttpServer/qhttpserverfutureresponse.h>
#endif

#include <private/qhttpserverrouterrule_p.h>
#include <private/qhttpserverliterals_p.h>

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>

#include <QtCore/qurl.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qtimer.h>

#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>


static const char g_privateKey[] = R"(-----BEGIN RSA PRIVATE KEY-----
MIICXQIBAAKBgQDykG51ZjNJra8iS27g3DJojH1qG8C3Z+Avo5U6Qz6NkOsjvr22
gXqOS4uwVUXdCAKxsP0Wwn2zGz5vxGpLPVKtbAmaqHYZuipMG/Qun3t+QYBgR+9t
lmHdI8TNP2Om8stDO5uQyBH7DcMjPyIgpfc8fBoNLhCn4oC2n6JK9EMuhQIDAQAB
AoGAUHTLzrEJjgTINI3kxz0Ck18WMl3mPG9+Ew8lbl/jnb1V4VNhReoIpq40NVbz
h28ixaG5MRVt8Dy3Jwd1YmOCylHSujdFQ2u0pcHFmERgDS2bOMwMTRoFOj2qgMGS
9SM+iXlPY5AQY8nEg7rLjMSfaC/8Hq4RXpkj4PeHh6N7AzkCQQD++HzM3xBr+Gvh
zco9Kt8IiKNlfeiA5gUQq1UPJzcWIEgW1Tgr5UzMUOcZ0HfYwhqL3+wMhzN4sba+
1plB1QRXAkEA84sfM0jm9BRSqtYTPlhsYAmuPjeo24Pxel8ijEkToAu0ppEC6AQ3
zfwZD0ISgaWQ7af5TN/RCsoNVX79twP6gwJBANbtB+Z6shERm38ARdZB6Tf8ViAb
fn4JZ4OhqVXYrKrOE3aLzYnTBGXGXMh53kytcksuOoBlB5JZ274Kj63arokCQFPo
9xMAZzJpXiImJ/MvHAfqzfH501/ukeCLrqeO9ggKgG9zPwEZkvCRj0DGjwHEPa7k
VOy7oJaLDxUJ7/iCkmkCQQCtTLsvDbGH4tyFK5VIPJbUcccIib+dTzSTeONdUxKL
Yk+C6o7OpaUWX+ikp4Ow/6iHOAgXaeA2OolDer/NspUy
-----END RSA PRIVATE KEY-----)";

static const char g_certificate[] = R"(-----BEGIN CERTIFICATE-----
MIICrjCCAhegAwIBAgIUcuXjCSkJ2+v/Rqv/UHThTRGFlpswDQYJKoZIhvcNAQEL
BQAwaDELMAkGA1UEBhMCRlIxDzANBgNVBAgMBkZyYW5jZTERMA8GA1UEBwwIR3Jl
bm9ibGUxFjAUBgNVBAoMDVF0Q29udHJpYnV0b3IxHTAbBgNVBAMMFHFodHRwc3Nl
cnZlcnRlc3QuY29tMCAXDTE5MDkyNjA4NTc1MloYDzIyNTUwMzEzMDg1NzUyWjBo
MQswCQYDVQQGEwJGUjEPMA0GA1UECAwGRnJhbmNlMREwDwYDVQQHDAhHcmVub2Js
ZTEWMBQGA1UECgwNUXRDb250cmlidXRvcjEdMBsGA1UEAwwUcWh0dHBzc2VydmVy
dGVzdC5jb20wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAPKQbnVmM0mtryJL
buDcMmiMfWobwLdn4C+jlTpDPo2Q6yO+vbaBeo5Li7BVRd0IArGw/RbCfbMbPm/E
aks9Uq1sCZqodhm6Kkwb9C6fe35BgGBH722WYd0jxM0/Y6byy0M7m5DIEfsNwyM/
IiCl9zx8Gg0uEKfigLafokr0Qy6FAgMBAAGjUzBRMB0GA1UdDgQWBBTDMYCcl2jz
UUWByEzTj5Ew/LWkeDAfBgNVHSMEGDAWgBTDMYCcl2jzUUWByEzTj5Ew/LWkeDAP
BgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4GBAMNupAOXoBih6RvuAn3w
W8jOIZfkn5CMYdbUSndY/Wrt4p07M8r9uFPWG4bXSwG6n9Nzl75X9b0ka/jqPjQ3
X769simPygCblBp2xwE6w14aHEBx4kcF1p2QbC1vHynszJxyVLvHqUjuJwVAoPrM
Imy6LOiw2tRTHPsj7UH16M6C
-----END CERTIFICATE-----)";

QT_BEGIN_NAMESPACE

class QueryRequireRouterRule : public QHttpServerRouterRule
{
public:
    QueryRequireRouterRule(const QString &pathPattern,
                           const char *queryKey,
                           RouterHandler &&routerHandler)
        : QHttpServerRouterRule(pathPattern, std::forward<RouterHandler>(routerHandler)),
          m_queryKey(queryKey)
    {
    }

    bool matches(const QHttpServerRequest &request, QRegularExpressionMatch *match) const override
    {
        if (QHttpServerRouterRule::matches(request, match)) {
            if (request.query().hasQueryItem(m_queryKey))
                return true;
        }

        return false;
    }

private:
    const char * m_queryKey;
};

class tst_QHttpServer final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void routeGet_data();
    void routeGet();
    void routeKeepAlive();
    void routePost_data();
    void routePost();
    void routeDelete_data();
    void routeDelete();
    void routeExtraHeaders();
    void invalidRouterArguments();
    void checkRouteLambdaCapture();
    void afterRequest();
    void disconnectedInEventLoop();

private:
    void checkReply(QNetworkReply *reply, const QString &response);

private:
    QHttpServer httpserver;
    QString urlBase;
    QString sslUrlBase;
    QNetworkAccessManager networkAccessManager;
};

struct CustomArg {
    int data = 10;

    CustomArg() {} ;
    CustomArg(const QString &urlArg) : data(urlArg.toInt()) {}
};

void tst_QHttpServer::initTestCase()
{

    httpserver.route("/req-and-resp", [] (QHttpServerResponder &&resp,
                                          const QHttpServerRequest &req) {
        resp.write(req.body(),
                   QHttpServerLiterals::contentTypeTextHtml());
    });

    httpserver.route("/resp-and-req", [] (const QHttpServerRequest &req,
                                          QHttpServerResponder &&resp) {
        resp.write(req.body(),
                   QHttpServerLiterals::contentTypeTextHtml());
    });

    httpserver.route("/test", [] (QHttpServerResponder &&responder) {
        responder.write("test msg",
                        QHttpServerLiterals::contentTypeTextHtml());
    });

    httpserver.route("/", QHttpServerRequest::Method::Get, [] () {
        return "Hello world get";
    });

    httpserver.route("/", QHttpServerRequest::Method::Post, [] () {
        return "Hello world post";
    });

    httpserver.route("/post-and-get", "GET|POST", [] (const QHttpServerRequest &request) {
        if (request.method() == QHttpServerRequest::Method::Get)
            return "Hello world get";
        else if (request.method() == QHttpServerRequest::Method::Post)
            return "Hello world post";

        return "This should not work";
    });

    httpserver.route("/any", "All", [] (const QHttpServerRequest &request) {
        static const auto metaEnum = QMetaEnum::fromType<QHttpServerRequest::Method>();
        return metaEnum.valueToKey(static_cast<int>(request.method()));
    });

    httpserver.route("/page/", [] (const qint32 number) {
        return QString("page: %1").arg(number);
    });

    httpserver.route("/page/<arg>/detail", [] (const quint32 number) {
        return QString("page: %1 detail").arg(number);
    });

    httpserver.route("/user/", [] (const QString &name) {
        return QString("%1").arg(name);
    });

    httpserver.route("/user/<arg>/", [] (const QString &name, const QByteArray &ba) {
        return QString("%1-%2").arg(name).arg(QString::fromLatin1(ba));
    });

    httpserver.route("/test/", [] (const QUrl &url) {
        return QString("path: %1").arg(url.path());
    });

    httpserver.route("/api/v", [] (const float api) {
        return QString("api %1v").arg(api);
    });

    httpserver.route("/api/v<arg>/user/", [] (const float api, const quint64 user) {
        return QString("api %1v, user id - %2").arg(api).arg(user);
    });

    httpserver.route("/api/v<arg>/user/<arg>/settings", [] (const float api, const quint64 user,
                                                             const QHttpServerRequest &request) {
        const auto &role = request.query().queryItemValue(QString::fromLatin1("role"));
        const auto &fragment = request.url().fragment();

        return QString("api %1v, user id - %2, set settings role=%3#'%4'")
                   .arg(api).arg(user).arg(role, fragment);
    });

    httpserver.route<QueryRequireRouterRule>(
            "/custom/",
            "key",
            [] (const quint64 num, const QHttpServerRequest &request) {
        return QString("Custom router rule: %1, key=%2")
                    .arg(num)
                    .arg(request.query().queryItemValue("key"));
    });

    httpserver.router()->addConverter<CustomArg>(QLatin1String("[+-]?\\d+"));
    httpserver.route("/check-custom-type/", [] (const CustomArg &customArg) {
        return QString("data = %1").arg(customArg.data);
    });

    httpserver.route("/post-body", "POST", [] (const QHttpServerRequest &request) {
        return request.body();
    });

    httpserver.route("/file/", [] (const QString &file) {
        return QHttpServerResponse::fromFile(QFINDTESTDATA(QLatin1String("data/") + file));
    });

    httpserver.route("/json-object/", [] () {
        return QJsonObject{
            {"property", "test"},
            {"value", 1}
        };
    });

    httpserver.route("/json-array/", [] () {
        return QJsonArray{
            1, "2",
            QJsonObject{
                {"name", "test"}
            }
        };
    });

    httpserver.route("/chunked/", [] (QHttpServerResponder &&responder) {
        responder.writeStatusLine(QHttpServerResponder::StatusCode::Ok);
        responder.writeHeaders({
                {"Content-Type", "text/plain"},
                {"Transfer-Encoding", "chunked"} });

        auto writeChunk = [&responder] (const char *message) {
            responder.writeBody(QByteArray::number(qstrlen(message), 16));
            responder.writeBody("\r\n");
            responder.writeBody(message);
            responder.writeBody("\r\n");
        };

        writeChunk("part 1 of the message, ");
        writeChunk("part 2 of the message");
        writeChunk("");
    });

    httpserver.route("/extra-headers", [] () {
        QHttpServerResponse resp("");
        resp.setHeader("Content-Type", "application/x-empty");
        resp.setHeader("Server", "test server");
        return resp;
    });

    httpserver.afterRequest([] (QHttpServerResponse &&resp) {
        return std::move(resp);
    });

#if QT_CONFIG(concurrent)
    httpserver.route("/future/", [] (int id) -> QHttpServerFutureResponse {
        if (id == 0)
            return QHttpServerResponse::StatusCode::NotFound;

        auto future = QtConcurrent::run([] () {
            QTest::qSleep(500);
            return QHttpServerResponse("future is coming");
        });

        return future;
    });
#endif

    quint16 port = httpserver.listen();
    if (!port)
        qCritical() << "Http server listen failed";

    urlBase = QStringLiteral("http://localhost:%1%2").arg(port);

#if QT_CONFIG(ssl)
    httpserver.sslSetup(QSslCertificate(g_certificate),
                        QSslKey(g_privateKey, QSsl::Rsa));

    port = httpserver.listen();
    if (!port)
        qCritical() << "Http server listen failed";

    sslUrlBase = QStringLiteral("https://localhost:%1%2").arg(port);

    QList<QSslError> expectedSslErrors;

// Non-OpenSSL backends are not able to report a specific error code
// for self-signed certificates.
#ifndef QT_NO_OPENSSL
# define FLUKE_CERTIFICATE_ERROR QSslError::SelfSignedCertificate
#else
# define FLUKE_CERTIFICATE_ERROR QSslError::CertificateUntrusted
#endif

    expectedSslErrors.append(QSslError(FLUKE_CERTIFICATE_ERROR,
                                       QSslCertificate(g_certificate)));
    expectedSslErrors.append(QSslError(QSslError::HostNameMismatch,
                                       QSslCertificate(g_certificate)));

    connect(&networkAccessManager, &QNetworkAccessManager::sslErrors,
            [expectedSslErrors](QNetworkReply *reply,
                                const QList<QSslError> &errors) {
        for (const auto &error: errors) {
            for (const auto &expectedError: expectedSslErrors) {
                if (error.error() != expectedError.error() ||
                    error.certificate() != expectedError.certificate()) {
                    qCritical() << "Got unexpected ssl error:"
                                << error << error.certificate();
                }
            }
        }
        reply->ignoreSslErrors(expectedSslErrors);
    });
#endif
}

void tst_QHttpServer::routeGet_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("body");

    QTest::addRow("hello world")
        << urlBase.arg("/")
        << 200
        << "text/plain"
        << "Hello world get";

    QTest::addRow("test msg")
        << urlBase.arg("/test")
        << 200
        << "text/html"
        << "test msg";

    QTest::addRow("not found")
        << urlBase.arg("/not-found")
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("arg:int")
        << urlBase.arg("/page/10")
        << 200
        << "text/plain"
        << "page: 10";

    QTest::addRow("arg:-int")
        << urlBase.arg("/page/-10")
        << 200
        << "text/plain"
        << "page: -10";

    QTest::addRow("arg:uint")
        << urlBase.arg("/page/10/detail")
        << 200
        << "text/plain"
        << "page: 10 detail";

    QTest::addRow("arg:-uint")
        << urlBase.arg("/page/-10/detail")
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("arg:string")
        << urlBase.arg("/user/test")
        << 200
        << "text/plain"
        << "test";

    QTest::addRow("arg:string")
        << urlBase.arg("/user/test test ,!a+.")
        << 200
        << "text/plain"
        << "test test ,!a+.";

    QTest::addRow("arg:string,ba")
        << urlBase.arg("/user/james/bond")
        << 200
        << "text/plain"
        << "james-bond";

    QTest::addRow("arg:url")
        << urlBase.arg("/test/api/v0/cmds?val=1")
        << 200
        << "text/plain"
        << "path: api/v0/cmds";

    QTest::addRow("arg:float 5.1")
        << urlBase.arg("/api/v5.1")
        << 200
        << "text/plain"
        << "api 5.1v";

    QTest::addRow("arg:float 5.")
        << urlBase.arg("/api/v5.")
        << 200
        << "text/plain"
        << "api 5v";

    QTest::addRow("arg:float 6.0")
        << urlBase.arg("/api/v6.0")
        << 200
        << "text/plain"
        << "api 6v";

    QTest::addRow("arg:float,uint")
        << urlBase.arg("/api/v5.1/user/10")
        << 200
        << "text/plain"
        << "api 5.1v, user id - 10";

    QTest::addRow("arg:float,uint,query")
        << urlBase.arg("/api/v5.2/user/11/settings?role=admin")
        << 200
        << "text/plain"
        << "api 5.2v, user id - 11, set settings role=admin#''";

    // The fragment isn't actually sent via HTTP (it's information for the user agent)
    QTest::addRow("arg:float,uint, query+fragment")
        << urlBase.arg("/api/v5.2/user/11/settings?role=admin#tag")
        << 200
        << "text/plain"
        << "api 5.2v, user id - 11, set settings role=admin#''";

    QTest::addRow("custom route rule")
        << urlBase.arg("/custom/15")
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("custom route rule + query")
        << urlBase.arg("/custom/10?key=11&g=1")
        << 200
        << "text/plain"
        << "Custom router rule: 10, key=11";

    QTest::addRow("custom route rule + query key req")
        << urlBase.arg("/custom/10?g=1&key=12")
        << 200
        << "text/plain"
        << "Custom router rule: 10, key=12";

    QTest::addRow("post-and-get, get")
        << urlBase.arg("/post-and-get")
        << 200
        << "text/plain"
        << "Hello world get";

    QTest::addRow("invalid-rule-method, get")
        << urlBase.arg("/invalid-rule-method")
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("check custom type, data=1")
        << urlBase.arg("/check-custom-type/1")
        << 200
        << "text/plain"
        << "data = 1";

    QTest::addRow("any, get")
        << urlBase.arg("/any")
        << 200
        << "text/plain"
        << "Get";

    QTest::addRow("response from html file")
        << urlBase.arg("/file/text.html")
        << 200
        << "text/html"
        << "<html></html>";

    QTest::addRow("response from json file")
        << urlBase.arg("/file/application.json")
        << 200
        << "application/json"
        << "{ \"key\": \"value\" }";

    QTest::addRow("json-object")
        << urlBase.arg("/json-object/")
        << 200
        << "application/json"
        << "{\"property\":\"test\",\"value\":1}";

    QTest::addRow("json-array")
        << urlBase.arg("/json-array/")
        << 200
        << "application/json"
        << "[1,\"2\",{\"name\":\"test\"}]";

    QTest::addRow("chunked")
        << urlBase.arg("/chunked/")
        << 200
        << "text/plain"
        << "part 1 of the message, part 2 of the message";

#if QT_CONFIG(concurrent)
    QTest::addRow("future")
        << urlBase.arg("/future/1")
        << 200
        << "text/plain"
        << "future is coming";

    QTest::addRow("future-not-found")
        << urlBase.arg("/future/0")
        << 404
        << "application/x-empty"
        << "";
#endif

#if QT_CONFIG(ssl)

    QTest::addRow("hello world, ssl")
        << sslUrlBase.arg("/")
        << 200
        << "text/plain"
        << "Hello world get";

    QTest::addRow("post-and-get, get, ssl")
        << sslUrlBase.arg("/post-and-get")
        << 200
        << "text/plain"
        << "Hello world get";

    QTest::addRow("invalid-rule-method, get, ssl")
        << sslUrlBase.arg("/invalid-rule-method")
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("check custom type, data=1, ssl")
        << sslUrlBase.arg("/check-custom-type/1")
        << 200
        << "text/plain"
        << "data = 1";

#endif // QT_CONFIG(ssl)
}

void tst_QHttpServer::routeGet()
{
    QFETCH(QString, url);
    QFETCH(int, code);
    QFETCH(QString, type);
    QFETCH(QString, body);

    auto reply = networkAccessManager.get(QNetworkRequest(url));

    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);
    QCOMPARE(reply->readAll().trimmed(), body);

    reply->deleteLater();
}

void tst_QHttpServer::routeKeepAlive()
{
    httpserver.route("/keep-alive", [] (const QHttpServerRequest &req) -> QHttpServerResponse {
        if (req.headers()["Connection"] != "keep-alive")
            return QHttpServerResponse::StatusCode::NotFound;

        return QString("header: %1, query: %2, body: %3, method: %4")
            .arg(req.value("CustomHeader"),
                 req.url().query(),
                 req.body())
            .arg(static_cast<int>(req.method()));
    });

    QNetworkRequest request(urlBase.arg("/keep-alive"));
    request.setRawHeader(QByteArray("Connection"), QByteArray("keep-alive"));

    checkReply(networkAccessManager.get(request),
               QString("header: , query: , body: , method: %1")
                 .arg(static_cast<int>(QHttpServerRequest::Method::Get)));
    if (QTest::currentTestFailed())
        return;

    request.setUrl(urlBase.arg("/keep-alive?po=98"));
    request.setRawHeader("CustomHeader", "1");
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QHttpServerLiterals::contentTypeTextHtml());

    checkReply(networkAccessManager.post(request, QByteArray("test")),
               QString("header: 1, query: po=98, body: test, method: %1")
                 .arg(static_cast<int>(QHttpServerRequest::Method::Post)));
    if (QTest::currentTestFailed())
        return;

    request = QNetworkRequest(urlBase.arg("/keep-alive"));
    request.setRawHeader(QByteArray("Connection"), QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QHttpServerLiterals::contentTypeTextHtml());

    checkReply(networkAccessManager.post(request, QByteArray("")),
               QString("header: , query: , body: , method: %1")
                 .arg(static_cast<int>(QHttpServerRequest::Method::Post)));
    if (QTest::currentTestFailed())
        return;

    checkReply(networkAccessManager.get(request),
               QString("header: , query: , body: , method: %1")
                 .arg(static_cast<int>(QHttpServerRequest::Method::Get)));
    if (QTest::currentTestFailed())
        return;
}

void tst_QHttpServer::routePost_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("data");
    QTest::addColumn<QString>("body");

    QTest::addRow("hello world")
        << urlBase.arg("/")
        << 200
        << "text/plain"
        << ""
        << "Hello world post";

    QTest::addRow("post-and-get, post")
        << urlBase.arg("/post-and-get")
        << 200
        << "text/plain"
        << ""
        << "Hello world post";

    QTest::addRow("any, post")
        << urlBase.arg("/any")
        << 200
        << "text/plain"
        << ""
        << "Post";

    QTest::addRow("post-body")
        << urlBase.arg("/post-body")
        << 200
        << "text/plain"
        << "some post data"
        << "some post data";

    QString body;
    for (int i = 0; i < 10000; i++)
        body.append(QString::number(i));

    QTest::addRow("post-body - huge body, chunk test")
        << urlBase.arg("/post-body")
        << 200
        << "text/plain"
        << body
        << body;

    QTest::addRow("req-and-resp")
        << urlBase.arg("/req-and-resp")
        << 200
        << "text/html"
        << "test"
        << "test";

    QTest::addRow("resp-and-req")
        << urlBase.arg("/resp-and-req")
        << 200
        << "text/html"
        << "test"
        << "test";

#if QT_CONFIG(ssl)

    QTest::addRow("post-and-get, post, ssl")
        << sslUrlBase.arg("/post-and-get")
        << 200
        << "text/plain"
        << ""
        << "Hello world post";

    QTest::addRow("any, post, ssl")
        << sslUrlBase.arg("/any")
        << 200
        << "text/plain"
        << ""
        << "Post";

    QTest::addRow("post-body, ssl")
        << sslUrlBase.arg("/post-body")
        << 200
        << "text/plain"
        << "some post data"
        << "some post data";

    QTest::addRow("post-body - huge body, chunk test, ssl")
        << sslUrlBase.arg("/post-body")
        << 200
        << "text/plain"
        << body
        << body;

#endif // QT_CONFIG(ssl)
}

void tst_QHttpServer::routePost()
{
    QFETCH(QString, url);
    QFETCH(int, code);
    QFETCH(QString, type);
    QFETCH(QString, data);
    QFETCH(QString, body);

    QNetworkRequest request(url);
    if (data.size()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QHttpServerLiterals::contentTypeTextHtml());
    }

    auto reply = networkAccessManager.post(request, data.toUtf8());

    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);
    QCOMPARE(reply->readAll(), body);

    reply->deleteLater();
}

void tst_QHttpServer::routeDelete_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("data");

    QTest::addRow("post-and-get, delete")
        << urlBase.arg("/post-and-get")
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("any, delete")
        << urlBase.arg("/any")
        << 200
        << "text/plain"
        << "Delete";

#if QT_CONFIG(ssl)

    QTest::addRow("post-and-get, delete, ssl")
        << sslUrlBase.arg("/post-and-get")
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("any, delete, ssl")
        << sslUrlBase.arg("/any")
        << 200
        << "text/plain"
        << "Delete";

#endif // QT_CONFIG(ssl)
}

void tst_QHttpServer::routeDelete()
{
    QFETCH(QString, url);
    QFETCH(int, code);
    QFETCH(QString, type);
    QFETCH(QString, data);

    auto reply = networkAccessManager.deleteResource(QNetworkRequest(url));

    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);

    reply->deleteLater();
}

void tst_QHttpServer::routeExtraHeaders()
{
    const QUrl requestUrl(urlBase.arg("/extra-headers"));
    auto reply = networkAccessManager.get(QNetworkRequest(requestUrl));

    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), "application/x-empty");
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader), "test server");
}

struct CustomType {
    CustomType() {}
    CustomType(const QString &) {}
};

void tst_QHttpServer::invalidRouterArguments()
{
    QTest::ignoreMessage(QtWarningMsg, "Can not find converter for type: QDateTime");
    QCOMPARE(
        httpserver.route("/datetime/", [] (const QDateTime &datetime) {
            return QString("datetime: %1").arg(datetime.toString());
        }),
        false);

    QTest::ignoreMessage(QtWarningMsg, "Can not convert GeT to QHttpServerRequest::Method");
    QCOMPARE(
        httpserver.route("/invalid-rule-method", "GeT", [] () {
            return "";
        }),
        false);

    QTest::ignoreMessage(QtWarningMsg, "Can not convert Garbage to QHttpServerRequest::Method");
    QCOMPARE(
        httpserver.route("/invalid-rule-method", "Garbage", [] () {
            return "";
        }),
        false);

    QCOMPARE(
        httpserver.route("/invalid-rule-method", "Unknown", [] () {
            return "";
        }),
        false);

    QTest::ignoreMessage(QtWarningMsg,
                         "CustomType has not registered a converter to QString. "
                         "Use QHttpServerRouter::addConveter<Type>(converter).");
    QCOMPARE(
        httpserver.route("/implicit-conversion-to-qstring-has-no-registered/",
                         [] (const CustomType &) {
            return "";
        }),
        false);
}

void tst_QHttpServer::checkRouteLambdaCapture()
{
    httpserver.route("/capture-this/", [this] () {
        return urlBase;
    });

    QString msg = urlBase + "/pod";
    httpserver.route("/capture-non-pod-data/", [&msg] () {
        return msg;
    });

    checkReply(networkAccessManager.get(
                   QNetworkRequest(QUrl(urlBase.arg("/capture-this/")))),
               urlBase);
    if (QTest::currentTestFailed())
        return;

    checkReply(networkAccessManager.get(
                   QNetworkRequest(QUrl(urlBase.arg("/capture-non-pod-data/")))),
               msg);
    if (QTest::currentTestFailed())
        return;
}

void tst_QHttpServer::afterRequest()
{
    httpserver.afterRequest([] (QHttpServerResponse &&resp,
                                const QHttpServerRequest &request) {
        if (request.url().path() == "/test-after-request")
            resp.setHeader("Arguments-Order-1", "resp, request");

        return std::move(resp);
    });

    httpserver.afterRequest([] (const QHttpServerRequest &request,
                                QHttpServerResponse &&resp) {
        if (request.url().path() == "/test-after-request")
            resp.setHeader("Arguments-Order-2", "request, resp");

        return std::move(resp);
    });

    const QUrl requestUrl(urlBase.arg("/test-after-request"));
    auto reply = networkAccessManager.get(QNetworkRequest(requestUrl));

    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), "application/x-empty");
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 404);
    QCOMPARE(reply->rawHeader("Arguments-Order-1"), "resp, request");
    QCOMPARE(reply->rawHeader("Arguments-Order-2"), "request, resp");
    reply->deleteLater();
}

void tst_QHttpServer::checkReply(QNetworkReply *reply, const QString &response) {
    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), "text/plain");
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    QCOMPARE(reply->readAll(), response);

    reply->deleteLater();
};

void tst_QHttpServer::disconnectedInEventLoop()
{
    httpserver.route("/event-loop/", [] () {
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, &QEventLoop::quit);
        loop.exec();
        return QHttpServerResponse::StatusCode::Ok;
    });

    const QUrl requestUrl(urlBase.arg("/event-loop/"));
    auto reply = networkAccessManager.get(QNetworkRequest(requestUrl));
    QTimer::singleShot(500, reply, &QNetworkReply::abort); // cancel connection
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->deleteLater();
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(CustomArg);
Q_DECLARE_METATYPE(CustomType);

QTEST_MAIN(tst_QHttpServer)

#include "tst_qhttpserver.moc"
