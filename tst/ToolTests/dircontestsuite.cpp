#include "dircontestsuite.h"

#include "../../src/devices/dircon/dirconpacket.h"
#include "../../src/devices/dircon/dirconprocessor.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QHostAddress>
#include <QElapsedTimer>
#include <QTcpServer>
#include <QTcpSocket>

namespace {

void processEventsFor(int milliseconds) {
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < milliseconds)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
}

} // namespace

TEST_F(DirconTestSuite, IncompleteFrameReturnsWaitWithoutReadingPayload) {
    QByteArray frame;
    frame.append(char(1));
    frame.append(char(DPKT_MSGID_READ_CHARACTERISTIC));
    frame.append(char(1));
    frame.append(char(DPKT_RESPCODE_SUCCESS_REQUEST));
    frame.append(char(0));
    frame.append(char(16));
    frame.append(char(0));

    DirconPacket packet;
    EXPECT_EQ(packet.parse(frame, 0), DPKT_PARSE_WAIT);
}

TEST_F(DirconTestSuite, QueuedDataCallbackAfterDisconnectIsSafe) {
    QList<DirconProcessorService *> services;
    DirconProcessor processor(services, QStringLiteral("test-dircon"), 0, QStringLiteral("test"),
                              QStringLiteral("00:00:00:00:00:00"));
    ASSERT_TRUE(processor.init());
    auto *server = processor.findChild<QTcpServer *>();
    ASSERT_NE(server, nullptr);
    ASSERT_TRUE(server->isListening());

    QByteArray frame;
    frame.append(char(1));
    frame.append(char(DPKT_MSGID_READ_CHARACTERISTIC));
    frame.append(char(1));
    frame.append(char(DPKT_RESPCODE_SUCCESS_REQUEST));
    frame.append(char(0));
    frame.append(char(16));
    frame.append(char(0));

    QTcpSocket clientSocket;
    clientSocket.connectToHost(QHostAddress::LocalHost, server->serverPort());
    ASSERT_TRUE(clientSocket.waitForConnected(1000));
    processEventsFor(1); // Let tcpNewConnection() create the server-side client.

    const auto serverSockets = server->findChildren<QTcpSocket *>();
    ASSERT_EQ(serverSockets.size(), 1);
    QTcpSocket *serverSocket = serverSockets.first();

    // tcpDisconnected() is already connected directly by tcpNewConnection().
    // This second connection queues the same data callback after the direct
    // disconnect handler has removed the client and scheduled the socket for
    // deletion. It models a stale callback delivered during Zwift shutdown.
    ASSERT_TRUE(QObject::connect(serverSocket, SIGNAL(disconnected()), &processor,
                                 SLOT(tcpDataAvailable()), Qt::QueuedConnection));

    ASSERT_EQ(clientSocket.write(frame), frame.size());
    clientSocket.abort();
    processEventsFor(20);
}
