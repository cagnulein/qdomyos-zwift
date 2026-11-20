#ifndef IOS_FLOATING_METRICS_H
#define IOS_FLOATING_METRICS_H

#include <QList>
#include <QPair>
#include <QString>

class iosFloatingMetrics {
public:
    static bool supported();
    static void start();
    static void stop();
    static void update(const QList<QPair<QString, QString>> &metrics);
    static bool running();
};
#endif
