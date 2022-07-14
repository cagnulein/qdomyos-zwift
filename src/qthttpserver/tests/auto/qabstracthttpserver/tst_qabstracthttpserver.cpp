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

#include <QtHttpServer/qabstracthttpserver.h>

#if defined(QT_WEBSOCKETS_LIB)
#  include <QtWebSockets/qwebsocket.h>
#endif

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

#include <QtCore/qregularexpression.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qtcpserver.h>
#include <QtHttpServer/qhttpserverrequest.h>

#if defined(Q_OS_UNIX)
#  include <signal.h>
#  include <unistd.h>
#endif

QT_BEGIN_NAMESPACE

class tst_QAbstractHttpServer : public QObject
{
    Q_OBJECT

private slots:
    void request_data();
    void request();
    void checkListenWarns();
    void websocket();
    void servers();
    void fork();
    void qtbug82053();
};

void tst_QAbstractHttpServer::request_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("query");

    QTest::addRow("127.0.0.1") << "127.0.0.1" << "/" << QString();
    QTest::addRow("0.0.0.0") << "0.0.0.0" << "/" << QString();
    QTest::addRow("localhost") << "localhost" << "/" << QString();
    QTest::addRow("localhost with query") << "localhost" << "/" << QString("key=value");
    QTest::addRow("0.0.0.0 path with spaces") << "0.0.0.0" << "/test test" << QString();
    QTest::addRow("0.0.0.0 path with spec spaces") << "0.0.0.0" << "/test%20test" << QString();
    QTest::addRow("127.0.0.1 path with spaces") << "127.0.0.1" << "/test test" << QString();
    QTest::addRow("127.0.0.1 path with spec spaces") << "127.0.0.1" << "/test%20test" << QString();
}

void tst_QAbstractHttpServer::request()
{
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(QString, query);

#if defined(Q_OS_WIN)
    if (host == QLatin1String("0.0.0.0"))
        QSKIP("Windows has problems with 0.0.0.0");
#endif

    struct HttpServer : QAbstractHttpServer
    {
        QUrl url;
        QByteArray body;
        QHttpServerRequest::Method method = QHttpServerRequest::Method::Unknown;
        quint8 padding[4];

        bool handleRequest(const QHttpServerRequest &request, QTcpSocket *) override
        {
            method = request.method();
            url = request.url();
            body = request.body();
            return true;
        }
    } server;
    auto tcpServer = new QTcpServer;
    QVERIFY(tcpServer->listen());
    server.bind(tcpServer);
    QNetworkAccessManager networkAccessManager;
    QUrl url(QStringLiteral("http://%1:%2%3")
             .arg(host)
             .arg(tcpServer->serverPort())
             .arg(path));
    if (!query.isEmpty())
        url.setQuery(query);
    const QNetworkRequest request(url);
    networkAccessManager.get(request);
    QTRY_COMPARE(server.method, QHttpServerRequest::Method::Get);
    QCOMPARE(server.url, url);
    QCOMPARE(server.body, QByteArray());
}

void tst_QAbstractHttpServer::checkListenWarns()
{
    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QTcpSocket *) override { return true; }
    } server;
    auto tcpServer = new QTcpServer;
    QTest::ignoreMessage(QtWarningMsg,
                         QRegularExpression(QStringLiteral("The TCP server .* is not listening.")));
    server.bind(tcpServer);
}

void tst_QAbstractHttpServer::websocket()
{
#if !defined(QT_WEBSOCKETS_LIB)
    QSKIP("This test requires WebSocket support");
#else
    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QTcpSocket *) override { return false; }
    } server;
    auto tcpServer = new QTcpServer;
    tcpServer->listen();
    server.bind(tcpServer);
    auto makeWebSocket = [this, tcpServer] () mutable {
        auto s = new QWebSocket(QString::fromUtf8(""),
                                QWebSocketProtocol::VersionLatest,
                                this);
        const QUrl url(QString::fromLatin1("ws://localhost:%1").arg(tcpServer->serverPort()));
        s->open(url);
        return s;
    };

    // We have to send two requests to make sure that swapping between
    // QTcpSocket and QWebSockets works correctly
    auto s1 = makeWebSocket();
    auto s2 = makeWebSocket();

    QSignalSpy newConnectionSpy(&server, &HttpServer::newWebSocketConnection);
    QTRY_COMPARE(newConnectionSpy.count(), 2);
    delete server.nextPendingWebSocketConnection();
    delete server.nextPendingWebSocketConnection();
    delete s1;
    delete s2;
