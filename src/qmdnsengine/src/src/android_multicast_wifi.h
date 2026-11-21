#ifndef QZ_ANDROID_MULTICAST_WIFI_H
#define QZ_ANDROID_MULTICAST_WIFI_H
#ifdef Q_OS_ANDROID
#include <netinet/in.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <limits>
#include <QThread>
#include <QVariant>
#include "localipaddress.h"
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>

namespace AndroidMulticast {
inline bool valid(const QHostAddress &address) {
    const quint32 ip = address.toIPv4Address();
    return address.protocol() == QAbstractSocket::IPv4Protocol &&
           ip != 0 && ip != 0xffffffff && (ip >> 24) != 0 &&
           !address.isLoopback() && !address.isLinkLocal() &&
           !address.isMulticast() && (ip >> 28) != 0xf;
}
inline QHostAddress wifiAddress() {
    QHostAddress selected;
    for (const QHostAddress &address : localipaddress::wifiIPv4()) {
        if (!valid(address)) continue;
        if (!selected.isNull() && selected != address) {
            qWarning() << "Android mDNS WIFI_AMBIGUOUS; IPv4 send skipped";
            return QHostAddress();
        }
        selected = address;
    }
    if (selected.isNull())
        qWarning() << "Android mDNS WIFI_ABSENT; IPv4 send skipped";
    return selected;
}
inline bool canSend(QUdpSocket &socket) {
    if (QThread::currentThread() != socket.thread()) {
        qWarning() << "Android mDNS WRONG_THREAD; IPv4 multicast skipped";
        return false;
    }
    if (socket.property("androidMdnsMulticastBlocked").toBool() || socket.property("androidMdnsMulticastActive").toBool()) {
        qWarning() << "Android mDNS BLOCKED_OR_REENTRANT; IPv4 multicast skipped";
        return false;
    }
    return true;
}
struct RestoreOption {
    QUdpSocket &socket;
    const int fd;
    const in_addr previous;
    bool armed = false;
    RestoreOption(QUdpSocket &s, int descriptor, in_addr value)
        : socket(s), fd(descriptor), previous(value) {}
    RestoreOption(const RestoreOption &) = delete;
    RestoreOption &operator=(const RestoreOption &) = delete;
    bool restore() {
        if (!armed) return true;
        armed = false;
        // Never apply the saved option to a replaced/recycled descriptor.
        bool ok = socket.socketDescriptor() == fd &&
                  ::setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &previous, sizeof(previous)) == 0;
        int error = ok ? 0 : (socket.socketDescriptor() == fd ? errno : EBADF);
        in_addr observed = {};
        socklen_t size = sizeof(observed);
        if (ok) {
            ok = ::getsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &observed, &size) == 0;
            error = ok ? 0 : errno;
            if (ok && (size != sizeof(observed) || observed.s_addr != previous.s_addr)) {
                ok = false; error = EIO;
            }
        }
        if (!ok) socket.setProperty("androidMdnsMulticastBlocked", true);
        socket.setProperty("androidMdnsMulticastActive", false);
        if (!ok) qWarning() << "Android mDNS RESTORE_IF_FAILED" << error
                        << "IPv4 multicast blocked until Server recreation; unicast/IPv6/RX unchanged";
        return ok;
    }
    ~RestoreOption() { restore(); }
};
inline void sendWifi(QUdpSocket &socket, const QByteArray &packet,
                     const QHostAddress &group, quint16 port) {
    // TX interface choice is independent of IP_ADD_MEMBERSHIP used for RX.
    // Only the already-open qmdnsengine socket is changed; no global route/bindProcessToNetwork.
    if (!canSend(socket)) return;
    const QHostAddress address = wifiAddress();
    if (!valid(address)) return;
    const qintptr descriptor = socket.socketDescriptor();
    if (socket.state() != QAbstractSocket::BoundState || descriptor < 0 ||
        descriptor > std::numeric_limits<int>::max()) {
        qWarning() << "Android mDNS SOCKET_INVALID; send skipped";
        return;
    }
    const int fd = int(descriptor);
    in_addr previous = {};
    socklen_t length = sizeof(previous);
    const int saved = ::getsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &previous, &length);
    if (saved != 0 || length != sizeof(previous)) {
        const int error = saved != 0 ? errno : EINVAL;
        qWarning() << "Android mDNS GET_IF_FAILED" << error << strerror(error) << "send skipped";
        return;
    }
    RestoreOption restore(socket, fd, previous);
    in_addr chosen = {};
    chosen.s_addr = htonl(address.toIPv4Address());
    restore.armed = true; // Restore the prior setting even when the attempted change fails.
    if (::setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &chosen, sizeof(chosen)) != 0) {
        const int error = errno;
        qWarning() << "Android mDNS SET_IF_FAILED" << address << error << strerror(error)
                   << "no default-route fallback";
        return;
    }
    socket.setProperty("androidMdnsMulticastActive", true);
    in_addr observed = {};
    socklen_t observedSize = sizeof(observed);
    const int readback = ::getsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &observed, &observedSize);
    if (readback != 0 || observedSize != sizeof(observed) || observed.s_addr != chosen.s_addr) {
        const int error = readback != 0 ? errno : EIO;
        qWarning() << "Android mDNS SET_IF_VERIFY_FAILED" << error << "send skipped";
        return; // RAII restores even on this error path.
    }
    if (socket.writeDatagram(packet, group, port) != packet.size())
        qWarning() << "Android mDNS IPv4 send failed" << socket.errorString();
    restore.restore();
}
}
#endif
#endif
