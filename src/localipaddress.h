#ifndef LOCALIPADDRESS_H
#define LOCALIPADDRESS_H

#include <QObject>
#include <QHostAddress>

class localipaddress
{
public:
    static QHostAddress getIP(const QHostAddress &srcAddress);
};

#endif // LOCALIPADDRESS_H
