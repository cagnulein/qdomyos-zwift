#include <gtest/gtest.h>
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QHostAddress>
#include <QUdpSocket>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../../src/qmdnsengine/src/src/android_multicast_wifi.h"

TEST(AndroidMulticast, RejectsAddressesUnsuitableForWifiMulticast) {
    EXPECT_TRUE(AndroidMulticast::valid(QHostAddress(QStringLiteral("192.168.1.12"))));
    EXPECT_FALSE(AndroidMulticast::valid(QHostAddress(QStringLiteral("0.0.0.0"))));
    EXPECT_FALSE(AndroidMulticast::valid(QHostAddress(QStringLiteral("127.0.0.1"))));
    EXPECT_FALSE(AndroidMulticast::valid(QHostAddress(QStringLiteral("169.254.1.1"))));
    EXPECT_FALSE(AndroidMulticast::valid(QHostAddress(QStringLiteral("224.0.0.251"))));
}

TEST(AndroidMulticast, RestoresOptionOnTheSameBoundSocket) {
    QUdpSocket socket;
    ASSERT_TRUE(socket.bind(QHostAddress::AnyIPv4, 0));
    const int descriptor = static_cast<int>(socket.socketDescriptor());
    in_addr previous = {};
    socklen_t length = sizeof(previous);
    ASSERT_EQ(0, ::getsockopt(descriptor, IPPROTO_IP, IP_MULTICAST_IF,
                              &previous, &length));
    ASSERT_EQ(sizeof(previous), length);

    in_addr selected = {};
    selected.s_addr = htonl(INADDR_LOOPBACK);
    ASSERT_EQ(0, ::setsockopt(descriptor, IPPROTO_IP, IP_MULTICAST_IF,
                              &selected, sizeof(selected)));

    AndroidMulticast::RestoreOption restore(socket, descriptor, previous);
    restore.armed = true;
    EXPECT_TRUE(restore.restore());
    in_addr observed = {};
    length = sizeof(observed);
    ASSERT_EQ(0, ::getsockopt(descriptor, IPPROTO_IP, IP_MULTICAST_IF,
                              &observed, &length));
    EXPECT_EQ(previous.s_addr, observed.s_addr);
    EXPECT_FALSE(socket.property("androidMdnsMulticastBlocked").toBool());
    EXPECT_FALSE(socket.property("androidMdnsMulticastActive").toBool());
}

TEST(AndroidMulticast, FailedRestoreBlocksSocketReuse) {
    QUdpSocket socket;
    ASSERT_TRUE(socket.bind(QHostAddress::AnyIPv4, 0));
    const int descriptor = static_cast<int>(socket.socketDescriptor());
    in_addr previous = {};
    socket.close();

    AndroidMulticast::RestoreOption restore(socket, descriptor, previous);
    restore.armed = true;
    EXPECT_FALSE(restore.restore());
    EXPECT_TRUE(socket.property("androidMdnsMulticastBlocked").toBool());
    EXPECT_FALSE(AndroidMulticast::canSend(socket));
}
#endif