#endif // defined(QT_WEBSOCKETS_LIB)
}

void tst_QAbstractHttpServer::servers()
{
    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QTcpSocket *) override { return true; }
    } server;
    auto tcpServer = new QTcpServer;
    tcpServer->listen();
    server.bind(tcpServer);
    auto tcpServer2 = new QTcpServer;
    tcpServer2->listen();
    server.bind(tcpServer2);
    QTRY_COMPARE(server.servers().count(), 2);
    QTRY_COMPARE(server.serverPorts().count(), 2);
    QTRY_COMPARE(server.servers().first(), tcpServer);
    QTRY_COMPARE(server.serverPorts().first(), tcpServer->serverPort());
    QTRY_COMPARE(server.servers().last(), tcpServer2);
    QTRY_COMPARE(server.serverPorts().last(), tcpServer2->serverPort());
}

void tst_QAbstractHttpServer::fork()
{
#if defined(Q_OS_UNIX)
    const auto message = QByteArrayLiteral("Hello world!");
    struct HttpServer : QAbstractHttpServer
    {
        const QByteArray &message;
        HttpServer(const QByteArray &message) : message(message) {}
        bool handleRequest(const QHttpServerRequest &, QTcpSocket *socket) override
        {
            socket->write(QByteArrayLiteral("HTTP/1.1 200 OK"));
            socket->write(QByteArrayLiteral("\r\n"));
            socket->write(QByteArrayLiteral("Content-Length: "));
            socket->write(QByteArray::number(message.size()));
            socket->write(QByteArrayLiteral("\r\n"));
            socket->write(QByteArrayLiteral("Connection: close"));
            socket->write(QByteArrayLiteral("\r\n"));
            socket->write(QByteArrayLiteral("Content-Type: text/html"));
            socket->write(QByteArrayLiteral("\r\n\r\n"));
            socket->write(message);
            socket->flush();
            ::kill(::getpid(), SIGKILL);  // Avoids continuing running tests in the child process
            return true;
        }
    } server = { message };

    struct TcpServer : QTcpServer
    {
        void incomingConnection(qintptr socketDescriptor) override
        {
            if (::fork() != 0) {
                // Parent process: Create a QTcpSocket with the descriptor to close it properly
                QTcpSocket socket;
                socket.setSocketDescriptor(socketDescriptor);
                socket.close();
            } else {
                // Child process: It will parse the request and call HttpServer::handleRequest
                QTcpServer::incomingConnection(socketDescriptor);
            }
        }
    };
    auto tcpServer = new TcpServer;
    tcpServer->listen();
    server.bind(tcpServer);
    QNetworkAccessManager networkAccessManager;
    const QUrl url(QString::fromLatin1("http://localhost:%1").arg(tcpServer->serverPort()));
    auto reply = networkAccessManager.get(QNetworkRequest(url));
    QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
    QTRY_VERIFY(finishedSpy.count());
    QCOMPARE(reply->readAll(), message);
    reply->close();
    reply->deleteLater();
#else
    QSKIP("fork() not supported by this platform");
#endif
}

void tst_QAbstractHttpServer::qtbug82053()
{
    struct HttpServer : QAbstractHttpServer
    {
        bool wasConnectRequest{false};
        bool handleRequest(const QHttpServerRequest &req, QTcpSocket *) override
        {
            wasConnectRequest = (req.method() == QHttpServerRequest::Method::Connect);
            return false;
        }
    } server;
    auto tcpServer = new QTcpServer;
    tcpServer->listen();
    server.bind(tcpServer);

    QTcpSocket client;
    client.connectToHost(QHostAddress::LocalHost, tcpServer->serverPort());
    client.waitForConnected();
    client.write("CONNECT / HTTP/1.1\n\n");
    client.waitForBytesWritten();
    QTest::qWait(0);
    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
    QTRY_VERIFY(server.wasConnectRequest);
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QAbstractHttpServer)

#include "tst_qabstracthttpserver.moc"
