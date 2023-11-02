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

#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qabstracthttpserver.h>

#include <private/qhttpserverliterals_p.h>

#include <QtCore/qjsondocument.h>
#include <QtCore/qfile.h>
#include <QtCore/qtemporaryfile.h>
#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>
#include <QtNetwork/qnetworkaccessmanager.h>

#include <functional>

QT_BEGIN_NAMESPACE

static const QByteArray headerServerString(QByteArrayLiteral("Server"));
static const QByteArray headerServerValue(QByteArrayLiteral("Test server"));

class tst_QHttpServerResponder : public QObject
{
    Q_OBJECT

    std::unique_ptr<QNetworkAccessManager> networkAccessManager;

private slots:
    void init() { networkAccessManager.reset(new QNetworkAccessManager); }
    void cleanup() { networkAccessManager.reset(); }

    void defaultStatusCodeNoParameters();
    void defaultStatusCodeByteArray();
    void defaultStatusCodeJson();
    void writeStatusCode_data();
    void writeStatusCode();
    void writeStatusCodeExtraHeader();
    void writeJson();
    void writeJsonExtraHeader();
    void writeFile_data();
    void writeFile();
    void writeFileExtraHeader();
    void writeByteArrayExtraHeader();
};

#define qWaitForFinished(REPLY) QVERIFY(QSignalSpy(REPLY, &QNetworkReply::finished).wait())

struct HttpServer : QAbstractHttpServer {
    std::function<void(QHttpServerResponder responder)> handleRequestFunction;
    QUrl url { QStringLiteral("http://localhost:%1").arg(listen()) };

    HttpServer(decltype(handleRequestFunction) function) : handleRequestFunction(function) {}
    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override;
};

bool HttpServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
{
    handleRequestFunction(makeResponder(request, socket));
    return true;
}

void tst_QHttpServerResponder::defaultStatusCodeNoParameters()
{
    HttpServer server([](QHttpServerResponder responder) { responder.write(); });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
}

void tst_QHttpServerResponder::defaultStatusCodeByteArray()
{
    HttpServer server([](QHttpServerResponder responder) {
        responder.write(QByteArray(), QHttpServerLiterals::contentTypeXEmpty());
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
}

void tst_QHttpServerResponder::defaultStatusCodeJson()
{
    const auto json = QJsonDocument::fromJson(QByteArrayLiteral("{}"));
    HttpServer server([json](QHttpServerResponder responder) { responder.write(json); });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
}

void tst_QHttpServerResponder::writeStatusCode_data()
{
    using StatusCode = QHttpServerResponder::StatusCode;

    QTest::addColumn<QHttpServerResponder::StatusCode>("statusCode");
    QTest::addColumn<QNetworkReply::NetworkError>("networkError");

    QTest::addRow("OK") << StatusCode::Ok << QNetworkReply::NoError;
    QTest::addRow("Content Access Denied") << StatusCode::Forbidden
                                           << QNetworkReply::ContentAccessDenied;
    QTest::addRow("Connection Refused") << StatusCode::NotFound
                                        << QNetworkReply::ContentNotFoundError;
}

void tst_QHttpServerResponder::writeStatusCode()
{
    QFETCH(QHttpServerResponder::StatusCode, statusCode);
    QFETCH(QNetworkReply::NetworkError, networkError);
    HttpServer server([statusCode](QHttpServerResponder responder) {
        responder.write(statusCode);
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->bytesAvailable(), 0);
    QCOMPARE(reply->error(), networkError);
    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader),
             QByteArrayLiteral("application/x-empty"));
}

void tst_QHttpServerResponder::writeStatusCodeExtraHeader()
{
    HttpServer server([=](QHttpServerResponder responder) {
        responder.write({{ headerServerString, headerServerValue }});
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->bytesAvailable(), 0);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader), headerServerValue);
}

void tst_QHttpServerResponder::writeJson()
{
    const auto json = QJsonDocument::fromJson(QByteArrayLiteral(R"JSON({ "key" : "value" })JSON"));
    HttpServer server([json](QHttpServerResponder responder) { responder.write(json); });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader),
             QHttpServerLiterals::contentTypeJson());
    QCOMPARE(QJsonDocument::fromJson(reply->readAll()), json);
}

void tst_QHttpServerResponder::writeJsonExtraHeader()
{
    const auto json = QJsonDocument::fromJson(QByteArrayLiteral(R"JSON({ "key" : "value" })JSON"));
    HttpServer server([json](QHttpServerResponder responder) {
        responder.write(json, {{ headerServerString, headerServerValue }});
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader),
             QHttpServerLiterals::contentTypeJson());
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader), headerServerValue);
    QCOMPARE(QJsonDocument::fromJson(reply->readAll()), json);
}

void tst_QHttpServerResponder::writeFile_data()
{
    QTest::addColumn<QIODevice *>("iodevice");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("data");

    QTest::addRow("index.html")
        << qobject_cast<QIODevice *>(new QFile(QFINDTESTDATA("data/index.html"), this))
        << 200
        << "text/html"
        << "<html></html>";

    QTest::addRow("index1.html - not found")
        << qobject_cast<QIODevice *>(new QFile("./index1.html", this))
        << 500
        << "application/x-empty"
        << QString();

    QTest::addRow("temporary file")
        << qobject_cast<QIODevice *>(new QTemporaryFile(this))
        << 200
        << "text/html"
        << QString();
}

void tst_QHttpServerResponder::writeFile()
{
    QFETCH(QIODevice *, iodevice);
    QFETCH(int, code);
    QFETCH(QString, type);
    QFETCH(QString, data);

    QSignalSpy spyDestroyIoDevice(iodevice, &QObject::destroyed);

    HttpServer server([&iodevice](QHttpServerResponder responder) {
        responder.write(iodevice, QHttpServerLiterals::contentTypeTextHtml());
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);
    QCOMPARE(reply->readAll().trimmed(), data);

    QCOMPARE(spyDestroyIoDevice.count(), 1);
}

void tst_QHttpServerResponder::writeFileExtraHeader()
{
    auto file = new QFile(QFINDTESTDATA("data/index.html"), this);
    QSignalSpy spyDestroyIoDevice(file, &QObject::destroyed);

    HttpServer server([=](QHttpServerResponder responder) {
        responder.write(
            file,
            {
                 {
                    QHttpServerLiterals::contentTypeHeader(),
                    QHttpServerLiterals::contentTypeTextHtml()
                 },
                 { headerServerString, headerServerValue }
            });
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader),
                           QHttpServerLiterals::contentTypeTextHtml());
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader), headerServerValue);
    QCOMPARE(reply->readAll().trimmed(), "<html></html>");

    QCOMPARE(spyDestroyIoDevice.count(), 1);
}

void tst_QHttpServerResponder::writeByteArrayExtraHeader()
{
    const QByteArray data("test data");
    const QByteArray contentType("text/plain");

    HttpServer server([=](QHttpServerResponder responder) {
        responder.write(
            data,
            {
                 { "Content-Type", contentType },
                 { headerServerString, headerServerValue }
            });
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), contentType);
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader), headerServerValue);
    QCOMPARE(reply->readAll(), data);
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QHttpServerResponder)

#include "tst_qhttpserverresponder.moc"
