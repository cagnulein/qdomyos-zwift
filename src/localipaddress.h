#ifndef LOCALIPADDRESS_H
#define LOCALIPADDRESS_H

#include <QObject>
#include <QHostAddress>

class localipaddress
{
public:
    static QHostAddress getIP(const QHostAddress &srcAddress);
    // Every IPv4 address of this device except localhost, to show the user where QZ can be reached.
    // On Android it asks Java: QNetworkInterface of Qt 5.15.0 returns no address there on Android 11+.
    static QList<QHostAddress> allIPv4();
#ifdef Q_OS_ANDROID
    // Explicit Wi-Fi subset of the existing Android NetworkInterface enumeration.
    static QList<QHostAddress> wifiIPv4();
    // Return a unique Wi-Fi address only if the peer routes through it.
    static QHostAddress wifiIPv4ForPeer(const QHostAddress &peer);
#endif
};

#endif // LOCALIPADDRESS_H
