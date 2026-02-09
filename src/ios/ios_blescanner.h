#ifndef IOS_BLESCANNER_H
#define IOS_BLESCANNER_H

#include <QString>

class ios_blescanner {
public:
    static void startScan();
    static void stopScan();
    static QString getDeviceServices(const QString& deviceId);
};

#endif // IOS_BLESCANNER_H
